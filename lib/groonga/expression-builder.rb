# -*- coding: utf-8 -*-
#
# Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>
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
  # @private
  module ExpressionBuildable
    attr_reader :table
    attr_accessor :query
    attr_accessor :syntax
    attr_accessor :allow_pragma
    attr_accessor :allow_column
    attr_accessor :allow_update
    attr_accessor :default_column

    VALID_COLUMN_NAME_RE = /\A[a-zA-Z\d_]+\z/

    def initialize
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
      expression = Expression.new(:name => @name, :context => @table.context)
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
      builders = []
      builders << match(@query, default_parse_options) if @query
      if block_given?
        custom_builder = yield(self)
        if custom_builder.is_a?(::Array)
          builders.concat(custom_builder)
        else
          builders << custom_builder
        end
      end

      if builders.empty? or builders == [self]
        expression.append_constant(1)
        expression.append_constant(1)
        expression.append_operation(Groonga::Operation::OR, 2)
      else
        combined_builder = builders.inject(nil) do |previous, builder|
          if previous.nil?
            builder
          else
            previous & builder
          end
        end
        combined_builder.build(expression, variable)
      end

      expression.compile
      expression
    end

    # @private
    class ExpressionBuilder
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

    # @private
    class SetExpressionBuilder < ExpressionBuilder
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

    # @private
    class AndExpressionBuilder < SetExpressionBuilder
      def initialize(*expression_builders)
        super(Groonga::Operation::AND, *expression_builders)
      end
    end

    # @private
    class OrExpressionBuilder < SetExpressionBuilder
      def initialize(*expression_builders)
        super(Groonga::Operation::OR, *expression_builders)
      end
    end

    # @private
    class ColumnValueExpressionBuilder < ExpressionBuilder
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

      def prefix_search(other)
        PrefixSearchExpressionBuilder.new(self, normalize(other))
      end

      def suffix_search(other)
        SuffixSearchExpressionBuilder.new(self, normalize(other))
      end

      private
      def normalize(other)
        if @range.is_a?(Groonga::Table)
          if other.respond_to?(:record_id)
            id = other.record_id
          else
            id = other
          end
          return Groonga::Record.new(@range, id) if id.is_a?(Integer)
        end
        other
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

    # @private
    class MatchTargetColumnExpressionBuilder < ColumnValueExpressionBuilder
      def build(expression, variable)
        if @column.is_a?(String)
          expression.append_constant(@column)
        else
          expression.append_object(@column)
        end
      end

      private
      def normalize(other)
        other
      end
    end

    # @private
    class MatchTargetExpressionBuilder < ExpressionBuilder
      def initialize(target)
        super()
        @target = target
      end

      def build(expression, variable)
        expression.append_object(variable)
        expression.append_object(@target)
        expression.append_operation(Groonga::Operation::GET_VALUE, 2)
      end

      def =~(query)
        if query.nil?
          raise ArgumentError, "match word should not be nil"
        end
        MatchExpressionBuilder.new(self, query)
      end
    end

    # @private
    class BinaryExpressionBuilder < ExpressionBuilder
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

    # @private
    class EqualExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::EQUAL, column_value_builder, value)
      end
    end

    # @private
    class MatchExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::MATCH, column_value_builder, value)
      end
    end

    # @private
    class LessExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::LESS, column_value_builder, value)
      end
    end

    # @private
    class LessEqualExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::LESS_EQUAL, column_value_builder, value)
      end
    end

    # @private
    class GreaterExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::GREATER, column_value_builder, value)
      end
    end

    # @private
    class GreaterEqualExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::GREATER_EQUAL, column_value_builder, value)
      end
    end

    # @private
    class PlusExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::PLUS, column_value_builder, value)
      end
    end

    # @private
    class MinusExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::MINUS, column_value_builder, value)
      end
    end

    # @private
    class StarExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::STAR, column_value_builder, value)
      end
    end

    # @private
    class SlashExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::SLASH, column_value_builder, value)
      end
    end

    # @private
    class ModExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::MOD, column_value_builder, value)
      end
    end

    # @private
    class SubExpressionBuilder < ExpressionBuilder
      def initialize(query, options)
        super()
        @query = query
        @options = options
      end

      def build(expression, variable)
        expression.parse(@query, @options)
      end
    end

    # @private
    class PrefixSearchExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::PREFIX, column_value_builder, value)
      end
    end

    # @private
    class SuffixSearchExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::SUFFIX, column_value_builder, value)
      end
    end
  end

  # @private
  class RecordExpressionBuilder
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
      column_expression_builder(column, name)
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
        options[:default_column] = build_match_target(&block)
      end
      SubExpressionBuilder.new(query, options)
    end

    def match_target(&block)
      MatchTargetExpressionBuilder.new(build_match_target(&block))
    end

    def index(name)
      object = @table.context[name]
      if object.nil?
        raise ArgumentError, "unknown index column: <#{name}>"
      end
      if object.range != @table
        raise ArgumentError,
              "different index column: <#{name}>: #{object.inspect}"
      end
      column_expression_builder(object, name)
    end

    private
    def build_match_target(&block)
      sub_builder = MatchTargetRecordExpressionBuilder.new(@table, nil)
      sub_builder.build do |record|
        block.call(record)
      end
    end

    def column_expression_builder(column, name)
      ColumnValueExpressionBuilder.new(column,
                                       :table => @table,
                                       :column_name => name)
    end

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

  # @private
  class MatchTargetRecordExpressionBuilder < RecordExpressionBuilder
    private
    def column_expression_builder(column, name)
      MatchTargetColumnExpressionBuilder.new(column,
                                             :table => @table,
                                             :column_name => name)
    end
  end

  # @private
  class ColumnExpressionBuilder
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

if RUBY_VERSION >= "1.9"
  require "groonga/expression-builder-19"
end
