# -*- coding: utf-8 -*-
#
# Copyright (C) 2011  Kouhei Sutou <kou@clear-code.com>
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

require 'stringio'

module Groonga
  # データベースの内容をgrn式形式の文字列として出力するクラス。
  class DatabaseDumper
    def initialize(options={})
      @options = options
    end

    def dump
      options = @options.dup
      have_output = !@options[:output].nil?
      options[:output] ||= StringIO.new
      output = options[:output]
      database = options[:database]
      if database.nil?
        options[:context] ||= Groonga::Context.default
        database = options[:database] = options[:context].database
      end

      dump_schema(options)
      database.each(:order_by => :key) do |object|
        next unless object.is_a?(Groonga::Table)
        next if object.size.zero?
        next if lexicon_table?(object)
        output.write("\n")
        dump_records(object, options)
      end

      if have_output
        nil
      else
        options[:output].string
      end
    end

    private
    def dump_schema(options)
      SchemaDumper.new(options.merge(:syntax => :command)).dump
    end

    def dump_records(table, options)
      TableDumper.new(table, options).dump
    end

    def lexicon_table?(table)
      table.default_tokenizer and
        table.columns.any? {|column| column.index?}
    end
  end

  # スキーマの内容をRubyスクリプトまたはgrn式形式の文字列と
  # して出力するクラス。
  class SchemaDumper
    def initialize(options={})
      @options = (options || {}).dup
    end

    def dump
      database = @options[:database]
      if database.nil?
        context = @options[:context] || Groonga::Context.default
        database = context.database
      end
      return nil if database.nil?

      output = @options[:output]
      have_output = !output.nil?
      output ||= StringIO.new
      result = syntax(database, output).dump
      if have_output
        result
      else
        output.string
      end
    end

    private
    def syntax(database, output)
      case @options[:syntax]
      when :command
        CommandSyntax.new(database, output)
      else
        RubySyntax.new(database, output)
      end
    end

    class BaseSyntax # :nodoc:
      def initialize(database, output)
        @database = database
        @output = output
        @table_defined = false
        @index_columns = []
        @reference_columns = []
      end

      def dump
        header
        dump_schema
        footer
      end

      def dump_schema
        @database.each do |object|
          create_table(object) if object.is_a?(Groonga::Table)
        end

        @reference_columns.group_by do |column|
          column.table
        end.each do |table, columns|
          change_table(table) do
            columns.each do |column|
              define_reference_column(table, column)
            end
          end
        end

        @index_columns.group_by do |column|
          column.table
        end.each do |table, columns|
          change_table(table) do
            columns.each do |column|
              define_index_column(table, column)
            end
          end
        end
      end

      private
      def write(content)
        @output.write(content)
      end

      def header
        write("")
      end

      def footer
        write("")
      end

      def table_separator
        write("\n")
      end

      def create_table(table)
        table_separator if @table_defined
        create_table_header(table)
        table.columns.sort_by {|column| column.local_name}.each do |column|
          if column.is_a?(Groonga::IndexColumn)
            @index_columns << column
          else
            if column.range.is_a?(Groonga::Table)
              @reference_columns << column
            else
              define_column(table, column)
            end
          end
        end
        create_table_footer(table)
        @table_defined = true
      end

      def change_table(table)
        table_separator if @table_defined
        change_table_header(table)
        yield(table)
        change_table_footer(table)
        @table_defined = true
      end
    end

    class RubySyntax < BaseSyntax # :nodoc:
      private
      def create_table_header(table)
        parameters = []
        unless table.is_a?(Groonga::Array)
          case table
          when Groonga::Hash
            parameters << ":type => :hash"
          when Groonga::PatriciaTrie
            parameters << ":type => :patricia_trie"
          end
          if table.domain
            parameters << ":key_type => #{table.domain.name.dump}"
            if table.normalize_key?
              parameters << ":key_normalize => true"
            end
          end
          default_tokenizer = table.default_tokenizer
          if default_tokenizer
            parameters << ":default_tokenizer => #{default_tokenizer.name.dump}"
          end
        end
        parameters << ":force => true"
        parameters.unshift("")
        parameters = parameters.join(",\n             ")
        write("create_table(#{table.name.dump}#{parameters}) do |table|\n")
      end

      def create_table_footer(table)
        write("end\n")
      end

      def change_table_header(table)
        write("change_table(#{table.name.inspect}) do |table|\n")
      end

      def change_table_footer(table)
        write("end\n")
      end

      def define_column(table, column)
        type = column_method(column)
        name = column.local_name
        write("  table.#{type}(#{name.inspect})\n")
      end

      def define_reference_column(table, column)
        name = column.local_name
        reference = column.range
        write("  table.reference(#{name.dump}, #{reference.name.dump})\n")
      end

      def define_index_column(table, column)
        target_table_name = column.range.name
        sources = column.sources
        source_names = sources.collect do |source|
          if source.is_a?(table.class)
            "_key".dump
          else
            source.local_name.dump
          end
        end.join(", ")
        arguments = [target_table_name.dump,
                     sources.size == 1 ? source_names : "[#{source_names}]",
                     ":name => #{column.local_name.dump}"]
        write("  table.index(#{arguments.join(', ')})\n")
      end

      def column_method(column)
        range = column.range
        case range.name
        when "Int32"
          "integer32"
        when "Int64"
          "integer64"
        when "UInt32"
          "unsigned_integer32"
        when "UInt64"
          "unsigned_integer64"
        when "Float"
          "float"
        when "Time"
          "time"
        when "ShortText"
          "short_text"
        when "Text"
          "text"
        when "LongText"
          "long_text"
        else
          raise ArgumentError, "unsupported column: #{column.inspect}"
        end
      end
    end

    class CommandSyntax < BaseSyntax # :nodoc:
      private
      def create_table_header(table)
        parameters = []
        flags = []
        case table
        when Groonga::Array
          flags << "TABLE_NO_KEY"
        when Groonga::Hash
          flags << "TABLE_HASH_KEY"
          if table.domain and table.normalize_key?
            flags << "KEY_NORMALIZE"
          end
        when Groonga::PatriciaTrie
          flags << "TABLE_PAT_KEY"
          if table.domain and table.register_key_with_sis?
            flags << "KEY_WITH_SIS"
          end
        end
        parameters << "#{flags.join('|')}"
        if table.domain
          parameters << "--key_type #{table.domain.name}"
        end
        if table.range
          parameters << "--value_type #{table.range.name}"
        end
        if table.domain
          default_tokenizer = table.default_tokenizer
          if default_tokenizer
            parameters << "--default_tokenizer #{default_tokenizer.name}"
          end
        end
        write("table_create #{table.name} #{parameters.join(' ')}\n")
      end

      def create_table_footer(table)
      end

      def change_table_header(table)
      end

      def change_table_footer(table)
      end

      def define_column(table, column)
        parameters = []
        parameters << table.name
        parameters << column.local_name
        flags = []
        if column.scalar?
          flags << "COLUMN_SCALAR"
        elsif column.vector?
          flags << "COLUMN_VECTOR"
        end
        # TODO: support COMPRESS_ZLIB and COMPRESS_LZO?
        parameters << "#{flags.join('|')}"
        parameters << "#{column.range.name}"
        write("column_create #{parameters.join(' ')}\n")
      end

      def define_reference_column(table, column)
        define_column(table, column)
      end

      def define_index_column(table, column)
        parameters = []
        parameters << table.name
        parameters << column.local_name
        flags = []
        flags << "COLUMN_INDEX"
        flags << "WITH_SECTION" if column.with_section?
        flags << "WITH_WEIGHT" if column.with_weight?
        flags << "WITH_POSITION" if column.with_position?
        parameters << "#{flags.join('|')}"
        parameters << "#{column.range.name}"
        source_names = column.sources.collect do |source|
          if source.is_a?(table.class)
            "_key"
          else
            source.local_name
          end
        end
        parameters << "#{source_names.join(',')}" unless source_names.empty?
        write("column_create #{parameters.join(' ')}\n")
      end
    end
  end

  class TableDumper
    def initialize(table, options={})
      @table = table
      @options = options
      @output = @options[:output]
      @have_output = !@output.nil?
      @output ||= StringIO.new
    end

    def dump
      dump_load_command
      if @have_output
        nil
      else
        @output.string
      end
    end

    private
    def write(content)
      @output.write(content)
    end

    def dump_load_command
      write("load --table #{@table.name}\n")
      write("[\n")
      columns = available_columns
      dump_columns(columns)
      dump_records(columns)
      write("\n]\n")
    end

    def dump_columns(columns)
      column_names = columns.collect do |column|
        column.local_name
      end
      write(column_names.to_json)
    end

    def dump_records(columns)
      @table.each do |record|
        write(",\n")
        values = columns.collect do |column|
          resolve_value(column[record.id])
        end
        write(values.to_json)
      end
    end

    def resolve_value(value)
      case value
      when ::Array
        value.collect do |v|
          resolve_value(v)
        end
      when Groonga::Record
        if value.support_key?
          value = value.key
        else
          value = value.id
        end
        resolve_value(value)
      when Time
        value.utc.strftime("%Y-%m-%d %H:%M:%S.%6N")
      when NilClass
        # TODO: remove me. nil reference column value
        # doesn't accept null.
        ""
      else
        value
      end
    end

    def available_columns
      columns = []
      if @table.support_key?
        columns << @table.column("_key")
      else
        columns << @table.column("_id")
      end
      columns << @table.column("_value") unless @table.range.nil?
      data_columns = @table.columns.reject do |column|
        column.index?
      end
      sorted_columns = data_columns.sort_by do |column|
        column.local_name
      end
      columns.concat(sorted_columns)
      columns
    end
  end
end
