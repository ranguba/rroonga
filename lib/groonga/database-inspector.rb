# -*- coding: utf-8 -*-
#
# Copyright (C) 2013  Kouhei Sutou <kou@clear-code.com>
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

module Groonga
  # It is a class that inspects database. You can know details metadata
  # of the database.
  class DatabaseInspector
    # @param database [Database] The database to be inspected.
    # @param options [Options] The options to custom this inspector behavior.
    def initialize(database, options=nil)
      @database = database
      @options = options || Options.new
    end

    # Report inspected result of the database.
    #
    # @param output [#write] (nil) The output of inspected result.
    #   If it is @nil@, @$stdout@ is used.
    def report(output=nil)
      output ||= $stdout
      reporter = Reporter.new(@database, @options, output)
      reporter.report
    end

    # It is a class that keeps options for {DatabaseInspector}.
    class Options
      # @return [Boolean] (true) Shows information about tables if true,
      #   doesn't show it otherwise.
      attr_writer :show_tables

      # @return [Boolean] (true) Shows information about columns if true,
      #   doesn't show it otherwise. If {#show_tables?} is false, information
      #   about columns isn't always shown.
      attr_writer :show_columns
      def initialize
        @show_tables = true
        @show_columns = true
      end

      # (see #show_tables=)
      def show_tables?
        @show_tables
      end

      # (see #show_columns=)
      def show_columns?
        @show_columns
      end
    end

    # @private
    class Reporter
      def initialize(database, options, output)
        @database = database
        @context = @database.context
        @options = options
        @output = output
        @indent_width = 0
      end

      def report
        write("Database\n")
        indent do
          write("Path:             #{inspect_path(@database.path)}\n")
          write("Total disk usage: " +
                "#{inspect_disk_usage(total_disk_usage)}\n")
          write("Disk usage:       " +
                "#{inspect_sub_disk_usage(@database.disk_usage)}\n")
          write("N records:        #{count_total_n_records}\n")
          write("N tables:         #{count_n_tables}\n")
          write("N columns:        #{count_total_n_columns}\n")
          report_plugins
          report_tables
        end
      end

      private
      def push_memory_pool(&block)
        @database.context.push_memory_pool(&block)
      end

      def report_plugins
        write("Plugins:\n")
        indent do
          plugin_paths = @database.plugin_paths
          if plugin_paths.empty?
            write("None\n")
            return
          end
          plugin_paths.each do |path|
            write("* #{path}\n")
          end
        end
      end

      def report_tables
        return unless @options.show_tables?
        write("Tables:\n")
        indent do
          tables = @database.tables
          if tables.empty?
            write("None\n")
            return
          end
          tables.each do |table|
            report_table(table)
          end
        end
      end

      def report_table(table)
        write("#{table.name}:\n")
        indent do
          write("ID:               #{table.id}\n")
          write("Type:             #{inspect_table_type(table)}\n")
          write("Key type:         #{inspect_key_type(table)}\n")
          write("Tokenizer:        #{inspect_tokenizer(table)}\n")
          write("Normalizer:       #{inspect_normalizer(table)}\n")
          write("Path:             #{inspect_path(table.path)}\n")
          total_table_disk_usage = count_total_table_disk_usage(table)
          write("Total disk usage: " +
                "#{inspect_sub_disk_usage(total_table_disk_usage)}\n")
          write("Disk usage:       " +
                "#{inspect_sub_disk_usage(table.disk_usage)}\n")
          write("N records:        #{table.size}\n")
          write("N columns:        #{table.columns.size}\n")
          report_columns(table)
        end
      end

      def report_columns(table)
        return unless @options.show_columns?
        write("Columns:\n")
        indent do
          columns = table.columns
          if columns.empty?
            write("None\n")
            return
          end
          columns.each do |column|
            report_column(column)
          end
        end
      end

      def report_column(column)
        write("#{column.local_name}:\n")
        indent do
          write("ID:         #{column.id}\n")
          write("Type:       #{inspect_column_type(column)}\n")
          if column.index?
            sources = column.sources
            write("N sources:  #{sources.size}\n")
            unless sources.empty?
              write("Sources:\n")
              indent do
                sources.each do |source|
                  write("Name:     #{inspect_source(source)}\n")
                end
              end
            end
          else
            write("Value type: #{inspect_value_type(column.range)}\n")
          end
          write("Path:       #{inspect_path(column.path)}\n")
          write("Disk usage: #{inspect_sub_disk_usage(column.disk_usage)}\n")
        end
      end

      def indent
        indent_width = @indent_width
        @indent_width += 2
        yield
      ensure
        @indent_width = indent_width
      end

      def write(message)
        indent = " " * @indent_width
        @output.write("#{indent}#{message}")
      end

      def inspect_path(path)
        if path.nil?
          "(null)"
        else
          "<#{path}>"
        end
      end

      KiB = (2 ** 10).to_f
      MiB = (2 ** 20).to_f
      GiB = (2 ** 30).to_f
      TiB = (2 ** 40).to_f
      PiB = (2 ** 50).to_f
      def inspect_disk_usage(disk_usage)
        if disk_usage < KiB
          "%dB" % disk_usage
        elsif disk_usage < MiB
          "%.3fKiB" % (disk_usage / KiB)
        elsif disk_usage < GiB
          "%.3fMiB" % (disk_usage / MiB)
        elsif disk_usage < TiB
          "%.3fGiB" % (disk_usage / GiB)
        elsif disk_usage < PiB
          "%.3fTiB" % (disk_usage / TiB)
        else
          "%.3fPiB" % (disk_usage / PiB)
        end
      end

      def inspect_sub_disk_usage(disk_usage)
        percent = disk_usage / total_disk_usage.to_f * 100
        "%s (%.3f%%)" % [inspect_disk_usage(disk_usage), percent]
      end

      def count_total_n_records
        @database.tables.inject(0) do |previous, table|
          previous + table.size
        end
      end

      def count_n_tables
        @database.tables.size
      end

      def count_total_n_columns
        @database.tables.inject(0) do |previous, table|
          push_memory_pool do
            previous + table.columns.size
          end
        end
      end

      def total_disk_usage
        @total_disk_usage ||= count_total_disk_usage
      end

      def count_total_disk_usage
        @database.tables.inject(@database.disk_usage) do |previous, table|
          previous + count_total_table_disk_usage(table)
        end
      end

      def count_total_table_disk_usage(table)
        push_memory_pool do
          table.columns.inject(table.disk_usage) do |previous, column|
            previous + column.disk_usage
          end
        end
      end

      def inspect_table_type(table)
        case table
        when Groonga::Array
          "array"
        when Groonga::Hash
          "hash"
        when Groonga::PatriciaTrie
          "patricia trie"
        when Groonga::DoubleArrayTrie
          "double array trie"
        else
          "unknown (#{table.class})"
        end
      end

      def inspect_key_type(table)
        if table.support_key?
          table.domain.name
        else
          "(no key)"
        end
      end

      def inspect_value_type(range)
        if range.nil?
          "(no value)"
        else
          range.name
        end
      end

      def inspect_tokenizer(table)
        if table.support_key?
          tokenizer = table.default_tokenizer
          if tokenizer
            tokenizer.name
          else
            "(no tokenizer)"
          end
        else
          "(no key)"
        end
      end

      def inspect_normalizer(table)
        if table.support_key?
          normalizer = table.normalizer
          if normalizer
            normalizer.name
          else
            "(no normalizer)"
          end
        else
          "(no key)"
        end
      end

      def inspect_column_type(column)
        if column.index?
          "index"
        elsif column.vector?
          "vector"
        else
          "scalar"
        end
      end

      def inspect_source(source)
        if source.is_a?(Table)
          "#{source.name}._key"
        else
          source.name
        end
      end
    end
  end
end
