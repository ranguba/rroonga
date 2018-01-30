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

module Groonga
  class Database
    # @return [Array<Groonga::Table>] tables defined in the database.
    def tables
      options = {
        :ignore_missing_object => true,
        :order_by => :key,
      }
      each(options).find_all do |object|
        object.is_a?(Groonga::Table)
      end
    end

    # @return [Array<String>] registered plugin paths.
    def plugin_paths
      processed_paths = {}
      paths = []
      each(:ignore_missing_object => true, :order_by => :id) do |object|
        next unless object.is_a?(Groonga::Procedure)
        next if object.builtin?
        path = object.path
        next if path.nil?
        next if processed_paths.has_key?(path)
        processed_paths[path] = true
        paths << path
      end
      paths
    end

    def dump_index(output_directory)
      each do |object|
        next unless object.is_a?(Groonga::IndexColumn)
        object.dump(output_directory)
      end
    end
  end
end
