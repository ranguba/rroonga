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

require "English"
require "time"

require "json"

module Groonga
  module GrntestLog
    class EnvironmentEvent
      def initialize(environment)
        @environment = environment
      end

      def script
        @environment["script"]
      end

      def user
        @environment["user"]
      end

      def date
        Time.parse(@environment["date"])
      end

      def cpu
        @environment["cpu"]
      end

      def bit
        @environment["bit"]
      end

      def core
        @environment["core"]
      end

      def ram
        @environment["RAM"]
      end

      def unevictable
        @environment["Unevictable"]
      end

      def mlocked
        @environment["Mlocked"]
      end

      def total_memory
        parse_size_with_unit(ram)
      end

      def unevictable_memory
        parse_size_with_unit(unevictable)
      end

      def locked_memory
        parse_size_with_unit(mlocked)
      end

      def hdd
        @environment["HDD"]
      end

      def disk_size
        parse_size_with_unit(hdd)
      end

      def os
        @environment["OS"]
      end

      def host
        @environment["HOST"]
      end

      def port
        @environment["PORT"]
      end

      def version
        @environment["VERSION"]
      end

      private
      def parse_size_with_unit(size_with_unit)
        case size_with_unit
        when /\A(\d+)\z/
          $1.to_i
        when /\A(\d+)([KM])Bytes\z/
          size = $1.to_i
          unit = $2
          case unit
          when "K"
            size * 1024
          when "M"
            size * (1024 ** 2)
          else
            raise ArgumentError,
                  "unknown size unit: <#{unit}>: <#{size_with_unit}>"
          end
        else
          raise ArgumentError, "unknown size: <#{size_with_unit}>"
        end
      end
    end

    class JobsStartEvent
      attr_reader :jobs
      def initialize(jobs)
        @jobs = jobs
      end
    end

    class TaskEvent < Struct.new(:id, :command,
                                 :relative_start_time, :relative_end_time,
                                 :result)
      def elapsed_time
        relative_end_time - relative_start_time
      end
    end

    class JobSummaryEvent
      attr_reader :job, :latency, :elapsed, :qps, :min, :max, :n_queries
      def initialize(summary)
        @job = summary["job"]
        @total_elapsed_time = summary["total_elapsed_time"] || summary["latency"]
        @job_elapsed_time = summary["job_elapsed_time"] || summary["self"]
        @qps = summary["qps"]
        @min = summary["min"]
        @max = summary["max"]
        @n_queries = summary["n_queries"] || summary["queries"]
      end
    end

    class JobsEndEvent
      attr_reader :summaries
      def initialize(summaries)
        @summaries = summaries
      end
    end

    class Parser
      def initialize
      end

      def parse(input, &block)
        in_environment = false
        buffer = ""
        input.each_line do |line|
          if in_environment
            case line
            when "},\n"
              buffer << "}"
              yield(EnvironmentEvent.new(parse_json(buffer)))
              buffer.clear
              in_environment = false
            else
              buffer << line
            end
          else
            case line
            when /\A\[\{"script":/
              buffer << line[1..-1]
              in_environment = true
            when /\A\{"jobs":/
              yield(JobsStartEvent.new(parse_json(line.sub(/,$/, "}"))))
            when /\A"detail": \[$/
              # ignore
            when /\A\[\d+,/
              yield(TaskEvent.new(*parse_json(line.sub(/\]+,$/, "]"))))
            when /\A"summary": /
              summaries = parse_json(line.gsub(/(?:\A"summary": |\},$)/, ''))
              summaries = summaries.collect do |summary|
                JobSummaryEvent.new(summary)
              end
              yield(JobsEndEvent.new(summaries))
            end
          end
        end
      end

      private
      def parse_json(string)
        JSON.parse(string)
      end
    end
  end
end
