# -*- coding: utf-8 -*-
#
# Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>
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
  class ViewRecord
    # レコードが所属するビュー
    attr_reader :view
    # レコードのID
    attr_reader :id

    # _table_の_id_に対応するレコードを作成する。
    def initialize(view, id)
      @view = view
      @id = id
    end

    # call-seq:
    #   record == other -> true/false
    #
    # _record_と_other_が同じgroongaのレコードなら+true+を返し、
    # そうでなければ+false+を返す。
    def ==(other)
      self.class == other.class and
        [view, id] == [other.view, other.id]
    end

    # call-seq:
    #   record[column_name] -> 値
    #
    # このレコードの_column_name_で指定されたカラムの値を返す。
    def [](column_name)
      @view.column_value(@id, column_name)
    end

    private
    def method_missing(name, *args, &block)
      return super if /(?:=|\?)\z/ =~ name.to_s
      return super unless args.empty?
      self[name.to_s]
    end
  end
end
