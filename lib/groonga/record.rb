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
      column(column_name)[@id]
    end

    def []=(column_name, value)
      column(column_name)[@id] = value
    end

    def append(column_name, value)
      column(column_name).append(@id, value)
    end

    def prepend(column_name, value)
      column(column_name).prepend(@id, value)
    end

    def have_column?(name)
      column(name).is_a?(Groonga::Column)
    rescue Groonga::InvalidArgument
      false
    end

    def reference_column?(name)
      column(name).range.is_a?(Groonga::Table)
    end

    def search(name, query, options={})
      column(name).search(query, options)
    end

    # call-seq:
    #   record.key -> 主キー
    #
    # レコードの主キーを返す。
    def key
      @table.key(@id)
    end

    def score
      self[".:score"]
    end

    def n_sub_records
      self[".:nsubrecs"]
    end

    # call-seq:
    #   record.value -> 値
    #
    # レコードの値を返す。
    def value
      @table[@id]
    end

    # call-seq:
    #   record.value = 値
    #
    # レコードの値を設定する。既存の値は上書きされる。
    def value=(value)
      @table[@id] = value
    end

    # call-seq:
    #   record.increment!(name, delta=nil)
    #
    # このレコードの_name_で指定されたカラムの値を_delta_だけ増
    # 加する。_delta_が+nil+の場合は1増加する。
    def increment!(name, delta=nil)
      column(name).increment!(@id, delta)
    end

    # call-seq:
    #   record.decrement!(name, delta=nil)
    #
    # このレコードの_name_で指定されたカラムの値を_delta_だけ減
    # 少する。_delta_が+nil+の場合は1減少する。
    def decrement!(name, delta=nil)
      column(name).decrement!(@id, delta)
    end

    # call-seq:
    #   record.columns -> Groonga::Columnの配列
    #
    # レコードが所属するテーブルの全てのカラムを返す。
    def columns
      @table.columns
    end

    def attributes
      attributes = {}
      table_name = @table.name
      columns.each do |column|
        next if column.is_a?(Groonga::IndexColumn)
        attributes[column.local_name] = column[@id]
      end
      attributes
    end

    def delete
      @table.delete(@id)
    end

    # call-seq:
    #   record.lock(options={})
    #   record.lock(options={}) {...}
    #
    # レコードが所属するテーブルをロックする。ロックに失敗した場
    # 合はGroonga::ResourceDeadlockAvoided例外が発生する。
    #
    # ブロックを指定した場合はブロックを抜けたときにunlockする。
    #
    # 利用可能なオプションは以下の通り。
    #
    # [_:timeout_]
    #   ロックを獲得できなかった場合は_:timeout_秒間ロックの獲
    #   得を試みる。_:timeout_秒以内にロックを獲得できなかった
    #   場合は例外が発生する。
    def lock(options={}, &block)
      @table.lock(options.merge(:id => @id), &block)
    end

    # call-seq:
    #   record.unlock(options={})
    #
    # レコードが所属するテーブルのロックを解除する。
    #
    # 利用可能なオプションは現在は無い。
    def unlock(options={})
      @table.unlock(options.merge(:id => @id))
    end

    # call-seq:
    #   record.clear_lock(options={})
    #
    # レコードが所属するテーブルのロックを強制的に解除する。
    #
    # 利用可能なオプションは現在は無い。
    def clear_lock(options={})
      @table.clear_lock(options.merge(:id => @id))
    end

    # call-seq:
    #   record.locked?(options={}) -> true/false
    #
    # レコードが所属するテーブルがロックされていれば+true+を返す。
    #
    # 利用可能なオプションは現在は無い。
    def locked?(options={})
      @table.locked?(options.merge(:id => @id))
    end

    private
    def column(name)
      _column = @table.column(name.to_s)
      raise InvalidArgument, "column(#{name.inspect}) is nil" if _column.nil?
      _column
    end
  end
end
