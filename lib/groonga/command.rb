# -*- coding: utf-8 -*-
#
# Copyright (C) 2012  Kouhei Sutou <kou@clear-code.com>
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
  module Command
    class Builder
      class << self
        def escape_value(value)
          escaped_value = value.to_s.gsub(/"/, '\\"')
          "\"#{escaped_value}\""
        end
      end

      attr_reader :command, :arguments
      def initialize(command, arguments={})
        @command = command
        @arguments = arguments
      end

      def [](key)
        @arguments[key]
      end

      def []=(key, value)
        @arguments[key] = value
      end

      def build
        query = "#{@command} "
        @arguments.each do |key, value|
          value = value.join(", ") if value.is_a?(::Array)
          escaped_value = self.class.escape_value(value)
          query << " --#{key} #{escaped_value}"
        end
        query
      end
    end

    class Select
      def initialize(context, table, options)
        @context = context
        @table = table
        @options = normalize_options(options)
      end

      def exec
        request_id = @context.send(query)
        loop do
          response_id, result = @context.receive
          if request_id == response_id
            drill_down_keys = @options["drilldown"]
            if drill_down_keys.is_a?(String)
              drill_down_keys = drill_down_keys.split(/(?:\s+|\s*,\s*)/)
            end
            return Result.parse(result, drill_down_keys)
          end
          # raise if request_id < response_id
        end
      end

      private
      def normalize_options(options)
        normalized_options = {}
        options.each do |key, value|
          normalized_options[normalize_option_name(key)] = value
        end
        normalized_options
      end

      def normalize_option_name(name)
        name = name.to_s.gsub(/-/, "_").gsub(/drill_down/, "drilldown")
        name.gsub(/sort_by/, 'sortby')
      end

      def query
        if @table.is_a?(String)
          table_name = @table
        else
          table_name = @table.name
        end
        builder = Builder.new("select", @options.merge(:table => table_name))
        builder.build
      end

      class Result < Struct.new(:n_hits, :columns, :values, :drill_down)
        class << self
          def parse(json, drill_down_keys)
            select_result, *drill_down_results = JSON.parse(json)
            result = new
            n_hits, columns, values = extract_result(select_result)
            result.n_hits = n_hits
            result.columns = columns
            result.values = values
            if drill_down_results
              result.drill_down = parse_drill_down_results(drill_down_results,
                                                           drill_down_keys)
            end
            result
          end

          def create_records(columns, values)
            records = []
            values.each do |value|
              record = {}
              columns.each_with_index do |(name, type), i|
                record[name] = convert_value(value[i], type)
              end
              records << record
            end
            records
          end

          private
          def parse_drill_down_results(results, keys)
            named_results = {}
            results.each_with_index do |drill_down, i|
              n_hits, columns, values = extract_result(drill_down)
              drill_down_result = DrillDownResult.new
              drill_down_result.n_hits = n_hits
              drill_down_result.columns = columns
              drill_down_result.values = values
              named_results[keys[i]] = drill_down_result
            end
            named_results
          end

          def extract_result(result)
            meta_data, columns, *values = result
            n_hits, = meta_data
            [n_hits, columns, values]
          end

          def convert_value(value, type)
            case type
            when "Time"
              Time.at(value)
            else
              value
            end
          end
        end

        def records
          @records ||= self.class.create_records(columns, values)
        end

        class DrillDownResult < Struct.new(:n_hits, :columns, :values)
          def records
            @records ||= Result.create_records(columns, values)
          end
        end
      end
    end
  end
end
