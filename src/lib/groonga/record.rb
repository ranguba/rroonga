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
    def initialize(table, id)
      @table = table
      @id = id
    end

    def ==(other)
      self.class == other.class and
        [table, id] == [other.table, other.id]
    end

    def [](column_name)
      column = @table.column(column_name.to_s)
      return nil if column.nil?
      column[@id]
    end

    def []=(column_name, value)
      column = @table.column(column_name.to_s)
      if column.nil?
        raise Groonga::Error, "nonexistent column: <#{column_name.inspect}>"
      end
      column[@id] = value
    end

    def have_column?(name)
      not @table.column(name.to_s).nil?
    end
  end
end
