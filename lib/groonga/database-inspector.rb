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
          write("path: #{inspect_path(@database.path)}\n")
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
    end
  end
end
