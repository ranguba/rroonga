# -*- coding: utf-8 -*-
#
# Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>
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

require 'English'

module Groonga
  class Record
    # レコードが所属するテーブル
    attr_reader :table
    # _table_ の _id_ に対応するレコードを作成する。_values_ には各
    # カラムに設定する値を以下のような形式で指定する。
    #
    #   [
    #    ["カラム名", 値],
    #    ["カラム名", 値],
    #    ...,
    #   ]
    def initialize(table, id, values=nil)
      @table = table
      @id = id
      @added = false
      if values
        values.each do |name, value|
          self[name] = value
        end
      end
    end

    # _record_ と _other_ が同じテーブルに属していて、さらに、
    # 同じレコードIDを持つなら +true+ を返し、そうでなければ
    # +false+ を返す。
    def ==(other)
      self.class == other.class and
        [table, id] == [other.table, other.id]
    end

    # Groonga::Record#==と同じ。
    def eql?(other)
      self == other
    end

    # 同じテーブルの同じIDのレコードに対しては常に同じハッシュ
    # 値を返す。
    def hash
      @table.hash ^ @id.hash
    end

    # このレコードの _column_name_ で指定されたカラムの値を返す。
    def [](column_name)
      @table.column_value(@id, column_name, :id => true)
    end

    # このレコードの _column_name_ で指定されたカラムの値を設定す
    # る。
    def []=(column_name, value)
      @table.set_column_value(@id, column_name, value, :id => true)
    end

    # このレコードの _column_name_ で指定されたカラムの値の最後に
    # _value_ を追加する。
    def append(column_name, value)
      column(column_name).append(@id, value)
    end

    # このレコードの _column_name_ で指定されたカラムの値の最初に
    # _value_ を追加する。
    def prepend(column_name, value)
      column(column_name).prepend(@id, value)
    end

    # _record_ が所属するテーブルで主キーを使える場合は +true+
    # を返し、使えない場合は +false+ を返す。
    def support_key?
      @table.support_key?
    end

    # 名前が _name_ のカラムがレコードの所属するテーブルで定義され
    # ているなら +true+ を返す。
    def have_column?(name)
      not @table.column(normalize_column_name(name)).nil?
    end

    # 名前が _name_ のカラムが参照カラムであるなら +true+ を返す。
    def reference_column?(name)
      column(name).reference?
    end

    # 名前が _name_ のカラムが索引カラム
    # （Groonga::IndexColumn）であるなら +true+ を返す。
    def index_column?(name)
      column(name).index?
    end

    # 名前が _name_ のカラムの値がベクターであるなら +true+ を返す。
    #
    # @since: 1.0.5
    def vector_column?(name)
      column(name).vector?
    end

    # 名前が _name_ のカラムの値がスカラーであるなら +true+ を返す。
    #
    # @since: 1.0.5
    def scalar_column?(name)
      column(name).scalar?
    end

    # 名前が _name_ のGroonga::IndexColumnの search メソッドを呼ぶ。
    # _query_ と _options_ はそのメソッドにそのまま渡される。詳しく
    # はGroonga::IndexColumn#searchを参照。
    def search(name, query, options={})
      column(name).search(query, options)
    end

    # レコードの主キーを返す。
    #
    # _record_ が所属するテーブルがGroonga:::Arrayの場合は常
    # に +nil+ を返す。
    def key
      if support_key?
        @key ||= @table.key(@id)
      else
        nil
      end
    end
    # レコードを一意に識別するための情報を返す。
    #
    # _record_ が所属するテーブルがGroonga:::Arrayの場合はID
    # を返し、それ以外の場合は主キーを返す。
    def record_id
      if support_key?
        key
      else
        id
      end
    end

    # レコードのIDを返す。
    def record_raw_id
      @id
    end
    alias_method :id, :record_raw_id

    # レコードのスコア値を返す。検索結果として生成されたテーブル
    # のみに定義される。
    def score
      self["_score"]
    end

    # Sets score. Score column exists only search result table.
    #
    # @param [Integer] new_score The new score.
    def score=(new_score)
      self["_score"] = new_score
    end

    # Groonga::Record#scoreが利用できる場合は +true+ を
    # 返す。
    def support_score?
      @table.have_column?("_score") # TODO delegate to Table
    end

    # 主キーの値が同一であったレコードの件数を返す。検索結果とし
    # て生成されたテーブルのみに定義される。
    #
    # Groonga::Record#support_sub_records?でこの値を利用でき
    # るかがわかる。
    def n_sub_records
      self["_nsubrecs"]
    end

    # Groonga::Record#n_sub_recordsが利用できる場合は +true+ を
    # 返す。
    def support_sub_records?
      @table.support_sub_records?
    end

    # レコードの値を返す。
    def value
      @table.value(@id, :id => true)
    end

    # レコードの値を設定する。既存の値は上書きされる。
    def value=(value)
      @table.set_value(@id, value, :id => true)
    end

    # このレコードの _name_ で指定されたカラムの値を _delta_ だけ増
    # 加する。 _delta_ が +nil+ の場合は1増加する。
    def increment!(name, delta=nil)
      column(name).increment!(@id, delta)
    end

    # このレコードの _name_ で指定されたカラムの値を _delta_ だけ減
    # 少する。 _delta_ が +nil+ の場合は1減少する。
    def decrement!(name, delta=nil)
      column(name).decrement!(@id, delta)
    end

    # レコードが所属するテーブルの全てのカラムを返す。
    def columns
      @table.columns
    end

    # レコードが所属しているテーブルで定義されているインデックス
    # 型のカラムでない全カラムを対象とし、カラムの名前をキーとし
    # たこのレコードのカラムの値のハッシュを返す。
    #
    # return same attributes object if duplicate records exist.
    def attributes
      accessor = AttributeHashBuilder.new(self)
      accessor.build
    end

    # レコードを削除する。
    def delete
      @table.delete(@id)
    end

    # レコードが所属するテーブルをロックする。ロックに失敗した場
    # 合はGroonga::ResourceDeadlockAvoided例外が発生する。
    #
    # ブロックを指定した場合はブロックを抜けたときにunlockする。
    #
    # 利用可能な _option_ は以下の通り。
    # @param [Hash] options The name and value
    #   pairs. Omitted names are initialized as the default value.
    # @option options [Integer] :timeout The timeout
    #   ロックを獲得できなかった場合は _:timeout_ 秒間ロックの獲得を試みる。
    #   _:timeout_ 秒以内にロックを獲得できなかった場合は例外が発生する。
    def lock(options={}, &block)
      @table.lock(options.merge(:id => @id), &block)
    end

    # レコードが所属するテーブルのロックを解除する。
    #
    # 利用可能なオプションは現在は無い。
    def unlock(options={})
      @table.unlock(options.merge(:id => @id))
    end

    # レコードが所属するテーブルのロックを強制的に解除する。
    #
    # 利用可能なオプションは現在は無い。
    def clear_lock(options={})
      @table.clear_lock(options.merge(:id => @id))
    end

    # レコードが所属するテーブルがロックされていれば +true+ を返す。
    #
    # 利用可能なオプションは現在は無い。
    def locked?(options={})
      @table.locked?(options.merge(:id => @id))
    end

    # レコードが持つIDが有効なIDであれば +true+ を返す。
    def valid_id?
      @table.exist?(@id)
    end

    # @private
    def methods(include_inherited=true)
      _methods = super
      return _methods unless include_inherited
      columns.each do |column|
        name = column.local_name
        _methods << name
        _methods << "#{name}="
      end
      _methods
    end

    # @private
    def respond_to?(name)
      super or !@table.column(name.to_s.sub(/=\z/, '')).nil?
    end

    def added?
      @added
    end

    # @private
    def added=(added)
      @added = added
    end

    private
    def normalize_column_name(name)
      name.to_s
    end

    # @private
    def column(name)
      _column = @table.column(normalize_column_name(name))
      raise NoSuchColumn, "column(#{name.inspect}) is nil" if _column.nil?
      _column
    end

    def method_missing(name, *args, &block)
      if /=\z/ =~ name.to_s
        base_name = $PREMATCH
        is_setter = true
      else
        base_name = name.to_s
        is_setter = false
      end
      _column = @table.column(base_name)
      if _column
        if is_setter
          _column.send("[]=", @id, *args, &block)
        else
          _column.send("[]", @id, *args, &block)
        end
      else
        super
      end
    end

    # @private
    class AttributeHashBuilder
      attr_reader :attributes

      def initialize(root_record)
        @root_record = root_record
        @all_built_attributes = {}
      end

      def build
        build_attributes(@root_record)
      end

      private
      def build_attributes(record)
        if @all_built_attributes.has_key?(record)
          return @all_built_attributes[record]
        end

        attributes = {}
        @all_built_attributes[record] = attributes

        add_id(attributes, record)
        add_key(attributes, record)
        add_score(attributes, record)
        add_n_sub_records(attributes, record)
        add_columns(attributes, record)

        attributes
      end

      def build_value(value)
        if value.is_a?(Record)
          build_attributes(value)
        else
          value
        end
      end

      def build_vector(vector)
        vector.collect do |element|
          build_value(element)
        end
      end

      def add_id(attributes, record)
        attributes["_id"] = record.id
      end

      def add_table(attributes, record)
        attributes["_table"] = record.table.name
      end

      def add_columns(attributes, record)
        record.columns.each do |column|
          next if column.is_a?(IndexColumn)

          value = column[record.id]
          attributes[column.local_name] = build_column(column, value)
        end
      end

      def build_column(column, value)
        if column.vector?
          build_vector(value)
        else
          build_value(value)
        end
      end

      def add_key(attributes, record)
        if record.support_key?
          attributes["_key"] = build_value(record.key)
        end
      end

      def add_score(attributes, record)
        if record.support_score?
          attributes["_score"] = record.score
        end
      end

      def add_n_sub_records(attributes, record)
        if record.support_sub_records?
          attributes["_nsubrecs"] = record.n_sub_records
        end
      end
    end
  end
end

