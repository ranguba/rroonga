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
require "shellwords"
require "cgi"

module Groonga
  module QueryLog
    class Command
      class << self
        @@registered_commands = {}
        def register(name, klass)
          @@registered_commands[name] = klass
        end

        def parse(input)
          if input.start_with?("/d/")
            parse_uri_path(input)
          else
            parse_command_line(input)
          end
        end

        private
        def parse_uri_path(path)
          name, parameters_string = path.split(/\?/, 2)
          parameters = {}
          if parameters_string
            parameters_string.split(/&/).each do |parameter_string|
              key, value = parameter_string.split(/\=/, 2)
              parameters[key] = CGI.unescape(value)
            end
          end
          name = name.gsub(/\A\/d\//, '')
          name, output_type = name.split(/\./, 2)
          parameters["output_type"] = output_type if output_type
          command_class = @@registered_commands[name] || self
          command = command_class.new(name, parameters)
          command.original_format = :uri
          command
        end

        def parse_command_line(command_line)
          name, *options = Shellwords.shellwords(command_line)
          parameters = {}
          options.each_slice(2) do |key, value|
            parameters[key.gsub(/\A--/, '')] = value
          end
          command_class = @@registered_commands[name] || self
          command = command_class.new(name, parameters)
          command.original_format = :command
          command
        end
      end

      attr_reader :name, :parameters
      attr_accessor :original_format
      def initialize(name, parameters)
        @name = name
        @parameters = parameters
        @original_format = nil
      end

      def ==(other)
        other.is_a?(self.class) and
          @name == other.name and
          @parameters == other.parameters
      end

      def uri_format?
        @original_format == :uri
      end

      def command_format?
        @original_format == :command
      end

      def to_uri_format
        path = "/d/#{@name}"
        parameters = @parameters.dup
        output_type = parameters.delete("output_type")
        path << ".#{output_type}" if output_type
        unless parameters.empty?
          sorted_parameters = parameters.sort_by do |name, _|
            name.to_s
          end
          uri_parameters = sorted_parameters.collect do |name, value|
            "#{CGI.escape(name)}=#{CGI.escape(value)}"
          end
          path << "?"
          path << uri_parameters.join("&")
        end
        path
      end

      def to_command_format
        command_line = [@name]
        sorted_parameters = @parameters.sort_by do |name, _|
          name.to_s
        end
        sorted_parameters.each do |name, value|
          escaped_value = value.gsub(/[\n"\\]/) do
            special_character = $MATCH
            case special_character
            when "\n"
              "\\n"
            else
              "\\#{special_character}"
            end
          end
          command_line << "--#{name}"
          command_line << "\"#{escaped_value}\""
        end
        command_line.join(" ")
      end
    end

    class SelectCommand < Command
      register("select", self)

      def sortby
        @parameters["sortby"]
      end

      def scorer
        @parameters["scorer"]
      end

      def query
        @parameters["query"]
      end

      def filter
        @parameters["filter"]
      end

      def conditions
        @conditions ||= filter.split(/(?:&&|&!|\|\|)/).collect do |condition|
          condition = condition.strip
          condition = condition.gsub(/\A[\s\(]*/, '')
          condition = condition.gsub(/[\s\)]*\z/, '') unless /\(/ =~ condition
          condition
        end
      end

      def drilldowns
        @drilldowns ||= (@parameters["drilldown"] || "").split(/\s*,\s*/)
      end

      def output_columns
        @parameters["output_columns"]
      end
    end

    class Statistic
      attr_reader :context_id, :start_time, :raw_command
      attr_reader :elapsed, :return_code
      attr_accessor :slow_operation_threshold, :slow_response_threshold
      def initialize(context_id)
        @context_id = context_id
        @start_time = nil
        @command = nil
        @raw_command = nil
        @operations = []
        @elapsed = nil
        @return_code = 0
        @slow_operation_threshold = 0.1
        @slow_response_threshold = 0.2
      end

      def start(start_time, command)
        @start_time = start_time
        @raw_command = command
      end

      def finish(elapsed, return_code)
        @elapsed = elapsed
        @return_code = return_code
      end

      def command
        @command ||= Command.parse(@raw_command)
      end

      def elapsed_in_seconds
        nano_seconds_to_seconds(@elapsed)
      end

      def last_time
        @start_time + elapsed_in_seconds
      end

      def slow?
        elapsed_in_seconds >= @slow_response_threshold
      end

      def each_operation
        previous_elapsed = 0
        ensure_parse_command
        operation_context_context = {
          :filter_index => 0,
          :drilldown_index => 0,
        }
        @operations.each_with_index do |operation, i|
          relative_elapsed = operation[:elapsed] - previous_elapsed
          relative_elapsed_in_seconds = nano_seconds_to_seconds(relative_elapsed)
          previous_elapsed = operation[:elapsed]
          parsed_operation = {
            :i => i,
            :elapsed => operation[:elapsed],
            :elapsed_in_seconds => nano_seconds_to_seconds(operation[:elapsed]),
            :relative_elapsed => relative_elapsed,
            :relative_elapsed_in_seconds => relative_elapsed_in_seconds,
            :name => operation[:name],
            :context => operation_context(operation[:name],
                                          operation_context_context),
            :n_records => operation[:n_records],
            :slow? => slow_operation?(relative_elapsed_in_seconds),
          }
          yield parsed_operation
        end
      end

      def add_operation(operation)
        @operations << operation
      end

      def operations
        _operations = []
        each_operation do |operation|
          _operations << operation
        end
        _operations
      end

      def select_command?
        command.name == "select"
      end

      private
      def nano_seconds_to_seconds(nano_seconds)
        nano_seconds / 1000.0 / 1000.0 / 1000.0
      end

      def operation_context(label, context)
        case label
        when "filter"
          if @select_command.query and context[:query_used].nil?
            context[:query_used] = true
            "query: #{@select_command.query}"
          else
            index = context[:filter_index]
            context[:filter_index] += 1
            @select_command.conditions[index]
          end
        when "sort"
          @select_command.sortby
        when "score"
          @select_command.scorer
        when "output"
          @select_command.output_columns
        when "drilldown"
          index = context[:drilldown_index]
          context[:drilldown_index] += 1
          @select_command.drilldowns[index]
        else
          nil
        end
      end

      def ensure_parse_command
        return unless select_command?
        @select_command = SelectCommand.parse(@raw_command)
      end

      def slow_operation?(elapsed)
        elapsed >= @slow_operation_threshold
      end
    end

    class Parser
      def initialize
      end

      def parse(input, &block)
        current_statistics = {}
        input.each_line do |line|
          case line
          when /\A(\d{4})-(\d\d)-(\d\d) (\d\d):(\d\d):(\d\d)\.(\d+)\|(.+?)\|([>:<])/
            year, month, day, hour, minutes, seconds, micro_seconds =
              $1, $2, $3, $4, $5, $6, $7
            context_id = $8
            type = $9
            rest = $POSTMATCH.strip
            time_stamp = Time.local(year, month, day, hour, minutes, seconds,
                                    micro_seconds)
            parse_line(current_statistics,
                       time_stamp, context_id, type, rest, &block)
          end
        end
      end

      private
      def parse_line(current_statistics,
                     time_stamp, context_id, type, rest, &block)
        case type
        when ">"
          statistic = Statistic.new(context_id)
          statistic.start(time_stamp, rest)
          current_statistics[context_id] = statistic
        when ":"
          return unless /\A(\d+) (.+)\((\d+)\)/ =~ rest
          elapsed = $1
          name = $2
          n_records = $3.to_i
          statistic = current_statistics[context_id]
          return if statistic.nil?
          statistic.add_operation(:name => name,
                                  :elapsed => elapsed.to_i,
                                  :n_records => n_records)
        when "<"
          return unless /\A(\d+) rc=(\d+)/ =~ rest
          elapsed = $1
          return_code = $2
          statistic = current_statistics.delete(context_id)
          return if statistic.nil?
          statistic.finish(elapsed.to_i, return_code.to_i)
          block.call(statistic)
        end
      end
    end
  end
end
