# -*- coding: utf-8 -*-
#
# Copyright (C) 2014-2015  Masafumi Yokoyama <yokoyama@clear-code.com>
# Copyright (C) 2009-2014  Kouhei Sutou <kou@clear-code.com>
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

require "English"
require "time"

require "groonga/sub-records"

module Groonga
  class Record
    # レコードが所属するテーブル
    attr_reader :table
    # _table_ の _id_ に対応するレコードを作成する。 _values_ には各
    # カラムに設定する値を以下のような形式で指定する。
    #
    # <pre>
    # !!!ruby
    # [
    #  ["カラム名", 値],
    #  ["カラム名", 値],
    #  ...,
    # ]
    # </pre>
    #
    # Each value is set by {#[]=}. See {#[]=} how to set weight for a
    # value.
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

    # {Groonga::Record#==} と同じ。
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

    # Sets column value of the record.
    #
    # @overload []=(column_name, value)
    #   @param column_name [String] The column name.
    #   @param value [Object] The column value. Weight of the value is 0.
    #
    #   @example Set a new value
    #     user["age"] = 29
    #
    # @overload []=(column_name, value_with_weight)
    #   @param column_name [String] The column name.
    #   @param value_with_weight [::Hash] The column value with weight.
    #   @option value_with_weight [Object] :value (nil) The column value.
    #   @option value_with_weight [Integer or nil] :weight (nil)
    #     The weight for the value. You need to use vector column and
    #     weight supported index column for weight. See
    #     {Groonga::Table#set_column_value} for details.
    #
    #   @example Set a new value with weight "2"
    #     user["tags"] = [{:value => "groonga", :weight => 2}]
    #
    # @see Groonga::Table#set_column_value
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

    # @return @true@ if the table that the record belongs to is
    # created with value type, @false@ otherwise.
    def support_value?
      @table.support_value?
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
    # （ {Groonga::IndexColumn} ）であるなら +true+ を返す。
    def index_column?(name)
      column(name).index?
    end

    # 名前が _name_ のカラムの値がベクターであるなら +true+ を返す。
    #
    # @since 1.0.5
    def vector_column?(name)
      column(name).vector?
    end

    # 名前が _name_ のカラムの値がスカラーであるなら +true+ を返す。
    #
    # @since 1.0.5
    def scalar_column?(name)
      column(name).scalar?
    end

    # 名前が _name_ の {Groonga::IndexColumn} の search メソッドを呼ぶ。
    # _query_ と _options_ はそのメソッドにそのまま渡される。詳しく
    # は {Groonga::IndexColumn#search} を参照。
    def search(name, query, options={})
      column(name).search(query, options)
    end

    # レコードの主キーを返す。
    #
    # _record_ が所属するテーブルが {Groonga::Array} の場合は常
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
    # _record_ が所属するテーブルが {Groonga::Array} の場合はID
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

    # {Groonga::Record#score} が利用できる場合は `true` を
    # 返す。
    def support_score?
      @table.support_score?
    end

    # 主キーの値が同一であったレコードの件数を返す。検索結果とし
    # て生成されたテーブルのみに定義される。
    #
    # {Groonga::Record#support_sub_records?} でこの値を利用でき
    # るかがわかる。
    def n_sub_records
      self["_nsubrecs"]
    end

    # {Groonga::Record#n_sub_records} が利用できる場合は +true+ を
    # 返す。
    def support_sub_records?
      @table.support_sub_records?
    end

    # The maximum integer value from integer values in grouped records.
    # It can be used when specifying _:calc_target_ and _:calc_types_
    # to {Groonga::Table#group}.
    #
    # @return [Integer] The maximum integer value from integer values in grouped records.
    #
    # @since 5.0.0
    def max
      self["_max"]
    end

    # The minimum integer value from integer values in grouped records.
    # It can be used when specifying _:calc_target_ and _:calc_types_
    # to {Groonga::Table#group}.
    #
    # @return [Integer] The minimum integer value from integer values in grouped records.
    #
    # @since 5.0.0
    def min
      self["_min"]
    end

    # The sum of integer values in grouped records.
    # It can be used when specifying _:calc_target_ and _:calc_types_
    # to {Groonga::Table#group}.
    #
    # @return [Integer] The sum of integer values in grouped records.
    #
    # @since 5.0.0
    def sum
      self["_sum"]
    end

    # The average of integer/float values in grouped records.
    # It can be used when specifying _:calc_target_ and _:calc_types_
    # to {Groonga::Table#group}.
    #
    # @return [Float] The average of integer/float values in grouped records.
    #
    # @since 5.0.0
    def average
      self["_avg"]
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

    def as_json
      accessor = AttributeHashBuilder.new(self) do |value|
        if value.is_a?(Time)
          value.iso8601
        else
          value
        end
      end
      accessor.build
    end

    # @return [String] the record formatted as JSON.
    def to_json(*args)
      as_json.to_json(*args)
    end

    # Delete the record.
    def delete
      if support_key?
        @table.delete(@id, :id => true)
      else
        @table.delete(@id)
      end
    end

    # Renames key of the record. Only available for
    # {Groonga::DoubleArrayTrie} table.
    #
    # @since 4.0.5
    def rename(new_key)
      if @table.is_a?(DoubleArrayTrie)
        @table.update(@id, new_key, :id => true)
      else
        raise OperationNotSupported, "Only Groonga::DoubleArrayTrie table supports Groonga::Record#rename: <#{@table.class}>"
      end
    end

    # レコードが所属するテーブルをロックする。ロックに失敗した場
    # 合は {Groonga::ResourceDeadlockAvoided} 例外が発生する。
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

    # Forces to clear lock of the table to which the record belongs.
    #
    # @return [void]
    def clear_lock
      @table.clear_lock
    end

    # Checks whether the table to which the record belongs is locked
    # or not.
    #
    # @return [Boolean] `true` if the table to which the record
    #   belongs is locked, `false` otherwise.
    def locked?
      @table.locked?
    end

    # レコードが持つIDが有効なIDであれば +true+ を返す。
    def valid_id?
      @table.exist?(@id)
    end

    # @return [SubRecords] Sub records of the record.
    def sub_records
      SubRecords.new(self)
    end

    # @private
    def methods(include_inherited=true)
      original_methods = super
      return original_methods unless include_inherited
      (original_methods + dynamic_methods).uniq
    end

    # @private
    def respond_to?(name, include_all=false)
      super or !@table.column(name.to_s.sub(/=\z/, '')).nil?
    end

    def added?
      @added
    end

    # @private
    def added=(added)
      @added = added
    end

    # @private
    def inspect
      if @table.closed?
        super.gsub(/>\z/, " (closed)>")
      else
        super.gsub(/>\z/, ", attributes: #{attributes.inspect}>")
      end
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

    # @private
    def dynamic_methods
      @dynamic_methods ||= compute_dynamic_methods
    end

    def compute_dynamic_methods
      methods = []

      table = @table
      while table
        table.columns.each do |column|
          name = column.local_name
          methods << name.to_sym
          methods << "#{name}=".to_sym
        end
        table = table.domain
        break unless table.is_a?(Groonga::Table)
      end

      if private_methods.first.is_a?(String)
        methods = methods.collect do |name|
          name.to_s
        end
      end

      methods
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

      def initialize(root_record, &value_translator)
        @root_record = root_record
        @all_built_attributes = {}
        @value_translator = value_translator
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
        add_value(attributes, record)
        add_score(attributes, record)
        add_n_sub_records(attributes, record)
        add_columns(attributes, record)

        attributes
      end

      def build_value(value)
        if value.is_a?(Record)
          build_attributes(value)
        else
          value = @value_translator.call(value) if @value_translator
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

      def add_value(attributes, record)
        if record.support_value?
          attributes["_value"] = build_value(record.value)
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

