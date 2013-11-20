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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

module Groonga
  # It is a class that inspects database. You can know details metadata
  # of the database.
  class DatabaseInspector
    # @param database [Database] The database to be inspected.
    def initialize(database)
      @database = database
    end

    # Report inspected result of the database.
    #
    # @param output [#write] (nil) The output of inspected result.
    #   If it is @nil@, @$stdout@ is used.
    def report(output=nil)
      output ||= $stdout
      reporter = Reporter.new(@database, output)
      reporter.report
    end

    # @private
    class Reporter
      def initialize(database, output)
        @database = database
        @context = @database.context
        @output = output
        @indent_width = 0
      end

      def report
        write("Database\n")
        indent do
          write("Path:       #{inspect_path(@database.path)}\n")
          write("Disk usage: #{inspect_disk_usage(@database.disk_usage)}\n")
          write("N records:  #{count_total_n_records}\n")
          write("N tables:   #{count_n_tables}\n")
          write("N columns:  #{count_total_n_columns}\n")
          report_plugins
          report_tables
        end
      end

      private
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
          write("ID:         #{table.id}\n")
          write("Type:       #{inspect_table_type(table)}\n")
          write("Key type:   #{inspect_key_type(table)}\n")
          write("Tokenizer:  #{inspect_tokenizer(table)}\n")
          write("Normalizer: #{inspect_normalizer(table)}\n")
          write("Path:       #{inspect_path(table.path)}\n")
          write("Disk usage: #{inspect_disk_usage(table.disk_usage)}\n")
          write("N records:  #{table.size}\n")
          write("N columns:  #{table.columns.size}\n")
          report_columns(table)
        end
      end

      def report_columns(table)
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
          write("Value type: #{inspect_column_value_type(column)}\n")
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
          previous + table.columns.size
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

      def inspect_column_value_type(column)
        column.domain.name
      end
    end
  end
end
