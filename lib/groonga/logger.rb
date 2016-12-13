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
  class Logger
    module Flags
      LABELS = {
        TIME     => "time",
        TITLE    => "title",
        MESSAGE  => "message",
        LOCATION => "location",
        PID      => "pid",
      }

      class << self
        def parse(input, base_flags)
          # TODO
          base_flags
        end

        def label(flags)
          labels = []
          LABELS.each do |flag, label|
            labels << label if (flags & flag) == flag
          end
          labels << "none" if labels.empty?
          labels.join("|")
        end
      end
    end

    class << self
      # @deprecated since 3.0.1. Use {Groonga::Logger.path}
      #   instead.
      def log_path
        path
      end

      # @deprecated since 3.0.1. Use {Groonga::Logger.path=}
      #   instead.
      def log_path=(path)
        self.path = path
      end

      # @deprecated since 3.0.1. Use {Groonga::QueryLogger.path}
      #   instead.
      def query_log_path
        QueryLogger.path
      end

      # @deprecated since 3.0.1. Use {Groonga::QueryLogger.path=}
      #   instead.
      def query_log_path=(path)
        QueryLogger.path = path
      end
    end

    def log(level, timestamp, title, message, location)
      guard do
        puts("#{timestamp}|#{mark(level)}|#{title} #{message} #{location}")
      end
    end

    def reopen
    end

    def fin
    end

    private
    def guard
      begin
        yield
      rescue Exception
        $stderr.puts("#{$!.class}: #{$!.message}")
        $stderr.puts($@)
      end
    end

    LEVEL_TO_MARK = {
      :none      => " ",
      :emergency => "E",
      :alert     => "A",
      :critical  => "C",
      :error     => "e",
      :warning   => "w",
      :notice    => "n",
      :debug     => "d",
      :dump      => "d",
    }
    def mark(level)
      LEVEL_TO_MARK[level] || "-"
    end
  end

  class FileLogger < Logger
    def initialize(file_name)
      super()
      @file = nil
      @file_name = file_name
    end

    def reopen
      guard do
        return unless @file
        @file.close
        @file = nil
      end
    end

    def fin
      guard do
        return unless @file
        @file.close
      end
    end

    private
    def ensure_open
      return if @file
      @file = File.open(@file_name, "ab")
    end

    def puts(*arguments)
      ensure_open
      @file.puts(*arguments)
      @file.flush
    end
  end

  class CallbackLogger < Logger
    def initialize(callback)
      super()
      @callback = callback
    end

    def log(level, timestamp, title, message, location)
      guard do
        @callback.call(:log, level, timestamp, title, message, location)
      end
    end

    def reopen
      guard do
        @callback.call(:reopen)
      end
    end

    def fin
      guard do
        @callback.call(:fin)
      end
    end
  end
end
