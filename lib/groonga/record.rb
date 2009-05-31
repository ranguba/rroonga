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
  class Record
    attr_reader :table, :id
    def initialize(table, id, values=nil)
      @table = table
      @id = id
      if values
        values.each do |name, value|
          self[name] = value
        end
      end
    end

    def ==(other)
      self.class == other.class and
        [table, id] == [other.table, other.id]
    end

    def [](column_name)
      (column(column_name) || {})[@id]
    end

    def []=(column_name, value)
      column(column_name, true)[@id] = value
    end

    def append(column_name, value)
      column(column_name, true).append(@id, value)
    end

    def have_column?(name)
      not column(name).nil?
    end

    def reference_column?(name)
      column(name, true).range.is_a?(Groonga::Table)
    end

    def search(name, query, options={})
      column(name, true).search(query, options)
    end

    def key
      @table.key(@id)
    end

    def score
      self[".:score"]
    end

    def value
      @table[@id]
    end

    def value=(value)
      @table[@id] = value
    end

    def columns
      @table.columns
    end

    def attributes
      attributes = {}
      table_name = @table.name
      columns.each do |column|
        next if column.is_a?(Groonga::IndexColumn)
        attributes[column.name[(table_name.size + 1)..-1]] = column[@id]
      end
      attributes
    end

    def delete
      @table.delete(@id)
    end

    private
    def column(name, required=false)
      _column = @table.column(name.to_s)
      if _column.nil? and required
        raise Groonga::Error, "nonexistent column: <#{name.inspect}>"
      end
      _column
    end
  end
end
