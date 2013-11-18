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
        end
      end

      private
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
    end
  end
end
