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
  class Schema
    class << self
      def define(options={})
        schema = new(options)
        yield(schema)
        schema.define
      end

      def create_table(name, options={}, &block)
        define do |schema|
          schema.create_table(name, options, &block)
        end
      end

      def normalize_type(type)
        return type if type.nil?
        return type if type.is_a?(Groonga::Object)
        case type.to_s
        when "string"
          "Shorttext"
        when "text"
          "Text"
        when "int", "integer"
          "Int32"
        when "float"
          "Float"
        when "decimal"
          "Int64"
        when "datetime", "timestamp", "time", "date"
          "Time"
        when "binary"
          "Longtext"
        when "boolean"
          "Bool"
        else
          type
        end
      end

      def dump(options={})
        Dumper.new(options).dump
      end
    end

    def initialize(options={})
      @options = (options || {}).dup
      @definitions = []
    end

    def define
      @definitions.each do |definition|
        definition.define
      end
    end

    def create_table(name, options={})
      definition = TableDefinition.new(name, @options.merge(options || {}))
      yield(definition)
      @definitions << definition
    end

    class TableDefinition
      def initialize(name, options)
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @columns = []
        @options = options
        @table_type = table_type
      end

      def define
        table = @table_type.create(create_options)
        @columns.each do |column|
          column.define(table)
        end
        table
      end

      def column(name, type, options={})
        column = self[name] || ColumnDefinition.new(name, options)
        column.type = type
        column.options.merge!(options)
        @columns << column unless @columns.include?(column)
        self
      end

      def integer32(name, options={})
        column(name, "<int>", options)
      end
      alias_method :integer, :integer32
      alias_method :int32, :integer32

      def integer64(name, options={})
        column(name, "<int64>", options)
      end
      alias_method :int64, :integer64

      def unsigned_integer32(name, options={})
        column(name, "<uint>", options)
      end
      alias_method :unsigned_integer, :unsigned_integer32
      alias_method :uint32, :unsigned_integer32

      def unsigned_integer64(name, options={})
        column(name, "<uint64>", options)
      end
      alias_method :uint64, :unsigned_integer64

      def float(name, options={})
        column(name, "<float>", options)
      end

      def time(name, options={})
        column(name, "<time>", options)
      end

      def short_text(name, options={})
        column(name, "<shorttext>", options)
      end
      alias_method :string, :short_text

      def text(name, options={})
        column(name, "<text>", options)
      end

      def long_text(name, options={})
        column(name, "<longtext>", options)
      end

      def index(name, target_column, options={})
        column = self[name] || IndexColumnDefinition.new(name, options)
        column.target = target_column
        column.options.merge!(options)
        @columns << column unless @columns.include?(column)
        self
      end

      def [](name)
        @columns.find {|column| column.name == name}
      end

      private
      def table_type
        type = @options[:type]
        case type
        when :array, nil
          Groonga::Array
        when :hash
          Groonga::Hash
        when :patricia_trie
          Groonga::PatriciaTrie
        else
          raise ArgumentError, "unknown table type: #{type.inspect}"
        end
      end

      def create_options
        common = {
          :name => @name,
          :path => @options[:path],
          :persistent => @options[:persistent],
          :value_size => @options[:value_size],
          :context => context,
        }
        key_support_table_common = {
          :key_type => Schema.normalize_type(@options[:key_type]),
          :default_tokenizer => @options[:default_tokenizer],
        }

        if @table_type == Groonga::Array
          common
        elsif @table_type == Groonga::Hash
          common.merge(key_support_table_common)
        elsif @table_type == Groonga::PatriciaTrie
          options = {
            :key_normalize => @options[:key_normalize],
            :key_with_sis => @options[:key_with_sis],
          }
          common.merge(key_support_table_common).merge(options)
        else
          raise ArgumentError, "unknown table type: #{@table_type.inspect}"
        end
      end

      def context
        @options[:context] || Groonga::Context.default
      end
    end

    class ColumnDefinition
      attr_accessor :name, :type
      attr_reader :options

      def initialize(name, options={})
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @options = (options || {}).dup
        @type = nil
      end

      def define(table)
        table.define_column(@name,
                            Schema.normalize_type(@type),
                            @options)
      end
    end

    class IndexColumnDefinition
      attr_accessor :name, :target
      attr_reader :options

      def initialize(name, options={})
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @options = (options || {}).dup
        @target = nil
      end

      def define(table)
        target = @target
        target = context[target] unless target.is_a?(Groonga::Object)
        index = table.define_index_column(@name,
                                          target.table,
                                          @options)
        index.source = target
        index
      end

      private
      def context
        @options[:context] || Groonga::Context.default
      end
    end

    class Dumper
      def initialize(options={})
        @options = (options || {}).dup
      end

      def dump
        context = @options[:context] || Groonga::Context.default
        database = context.database
        return nil if database.nil?

        schema = ""
        database.each do |object|
          next unless object.is_a?(Groonga::Table)
          next if object.name == "<ranguba:objects>"
          schema << "create_table(#{object.name.inspect}) do |table|\n"
          object.columns.each do |column|
            type = column_method(column)
            name = column.local_name
            schema << "  table.#{type}(#{name.inspect})\n"
          end
          schema << "end\n"
        end
        schema
      end

      private
      def column_method(column)
        case column.range.name
        when "Int32"
          "integer32"
        when "Int64"
          "integer64"
        when "Uint32"
          "unsigned_integer32"
        when "Uint64"
          "unsigned_integer64"
        when "Float"
          "float"
        when "Time"
          "time"
        when "Shorttext"
          "short_text"
        when "Text"
          "text"
        when "Longtext"
          "long_text"
        else
          raise ArgumentError, "unsupported column: #{column.inspect}"
        end
      end
    end
  end
end
