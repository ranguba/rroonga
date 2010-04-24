# -*- coding: utf-8 -*-
#
# Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>
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

    VALID_COLUMN_NAME_RE = /\A[a-zA-Z\d_]+\z/

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

    def build(&block)
      expression = Expression.new(:name => @name)
      variable = expression.define_variable(:domain => @table)
      build_expression(expression, variable, &block)
    end

    def &(other)
      other
    end

    def |(other)
      other
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

    def build_expression(expression, variable)
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

    class ColumnValueExpressionBuilder < ExpressionBuilder # :nodoc:
      def initialize(column, options={})
        super()
        @table = options[:table] || column.table
        @column = column
        @column_name = options[:column_name] || @column.local_name
        @range = options[:range] || @column.range
        @name = options[:name]
      end

      def build(expression, variable)
        expression.append_object(variable)
        if @column.is_a?(String)
          expression.append_constant(@column)
        else
          expression.append_object(@column)
        end
        expression.append_operation(Groonga::Operation::GET_VALUE, 2)
      end

      def ==(other)
        EqualExpressionBuilder.new(self, normalize(other))
      end

      def =~(other)
        if other.nil?
          full_column_name = "#{@table.name}.#{@column_name}"
          raise ArgumentError,
                 "match word should not be nil: #{full_column_name}"
        end
        MatchExpressionBuilder.new(self, normalize(other))
      end

      def <(other)
        LessExpressionBuilder.new(self, normalize(other))
      end

      def <=(other)
        LessEqualExpressionBuilder.new(self, normalize(other))
      end

      def >(other)
        GreaterExpressionBuilder.new(self, normalize(other))
      end

      def >=(other)
        GreaterEqualExpressionBuilder.new(self, normalize(other))
      end

      def +(other)
        PlusExpressionBuilder.new(self, normalize(other))
      end

      def -(other)
        MinusExpressionBuilder.new(self, normalize(other))
      end

      def *(other)
        StarExpressionBuilder.new(self, normalize(other))
      end

      def /(other)
        SlashExpressionBuilder.new(self, normalize(other))
      end

      def %(other)
        ModExpressionBuilder.new(self, normalize(other))
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

      def method_missing(name, *args, &block)
        return super if block
        return super unless args.empty?
        if VALID_COLUMN_NAME_RE =~ name.to_s
          RecordExpressionBuilder.new(@table, @name)["#{@column_name}.#{name}"]
        else
          super
        end
      end
    end

    class BinaryExpressionBuilder < ExpressionBuilder # :nodoc:
      def initialize(operation, column_value_builder, value)
        super()
        @operation = operation
        @column_value_builder = column_value_builder
        @value = value
      end

      def build(expression, variable)
        @column_value_builder.build(expression, variable)
        expression.append_constant(@value)
        expression.append_operation(@operation, 2)
      end
    end

    class EqualExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::EQUAL, column_value_builder, value)
      end
    end

    class MatchExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::MATCH, column_value_builder, value)
      end
    end

    class LessExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::LESS, column_value_builder, value)
      end
    end

    class LessEqualExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::LESS_EQUAL, column_value_builder, value)
      end
    end

    class GreaterExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::GREATER, column_value_builder, value)
      end
    end

    class GreaterEqualExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::GREATER_EQUAL, column_value_builder, value)
      end
    end

    class PlusExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::PLUS, column_value_builder, value)
      end
    end

    class MinusExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::MINUS, column_value_builder, value)
      end
    end

    class StarExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::STAR, column_value_builder, value)
      end
    end

    class SlashExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::SLASH, column_value_builder, value)
      end
    end

    class ModExpressionBuilder < BinaryExpressionBuilder # :nodoc:
      def initialize(column_value_builder, value)
        super(Groonga::Operation::MOD, column_value_builder, value)
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
      ColumnValueExpressionBuilder.new(column,
                                       :table => @table,
                                       :column_name => name)
    end

    def id
      self["_id"]
    end

    def key
      self["_key"]
    end

    def score
      self["_score"]
    end

    def n_sub_records
      self["_nsubrecs"]
    end

    def match(query, options_or_default_column={}, &block)
      if options_or_default_column.is_a?(String)
        options = {:default_column => options_or_default_column}
      else
        options = options_or_default_column
      end
      options = options.dup
      options[:syntax] ||= :query
      if block_given? and options[:default_column].nil?
        default_column = self.class.new(@table, nil)
        options[:default_column] = default_column.build do |record|
          block.call(record)
        end
      end
      SubExpressionBuilder.new(query, options)
    end

    private
    def method_missing(name, *args, &block)
      return super if block
      return super unless args.empty?
      if VALID_COLUMN_NAME_RE =~ name.to_s
        self[name]
      else
        super
      end
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
      column_value_builder == other
    end

    def =~(other)
      column_value_builder =~ other
    end

    def <(other)
      column_value_builder < other
    end

    def <=(other)
      column_value_builder <= other
    end

    def >(other)
      column_value_builder > other
    end

    def >=(other)
      column_value_builder >= other
    end

    def +(other)
      column_value_builder + other
    end

    def -(other)
      column_value_builder - other
    end

    def *(other)
      column_value_builder * other
    end

    def /(other)
      column_value_builder / other
    end

    def %(other)
      column_value_builder % other
    end

    def match(query, options={}, &block)
      column_value_builder.match(query, options, &block)
    end

    private
    def column_value_builder
      ColumnValueExpressionBuilder.new(@default_column,
                                       :table => @table,
                                       :column_name => @column_name,
                                       :range => @range)
    end

    def method_missing(name, *args, &block)
      return super if block
      return super unless args.empty?
      if VALID_COLUMN_NAME_RE =~ name.to_s
        column_value_builder.send(name)
      else
        super
      end
    end
  end
end
