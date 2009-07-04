# -*- coding: utf-8 -*-
#
# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
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
  class Schema
    class << self
      def create_table(name, options={})
        options ||= {}

        definition = TableDefinition.new(name, options.dup)
        yield(definition)
        definition.create
      end
    end

    class TableDefinition
      def initialize(name, options)
        @name = name
        @options = options
        @table_type = table_type
      end

      def create
        table = @table_type.create(:name => @name, :path => @options[:path])
        table
      end

      private
      def table_type
        type = @options[:type]
        case type
        when :array, nil
          Groonga::Array
        when :hash
          Groonga::Hash
        when :patricia_trie
          Groonga::PatriciaTrie
        else
          raise ArgumentError, "unknown table type: #{type.inspect}"
        end
      end
    end
  end
end
