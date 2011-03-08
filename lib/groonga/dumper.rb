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

module Groonga
  # データベースの内容をgrn式形式の文字列として出力するクラス。
  class DatabaseDumper
    def initialize(output, options={})
      @output = output
      @options = options
      @context = @options[:context] || Groonga::Context.default
    end

    def dump
      tables.each do
      end
    end

    private
    def dump_records(table)
      @output.write("load --table #{table}\n")
      @output.write("[\n")
      columns = table.columns.reject do |column|
        false
      end
      column_names = columns.collect do |column|
        "\"#{column.name}\""
      end
      @output.write("[#{column_names.join(', ')}]\n")
      dump_records(table, columns)
      @output.write("]\n")
    end
  end

  # スキーマの内容をRubyスクリプトまたはgrn式形式の文字列と
  # して出力するクラス。
  class SchemaDumper
    def initialize(options={})
      @options = (options || {}).dup
    end

    def dump
      context = @options[:context] || Groonga::Context.default
      database = context.database
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
    end
  end

  class TableDumper
  end
end
