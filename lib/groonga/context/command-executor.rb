# -*- coding: utf-8 -*-
#
# Copyright (C) 2012-2013  Kouhei Sutou <kou@clear-code.com>
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

require "groonga/command"
require "groonga/client/response"

module Groonga
  class Context
    class CommandExecutor
      def initialize(context)
        @context = context
      end

      def execute(name, parameters={})
        parameters = normalize_parameters(name, parameters)
        command_class = Command.find(name)
        command = command_class.new(name, parameters)
        request_id = @context.send(command.to_command_format)
        loop do
          response_id, raw_response = @context.receive
          if request_id == response_id
            response_class = Client::Response.find(command.name)
            header = [0, 0, 0]
            case command.output_type
            when :json
              body = JSON.parse(raw_response)
            else
              body = raw_response
            end
            response = response_class.new(command, header, body)
            response.raw = raw_response
            return response
          end
          # raise if request_id < response_id
        end
      end

      private
      def normalize_parameters(name, parameters)
        case name
        when "select"
          normalize_select_parameters(parameters)
        else
          parameters
        end
      end

      def normalize_select_parameters(parameters)
        table = parameters[:table]
        parameters[:table] = table.name if table.is_a?(Table)

        normalize_key(parameters, :drilldown, :drill_down)
        normalize_key(parameters,
                      :drilldown_output_columns,
                      :drill_down_output_columns)
        normalize_key(parameters, :drilldown_limit, :drill_down_limit)

        normalize_array_value(parameters, :output_columns)
        normalize_array_value(parameters, :drilldown)
        normalize_array_value(parameters, :drilldown_output_columns)

        normalize_integer_value(parameters, :limit)
        normalize_integer_value(parameters, :drilldown_limit)

        parameters
      end

      def normalize_key(parameters, real_key, alias_key)
        return unless parameters.has_key?(alias_key)
        parameters[real_key] ||= parameters.delete(alias_key)
      end

      def normalize_array_value(parameters, key)
        if parameters[key].is_a?(::Array)
          parameters[key] = parameters[key].join(", ")
        end
      end

      def normalize_integer_value(parameters, key)
        if parameters[key].is_a?(Integer)
          parameters[key] = parameters[key].to_s
        end
      end
    end
  end
end
