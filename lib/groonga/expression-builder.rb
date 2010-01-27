# -*- coding: utf-8 -*-
#
# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

module Groonga
  module ExpressionBuildable # :nodoc:
    attr_reader :table
    attr_accessor :query
    attr_accessor :syntax
    attr_accessor :allow_pragma
    attr_accessor :allow_column
    attr_accessor :allow_update
    attr_accessor :default_column

    def initialize(*args)
      @table = nil
      @name = nil
      @query = nil
      @syntax = nil
      @allow_pragma = nil
      @allow_column = nil
      @allow_update = nil
      @default_column = nil
    end

    def build
      expression = Expression.new(:name => @name)
      variable = expression.define_variable(:domain => @table)

      builder = nil
      builder = match(@query, default_parse_options) if @query
      if block_given?
        if builder
          builder &= yield(self)
        else
          builder = yield(self)
        end
      end
      if builder.nil? or builder == self
        expression.append_constant(1)
        expression.append_constant(1)
        expression.append_operation(Groonga::Operation::OR, 2)
      else
        builder.build(expression, variable)
      end

      expression.compile
      expression
    end

    private
    def default_parse_options
      {
        :syntax => @syntax,
        :allow_pragma => @allow_pragma,
        :allow_column => @allow_column,
        :allow_update => @allow_update,
        :default_column => @default_column,
      }
    end

    class ExpressionBuilder # :nodoc:
      def initialize
        super()
      end

      def &(other)
        AndExpressionBuilder.new(self, other)
      end

      def |(other)
        OrExpressionBuilder.new(self, other)
      end
    end

    class SetExpressionBuilder < ExpressionBuilder # :nodoc:
      def initialize(operation, *expression_builders)
        super()
        @operation = operation
        @expression_builders = expression_builders
      end

      def build(expression, variable)
        return if @expression_builders.empty?
        @expression_builders.each do |builder|
          builder.build(expression, variable)
        end
        expression.append_operation(@operation, @expression_builders.size)
      end
    end

    class AndExpressionBuilder < SetExpressionBuilder # :nodoc:
      def initialize(*expression_builders)
        super(Groonga::Operation::AND, *expression_builders)
      end
    end

    class OrExpressionBuilder < SetExpressionBuilder # :nodoc:
      def initialize(*expression_builders)
        super(Groonga::Operation::OR, *expression_builders)
      end
    end

    class BinaryExpressionBuilder < ExpressionBuilder # :nodoc:
      def initialize(operation, column, value)
        super()
        @operation = operation
        @default_column = column
        @value = value
      end

      def build(expression, variable)
        expression.append_object(variable)
        if @default_column.is_a?(String)
          expression.append_constant(@default_column)
        else
          expression.append_object(@default_column)
        end
        expression.append_operation(Groonga::Operation::GET_VALUE, 2)
        expression.append_constant(@value)
        expression.append_operation(@operation, 2)
      end
    end

    class EqualExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column, value)
        super(Groonga::Operation::EQUAL, column, value)
      end
    end

    class MatchExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column, value)
        super(Groonga::Operation::MATCH, column, value)
      end
    end

    class LessExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column, value)
        super(Groonga::Operation::LESS, column, value)
      end
    end

    class LessEqualExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column, value)
        super(Groonga::Operation::LESS_EQUAL, column, value)
      end
    end

    class GreaterExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column, value)
        super(Groonga::Operation::GREATER, column, value)
      end
    end

    class GreaterEqualExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column, value)
        super(Groonga::Operation::GREATER_EQUAL, column, value)
      end
    end

    class SubExpressionBuilder < ExpressionBuilder # :nodoc:
      def initialize(query, options)
        super()
        @query = query
        @options = options
      end

      def build(expression, variable)
        expression.parse(@query, @options)
      end
    end
  end

  class RecordExpressionBuilder # :nodoc:
    include ExpressionBuildable

    def initialize(table, name)
      super()
      @table = table
      @name = name
    end

    def [](name)
      column = @table.column(name)
      if column.nil?
        message = "unknown column <#{name.inspect}> " +
          "for table <#{@table.inspect}>"
        raise ArgumentError, message
      end
      ColumnExpressionBuilder.new(column, nil, nil,
                                  :table => @table,
                                  :column_name => name)
    end

    def match(query, options_or_default_column={})
      if options_or_default_column.is_a?(String)
        options = {:default_column => options_or_default_column}
      else
        options = options_or_default_column
      end
      options = options.dup
      options[:syntax] ||= :query
      SubExpressionBuilder.new(query, options)
    end
  end

  class ColumnExpressionBuilder # :nodoc:
    include ExpressionBuildable

    def initialize(column, name, query, options={})
      super()
      @table = options[:table] || column.table
      @default_column = column
      @column_name = options[:column_name] || @default_column.local_name
      @range = @default_column.range
      @name = name
      @query = query
    end

    def ==(other)
      EqualExpressionBuilder.new(@default_column, normalize(other))
    end

    def =~(other)
      if other.nil?
        full_column_name = "#{@table.name}.#{@column_name}"
        raise ArgumentError, "match word should not be nil: #{full_column_name}"
      end
      MatchExpressionBuilder.new(@default_column, normalize(other))
    end

    def <(other)
      LessExpressionBuilder.new(@default_column, normalize(other))
    end

    def <=(other)
      LessEqualExpressionBuilder.new(@default_column, normalize(other))
    end

    def >(other)
      GreaterExpressionBuilder.new(@default_column, normalize(other))
    end

    def >=(other)
      GreaterEqualExpressionBuilder.new(@default_column, normalize(other))
    end

    def match(query, options={})
      options = options.dup
      options[:syntax] ||= :query
      options[:default_column] = @column_name
      SubExpressionBuilder.new(query, options)
    end

    private
    def normalize(other)
      if @range.is_a?(Groonga::Table) and other.is_a?(Integer)
        Groonga::Record.new(@range, other)
      else
        other
      end
    end
  end
end
