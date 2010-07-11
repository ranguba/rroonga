# -*- coding: utf-8 -*-
#
# Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>
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

  # groongaのスキーマ（データ構造）を管理するクラス。
  #
  # Groonga::Schemaを使うことにより簡単にテーブルやカラムを
  # 追加・削除することができる。
  #
  # http://qwik.jp/senna/senna2.files/rect4605.png
  # のようなスキーマを定義する場合は以下のようになる。
  #
  #   Groonga::Schema.define do |schema|
  #     schema.create_table("Items") do |table|
  #       table.short_text("title")
  #     end
  #
  #     schema.create_table("Users") do |table|
  #       table.short_text("name")
  #     end
  #
  #     schema.create_table("comments") do |table|
  #       table.reference("item", "Items")
  #       table.reference("author", "Users")
  #       table.text("content")
  #       table.time("issued")
  #     end
  #   end
  class Schema
    # スキーマ操作で発生する例外のスーパークラス。
    class Error < Groonga::Error
    end

    # テーブルが存在しないときに発生する。
    class TableNotExists < Error
      attr_reader :name
      def initialize(name)
        @name = name
        super("table doesn't exist: <#{@name}>")
      end
    end

    # すでに存在するテーブルと違うオプションでテーブルを作ろ
    # うとしたときに発生する。
    class TableCreationWithDifferentOptions < Error
      attr_reader :table, :options
      def initialize(table, options)
        @table = table
        @options = options
        super("creating table with differnt options: " +
              "#{@table.inspect}: #{@options.inspect}")
      end
    end

    # すでに存在するカラムと違うオプションでテーブルを作ろ
    # うとしたときに発生する。
    class ColumnCreationWithDifferentOptions < Error
      attr_reader :column, :options
      def initialize(column, options)
        @column = column
        @options = options
        super("creating column with differnt option: " +
              "#{@column.inspect}: #{@options.inspect}")
      end
    end

    # 未知のインデックス対象を指定したときに発生する。
    class UnknownIndexTarget < Error
      attr_reader :target_name
      def initialize(target_name)
        @target_name = target_name
        super("unknown index target: <#{@target_name}>")
      end
    end

    # 未知のオプションを指定したときに発生する。
    class UnknownOptions < Error
      attr_reader :options, :unknown_keys, :available_keys
      def initialize(options, unknown_keys, available_keys)
        @options = options
        @unknown_keys = unknown_keys
        @available_keys = available_keys
        message = "unknown keys are specified: #{u@nknown_keys.inspect}"
        message << ": available keys: #{@available_keys.inspect}"
        message << ": options: #{@options.inspect}"
        super(message)
      end
    end

    # 未知のテーブルの種類を指定したときに発生する。
    class UnknownTableType < Error
      attr_reader :type, :available_types
      def initialize(type, available_types)
        @type = type
        @available_types = available_types
        super("unknown table type: #{@type.inspect}: " +
              "available types: #{@available_types.inspect}")
      end
    end

    class << self

      # call-seq:
      #   Groonga::Schema.define(options={}) {|schema| ...}
      #
      # スキーマを定義する。ブロックにはGroonga::Schemaオブ
      # ジェクトがわたるので、そのオブジェクトを利用してスキー
      # マを定義する。以下の省略形。
      #
      #  schema = Groonga::Scheme.new(options)
      #  ...
      #  schema.define
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:context+]
      #   スキーマ定義時に使用するGroonga::Contextを指定する。
      #   省略した場合はGroonga::Context.defaultを使用する。
      def define(options={})
        schema = new(options)
        yield(schema)
        schema.define
      end

      # call-seq:
      #   Groonga::Schema.create_table(name, options={}) {|table| ...}
      #
      # 名前が_name_のテーブルを作成する。以下の省略形。
      #
      #   Groonga::Schema.define do |schema|
      #     schema.create_table(name, options) do |table|
      #       ...
      #     end
      #   end
      #
      # ブロックにはGroonga::Schema::TableDefinitionオブジェ
      # クトがわたるので、そのオブジェクトを利用してテーブル
      # の詳細を定義する。
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:force+]
      #   +true+を指定すると既存の同名のテーブルが存在してい
      #   ても、強制的にテーブルを作成する。
      #
      # [+:type+]
      #   テーブルの型を指定する。+:array+, +:hash+,
      #   +:patricia_trie+のいずれかを指定する。デフォルトで
      #   は+:array+になる。
      #
      # [+:context+]
      #   スキーマ定義時に使用するGroonga::Contextを指定する。
      #   省略した場合はGroonga::Context.defaultを使用する。
      #
      # [+:path+]
      #   テーブルを保存するパスを指定する。パスを指定すると
      #   永続テーブルになる。
      #
      # [+:persistent+]
      #   テーブルを永続テーブルとする。+:path:+を省略した場
      #   合はパス名は自動的に作成される。デフォルトでは永続
      #   テーブルとなる。
      #
      # [+:value_type+]
      #   値の型を指定する。省略すると値のための領域を確保しない。
      #   値を保存したい場合は必ず指定すること。
      #
      # [+:sub_records+]
      #   +true+を指定するとGroonga::Table#groupでグループ化
      #   したときに、Groonga::Record#n_sub_recordsでグルー
      #   プに含まれるレコードの件数を取得できる。
      #
      # 以下は+:type+に+:hash+あるいは+:patricia_trie+を指定
      # した時に指定可能。
      #
      # [+:key_type+]
      #   キーの種類を示すオブジェクトを指定する。キーの種類
      #   には型名（"Int32"や"ShortText"など）または
      #   Groonga::Typeまたはテーブル（Groonga::Array、
      #   Groonga::Hash、Groonga::PatriciaTrieのどれか）を指
      #   定する。
      #
      #   Groonga::Typeを指定した場合は、その型が示す範囲の
      #   値をキーとして使用する。ただし、キーの最大サイズは
      #   4096バイトであるため、Groonga::Type::TEXTや
      #   Groonga::Type::LONG_TEXTは使用できない。
      #
      #   テーブルを指定した場合はレコードIDをキーとして使用
      #   する。指定したテーブルのGroonga::Recordをキーとし
      #   て使用することもでき、その場合は自動的に
      #   Groonga::RecordからレコードIDを取得する。
      #
      #   省略した場合は文字列をキーとして使用する。この場合、
      #   4096バイトまで使用可能である。
      #
      # [+:default_tokenizer+]
      #   Groonga::IndexColumnで使用するトークナイザを指定す
      #   る。デフォルトでは何も設定されていないので、テーブ
      #   ルにGroonga::IndexColumnを定義する場合は
      #   <tt>"TokenBigram"</tt>などを指定する必要がある。
      #
      # 以下は+:type+に+:patricia_trie+を指定した時に指定可能。
      #
      # [+:key_normalize+]
      #   +true+を指定するとキーを正規化する。
      #
      # [+:key_with_sis+]
      #   +true+を指定するとキーの文字列の全suffixが自動的に
      #   登録される。
      def create_table(name, options={}, &block)
        define do |schema|
          schema.create_table(name, options, &block)
        end
      end

      # 名前が_name_のテーブルを削除する。
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:context+]
      #   スキーマ定義時に使用するGroonga::Contextを指定する。
      #   省略した場合はGroonga::Context.defaultを使用する。
      def remove_table(name, options={})
        define do |schema|
          schema.remove_table(name, options)
        end
      end

      # call-seq:
      #   Groonga::Schema.change_table(name, options={}) {|table| ...}
      #
      # 名前が_name_のテーブルを変更する。以下の省略形。
      #
      #   Groonga::Schema.define do |schema|
      #     schema.change_table(name, options) do |table|
      #       ...
      #     end
      #   end
      #
      # ブロックにはGroonga::Schema::TableDefinitionオブジェ
      # クトがわたるので、そのオブジェクトを利用してテーブル
      # の詳細を定義する。
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:context+]
      #   スキーマ定義時に使用するGroonga::Contextを指定する。
      #   省略した場合はGroonga::Context.defaultを使用する。
      def change_table(name, options={}, &block)
        define do |schema|
          schema.change_table(name, options, &block)
        end
      end

      # call-seq:
      #   Groonga::Schema.create_view(name, options={}) {|view| ...}
      #
      # 名前が_name_のビューを作成する。以下の省略形。
      #
      #   Groonga::Schema.define do |schema|
      #     schema.create_view(name, options) do |view|
      #       ...
      #     end
      #   end
      #
      # ブロックにはGroonga::Schema::ViewDefinitionオブジェ
      # クトがわたるので、そのオブジェクトを利用してビュー
      # の詳細を定義する。
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:force+]
      #   +true+を指定すると既存の同名のビューが存在してい
      #   ても、強制的にビューを作成する。
      #
      # [+:context+]
      #   スキーマ定義時に使用するGroonga::Contextを指定する。
      #   省略した場合はGroonga::Context.defaultを使用する。
      #
      # [+:path+]
      #   ビューを保存するパスを指定する。パスを指定すると
      #   永続ビューになる。
      #
      # [+:persistent+]
      #   ビューを永続ビューとする。+:path:+を省略した場
      #   合はパス名は自動的に作成される。デフォルトでは永続
      #   ビューとなる。
      def create_view(name, options={}, &block)
        define do |schema|
          schema.create_view(name, options, &block)
        end
      end

      # 名前が_name_のテーブルを削除する。
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:context+]
      #   スキーマ定義時に使用するGroonga::Contextを指定する。
      #   省略した場合はGroonga::Context.defaultを使用する。
      def remove_view(name, options={})
        define do |schema|
          schema.remove_view(name, options)
        end
      end

      # call-seq:
      #   Groonga::Schema.change_view(name, options={}) {|view| ...}
      #
      # 名前が_name_のビューを変更する。以下の省略形。
      #
      #   Groonga::Schema.define do |schema|
      #     schema.change_view(name, options) do |view|
      #       ...
      #     end
      #   end
      #
      # ブロックにはGroonga::Schema::ViewDefinitionオブジェ
      # クトがわたるので、そのオブジェクトを利用してテーブル
      # の詳細を定義する。
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:context+]
      #   スキーマ定義時に使用するGroonga::Contextを指定する。
      #   省略した場合はGroonga::Context.defaultを使用する。
      def change_view(name, options={}, &block)
        define do |schema|
          schema.change_view(name, options, &block)
        end
      end

      # スキーマの内容を文字列で返す。返された値は
      # Groonga::Schema.restoreすることによりスキーマ内に組
      # み込むことができる。
      #
      #   dump.rb:
      #     File.open("/tmp/groonga-schema.rb", "w") do |schema|
      #       dumped_text = Groonga::Schema.dump
      #     end
      #
      #   restore.rb:
      #     dumped_text = Groonga::Schema.dump
      #     Groonga::Database.create(:path => "/tmp/new-db.grn")
      #     Groonga::Schema.restore(dumped_text)
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:context+]
      #   スキーマ定義時に使用するGroonga::Contextを指定する。
      #   省略した場合はGroonga::Context.defaultを使用する。
      def dump(options={})
        Dumper.new(options).dump
      end

      # Groonga::Schema.dumpで文字列化したスキーマを組み込む。
      def restore(dumped_text, options={})
        define(options) do |schema|
          schema.load(dumped_text)
	end
      end

      def normalize_type(type) # :nodoc:
        return type if type.nil?
        return type if type.is_a?(Groonga::Object)
        case type.to_s
        when "string"
          "ShortText"
        when "text"
          "Text"
        when "int", "integer"
          "Int32"
        when "float"
          "Float"
        when "decimal"
          "Int64"
        when "datetime", "timestamp", "time", "date"
          "Time"
        when "binary"
          "LongText"
        when "boolean"
          "Bool"
        else
          type
        end
      end
    end

    # スキーマ定義を開始する。
    #
    # _options_に指定可能な値は以下の通り。
    #
    # [+:context+]
    #   スキーマ定義時に使用するGroonga::Contextを指定する。
    #   省略した場合はGroonga::Context.defaultを使用する。
    def initialize(options={})
      @options = (options || {}).dup
      @definitions = []
    end

    # 定義されたスキーマ定義を実際に実行する。
    def define
      @definitions.each do |definition|
        definition.define
      end
    end

    # Groonga::Schema.dumpで返されたスキーマの内容を読み込む。
    #
    # 読み込まれた内容は#defineを呼び出すまでは実行されない
    # ことに注意すること。
    def load(dumped_text)
      instance_eval(dumped_text)
    end

    # call-seq:
    #   schema.create_table(name, options={}) {|table| ...}
    #
    # 名前が_name_のテーブルを作成する。
    #
    # テーブルの作成は#defineを呼び出すまでは実行されないこ
    # とに注意すること。
    #
    # _options_に指定可能な値は以下の通り。
    #
    # [+:force+]
    #   +true+を指定すると既存の同名のテーブルが存在してい
    #   ても、強制的にテーブルを作成する。
    #
    # [+:type+]
    #   テーブルの型を指定する。+:array+, +:hash+,
    #   +:patricia_trie+のいずれかを指定する。デフォルトで
    #   は+:array+になる。
    #
    # [+:context+]
    #   スキーマ定義時に使用するGroonga::Contextを指定する。
    #   省略した場合はGroonga::Schema.newで指定した
    #   Groonga::Contextを使用する。Groonga::Schema.newで指
    #   定していない場合はGroonga::Context.defaultを使用する。
    #
    # [+:path+]
    #   テーブルを保存するパスを指定する。パスを指定すると
    #   永続テーブルになる。
    #
    # [+:persistent+]
    #   テーブルを永続テーブルとする。+:path:+を省略した場
    #   合はパス名は自動的に作成される。デフォルトでは永続
    #   テーブルとなる。
    #
    # [+:value_type+]
    #   値の型を指定する。省略すると値のための領域を確保しな
    #   い。値を保存したい場合は必ず指定すること。
    #
    #   参考: Groonga::Type.new
    #
    # [+:sub_records+]
    #   +true+を指定するとGroonga::Table#groupでグループ化
    #   したときに、Groonga::Record#n_sub_recordsでグルー
    #   プに含まれるレコードの件数を取得できる。
    #
    # 以下は+:type+に+:hash+あるいは+:patricia_trie+を指定
    # した時に指定可能。
    #
    # [+:key_type+]
    #   キーの種類を示すオブジェクトを指定する。キーの種類
    #   には型名（"Int32"や"ShortText"など）または
    #   Groonga::Typeまたはテーブル（Groonga::Array、
    #   Groonga::Hash、Groonga::PatriciaTrieのどれか）を指
    #   定する。
    #
    #   Groonga::Typeを指定した場合は、その型が示す範囲の
    #   値をキーとして使用する。ただし、キーの最大サイズは
    #   4096バイトであるため、Groonga::Type::TEXTや
    #   Groonga::Type::LONG_TEXTは使用できない。
    #
    #   テーブルを指定した場合はレコードIDをキーとして使用
    #   する。指定したテーブルのGroonga::Recordをキーとし
    #   て使用することもでき、その場合は自動的に
    #   Groonga::RecordからレコードIDを取得する。
    #
    #   省略した場合は文字列をキーとして使用する。この場合、
    #   4096バイトまで使用可能である。
    #
    # [+:default_tokenizer+]
    #   Groonga::IndexColumnで使用するトークナイザを指定す
    #   る。デフォルトでは何も設定されていないので、テーブ
    #   ルにGroonga::IndexColumnを定義する場合は
    #   <tt>"TokenBigram"</tt>などを指定する必要がある。
    #
    # 以下は+:type+に+:patricia_trie+を指定した時に指定可能。
    #
    # [+:key_normalize+]
    #   +true+を指定するとキーを正規化する。
    #
    # [+:key_with_sis+]
    #   +true+を指定するとキーの文字列の全suffixが自動的に
    #   登録される。
    def create_table(name, options={})
      definition = TableDefinition.new(name, @options.merge(options || {}))
      yield(definition)
      @definitions << definition
    end

    # 名前が_name_のテーブルを削除する。
    #
    # テーブルの削除は#defineを呼び出すまでは実行されないこ
    # とに注意すること。
    #
    # _options_に指定可能な値は以下の通り。
    #
    # [+:context+]
    #   スキーマ定義時に使用するGroonga::Contextを指定する。
    #   省略した場合はGroonga::Context.defaultを使用する。
    def remove_table(name, options={})
      definition = TableRemoveDefinition.new(name, @options.merge(options || {}))
      @definitions << definition
    end

    # call-seq:
    #   schema.change_table(name, options={}) {|table| ...}
    #
    # 名前が_name_のテーブルを変更する。
    #
    # テーブルの変更は#defineを呼び出すまでは実行されないこ
    # とに注意すること。
    #
    # _options_に指定可能な値は以下の通り。
    #
    # [+:context+]
    #   スキーマ定義時に使用するGroonga::Contextを指定する。
    #   省略した場合はGroonga::Context.defaultを使用する。
    def change_table(name, options={})
      options = @options.merge(options || {}).merge(:change => true)
      definition = TableDefinition.new(name, options)
      yield(definition)
      @definitions << definition
    end

    # call-seq:
    #   schema.create_view(name, options={}) {|view| ...}
    #
    # 名前が_name_のビューを作成する。
    #
    # ビューの作成は#defineを呼び出すまでは実行されないこ
    # とに注意すること。
    #
    # _options_に指定可能な値は以下の通り。
    #
    # [+:force+]
    #   +true+を指定すると既存の同名のビューが存在してい
    #   ても、強制的にビューを作成する。
    #
    # [+:context+]
    #   スキーマ定義時に使用するGroonga::Contextを指定する。
    #   省略した場合はGroonga::Schema.newで指定した
    #   Groonga::Contextを使用する。Groonga::Schema.newで指
    #   定していない場合はGroonga::Context.defaultを使用する。
    #
    # [+:path+]
    #   テーブルを保存するパスを指定する。パスを指定すると
    #   永続テーブルになる。
    #
    # [+:persistent+]
    #   テーブルを永続テーブルとする。+:path:+を省略した場
    #   合はパス名は自動的に作成される。デフォルトでは永続
    #   テーブルとなる。
    def create_view(name, options={})
      definition = ViewDefinition.new(name, @options.merge(options || {}))
      yield(definition)
      @definitions << definition
    end

    # 名前が_name_のビューを削除する。
    #
    # ビューの削除は#defineを呼び出すまでは実行されないことに
    # 注意すること。
    #
    # _options_に指定可能な値は以下の通り。
    #
    # [+:context+]
    #   スキーマ定義時に使用するGroonga::Contextを指定する。
    #   省略した場合はGroonga::Context.defaultを使用する。
    def remove_view(name, options={})
      definition = ViewRemoveDefinition.new(name, @options.merge(options || {}))
      @definitions << definition
    end

    # call-seq:
    #   schema.change_view(name, options={}) {|table| ...}
    #
    # 名前が_name_のビューを変更する。
    #
    # ビューの変更は#defineを呼び出すまでは実行されないこ
    # とに注意すること。
    #
    # _options_に指定可能な値は以下の通り。
    #
    # [+:context+]
    #   スキーマ定義時に使用するGroonga::Contextを指定する。
    #   省略した場合はGroonga::Context.defaultを使用する。
    def change_view(name, options={})
      options = @options.merge(options || {}).merge(:change => true)
      definition = ViewDefinition.new(name, options)
      yield(definition)
      @definitions << definition
    end

    # スキーマ定義時にGroonga::Schema.create_tableや
    # Groonga::Schema#create_tableからブロックに渡されてくる
    # オブジェクト
    class TableDefinition
      # テーブルの名前
      attr_reader :name

      def initialize(name, options) # :nodoc:
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @definitions = []
        validate_options(options)
        @options = options
        @table_type = table_type
      end

      def define # :nodoc:
        table = context[@name]
        if @options[:change]
          raise TableNotExists.new(@name) if table.nil?
        else
          if table
            unless same_table?(table, create_options)
              if @options[:force]
                table.remove
                table = nil
              else
                options = create_options
                raise TableCreationWithDifferentOptions.new(table, options)
              end
            end
          end
          table ||= @table_type.create(create_options)
        end
        @definitions.each do |definition|
          definition.define(self, table)
        end
        table
      end

      # 名前が_name_で型が_type_のカラムを作成する。
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:force+]
      #   +true+を指定すると既存の同名のカラムが存在してい
      #   ても、強制的に新しいカラムを作成する。
      #
      # [+:path+]
      #   カラムを保存するパス。
      #
      # [+:persistent+]
      #   +true+を指定すると永続カラムとなる。+:path+を省略
      #   した場合は自動的にパスが付加される。
      #
      # [+:type+]
      #   カラムの値の格納方法について指定する。省略した場合は、
      #   +:scalar+になる。
      #
      #   [+:scalar+]
      #     スカラ値(単独の値)を格納する。
      #
      #   [+:vector+]
      #     値の配列を格納する。
      #
      # [+:compress+]
      #   値の圧縮方法を指定する。省略した場合は、圧縮しない。
      #
      #   [+:zlib+]
      #     値をzlib圧縮して格納する。
      #
      #   [+:lzo+]
      #     値をlzo圧縮して格納する。
      def column(name, type, options={})
        definition = self[name, ColumnDefinition]
        if definition.nil?
          definition = ColumnDefinition.new(name, options)
          update_definition(name, ColumnDefinition, definition)
        end
        definition.type = type
        definition.options.merge!(column_options.merge(options))
        self
      end

      # 名前が_name_のカラムを削除する。
      #
      # _options_に指定可能な値はない(TODO _options_は不要?)。
      #
      def remove_column(name, options={})
        definition = self[name, ColumnRemoveDefinition]
        if definition.nil?
          definition = ColumnRemoveDefinition.new(name, options)
          update_definition(name, ColumnRemoveDefinition, definition)
        end
        definition.options.merge!(options)
        self
      end

      # call-seq:
      #   table.index(target_column_full_name, options={})
      #   table.index(target_table, target_column, options={})
      #
      # _target_table_の_target_column_を対象とするインデッ
      # クスカラムを作成する。
      #
      # _target_column_full_name_で指定するときはテーブル名
      # とカラム名を"."でつなげます。例えば、「Users」テーブ
      # ルの「name」カラムのインデックスカラムを指定する場合
      # はこうなります。
      #
      #   table.index("Users.name")
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:name+]
      #   インデックスカラムのカラム名を任意に指定する。
      #
      # [+:force+]
      #   +true+を指定すると既存の同名のカラムが存在してい
      #   ても、強制的に新しいカラムを作成する。
      #
      # [+:path+]
      #   カラムを保存するパス。
      #
      # [+:persistent+]
      #   +true+を指定すると永続カラムとなる。+:path+を省略
      #   した場合は自動的にパスが付加される。
      #
      # [+:with_section+]
      #   転置索引にsection(段落情報)を合わせて格納する。
      #
      # [+:with_weight+]
      #   転置索引にweight情報を合わせて格納する。
      #
      # [+:with_position+]
      #   転置索引に出現位置情報を合わせて格納する。
      def index(target_table_or_target_column_full_name, *args)
        if args.size > 2
          n_args = args.size + 1
          raise ArgumentError, "wrong number of arguments (#{n_args} for 2 or 3)"
        end
        options = nil
        options = args.pop if args.last.is_a?(::Hash)
        if args.empty?
          target_column_full_name = target_table_or_target_column_full_name
          if target_column_full_name.is_a?(Groonga::Column)
            target_column_full_name = target_column_full_name.name
          end
          target_table, target_column = target_column_full_name.split(/\./, 2)
        else
          target_table = target_table_or_target_column_full_name
          target_column = args.pop
        end
        define_index(target_table, target_column, options || {})
      end

      # 名前が_name_の32bit符号付き整数のカラムを作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def integer32(name, options={})
        column(name, "Int32", options)
      end
      alias_method :integer, :integer32
      alias_method :int32, :integer32

      # 名前が_name_の64bit符号付き整数のカラムを作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def integer64(name, options={})
        column(name, "Int64", options)
      end
      alias_method :int64, :integer64

      # 名前が_name_の32bit符号なし整数のカラムを作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def unsigned_integer32(name, options={})
        column(name, "UInt32", options)
      end
      alias_method :unsigned_integer, :unsigned_integer32
      alias_method :uint32, :unsigned_integer32

      # 名前が_name_の64bit符号なし整数のカラムを作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def unsigned_integer64(name, options={})
        column(name, "UInt64", options)
      end
      alias_method :uint64, :unsigned_integer64

      # 名前が_name_のieee754形式の64bit浮動小数点数のカラム
      # を作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def float(name, options={})
        column(name, "Float", options)
      end

      # 名前が_name_の64bit符号付き整数で1970年1月1日0時0分
      # 0秒からの経過マイクロ秒数を格納するカラムを作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def time(name, options={})
        column(name, "Time", options)
      end

      # 名前が_name_の4Kbyte以下の文字列を格納できるカラムを
      # 作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def short_text(name, options={})
        column(name, "ShortText", options)
      end
      alias_method :string, :short_text

      # 名前が_name_の64Kbyte以下の文字列を格納できるカラムを
      # 作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def text(name, options={})
        column(name, "Text", options)
      end

      # 名前が_name_の2Gbyte以下の文字列を格納できるカラムを
      # 作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def long_text(name, options={})
        column(name, "LongText", options)
      end

      # 名前が_name_で_table_のレコードIDを格納する参照カラ
      # ムを作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def reference(name, table, options={})
        column(name, table, options)
      end

      # 名前が_name_の真偽値を格納できるカラムを作成する。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def boolean(name, options={})
        column(name, "Bool", options)
      end
      alias_method :bool, :boolean

      def [](name, definition_class=nil) # :nodoc:
        @definitions.find do |definition|
          definition.name.to_s == name.to_s and
            (definition_class.nil? or definition.is_a?(definition_class))
        end
      end

      def context # :nodoc:
        @options[:context] || Groonga::Context.default
      end

      private
      def update_definition(name, definition_class, definition) # :nodoc:
        old_definition = self[name, definition_class]
        if old_definition
          index = @definitions.index(old_definition)
          @definitions[index] = definition
        else
          @definitions << definition
        end
      end

      AVAILABLE_OPTION_KEYS = [:context, :change, :force,
                               :type, :path, :persistent,
                               :key_type, :value_type, :sub_records,
                               :default_tokenizer,
                               :key_normalize, :key_with_sis] # :nodoc:
      def validate_options(options) # :nodoc:
        return if options.nil?
        unknown_keys = options.keys - AVAILABLE_OPTION_KEYS
        unless unknown_keys.empty?
          raise UnknownOptions.new(options, unknown_keys, AVAILABLE_OPTION_KEYS)
        end
      end

      def table_type # :nodoc:
        type = @options[:type]
        case type
        when :array, nil
          Groonga::Array
        when :hash
          Groonga::Hash
        when :patricia_trie
          Groonga::PatriciaTrie
        else
          raise UnknownTableType.new(type, [nil, :array, :hash, :patricia_trie])
        end
      end

      def create_options # :nodoc:
        common = {
          :name => @name,
          :path => @options[:path],
          :persistent => persistent?,
          :value_type => @options[:value_type],
          :context => context,
          :sub_records => @options[:sub_records],
        }
        key_support_table_common = {
          :key_type => Schema.normalize_type(@options[:key_type]),
          :default_tokenizer => @options[:default_tokenizer],
        }

        if @table_type == Groonga::Array
          common
        elsif @table_type == Groonga::Hash
          common.merge(key_support_table_common)
        elsif @table_type == Groonga::PatriciaTrie
          options = {
            :key_normalize => @options[:key_normalize],
            :key_with_sis => @options[:key_with_sis],
          }
          common.merge(key_support_table_common).merge(options)
        end
      end

      def column_options # :nodoc:
        {:persistent => persistent?}
      end

      def persistent? # :nodoc:
        @options[:persistent].nil? ? true : @options[:persistent]
      end

      def define_index(target_table, target_column, options)
        name = options.delete(:name)
        name ||= "#{target_table}_#{target_column}".gsub(/\./, "_")

        definition = self[name, IndexColumnDefinition]
        if definition.nil?
          definition = IndexColumnDefinition.new(name, options)
          update_definition(name, IndexColumnDefinition, definition)
        end
        definition.target_table = target_table
        definition.target_column = target_column
        definition.options.merge!(column_options.merge(options))
        self
      end

      def same_table?(table, options)
        return false unless table.class == @table_type
        return false unless table.range == resolve_type(options[:value_type])
        return true
        return false unless table.sub_records == options[:sub_records]

        case table
        when Groonga::Array
          true
        when Groonga::Hash, Groonga::PatriciaTrie
          return false unless table.domain == options[:key_type]
          return false unless table.default_tokenizer == options[:default_tokenizer]
          if table.is_a?(Groonga::PatriciaTrie)
            return false unless table.key_normalize == options[:key_normalize]
            return false unless table.key_with_sis == options[:key_with_sis]
          end
          true
        else
          false
        end
      end

      def resolve_type(type)
        if type.nil?
          nil
        elsif type.is_a?(String)
          context[type]
        else
          type
        end
      end
    end

    class TableRemoveDefinition # :nodoc:
      def initialize(name, options={})
        @name = name
        @options = options
      end

      def define
        context = @options[:context] || Groonga::Context.default
        context[@name].remove
      end
    end

    # スキーマ定義時にGroonga::Schema.create_viewや
    # Groonga::Schema#create_viewからブロックに渡されてくる
    # オブジェクト
    class ViewDefinition
      # ビューの名前
      attr_reader :name

      def initialize(name, options) # :nodoc:
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @tables = []
        validate_options(options)
        @options = options
      end

      def define # :nodoc:
        view = context[@name]
        if @options[:change]
          raise TableNotExists.new(@name) if view.nil?
        else
          if view and @options[:force]
            view.remove
            view = nil
          end
          view ||= Groonga::View.create(create_options)
        end
        _context = context
        @tables.each do |table|
          unless table.is_a?(Groonga::Table)
            table_name = table
            table = context[table_name]
            raise TableNotExists.new(table_name) if table.nil?
          end
          view.add_table(table)
        end
        view
      end

      # 名前が_table_のテーブルをビューに追加する。
      def add(table)
        table = table.to_s if table.is_a?(Symbol)
        @tables << table
        self
      end

      def context # :nodoc:
        @options[:context] || Groonga::Context.default
      end

      private
      AVAILABLE_OPTION_KEYS = [:context, :change, :force,
                               :path, :persistent] # :nodoc:
      def validate_options(options) # :nodoc:
        return if options.nil?
        unknown_keys = options.keys - AVAILABLE_OPTION_KEYS
        unless unknown_keys.empty?
          raise UnknownOptions.new(options, unknown_keys, AVAILABLE_OPTION_KEYS)
        end
      end

      def create_options # :nodoc:
        {
          :name => @name,
          :path => @options[:path],
          :persistent => persistent?,
          :context => context,
        }
      end

      def persistent? # :nodoc:
        @options[:persistent].nil? ? true : @options[:persistent]
      end
    end

    class ViewRemoveDefinition # :nodoc:
      def initialize(name, options={})
        @name = name
        @options = options
      end

      def define
        context = @options[:context] || Groonga::Context.default
        context[@name].remove
      end
    end

    class ColumnDefinition # :nodoc:
      attr_accessor :name, :type
      attr_reader :options

      def initialize(name, options={})
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @options = (options || {}).dup
        @type = nil
      end

      def define(table_definition, table)
        column = table.column(@name)
        if column
          return column if same_column?(table_definition, column)
          if @options.delete(:force)
            column.remove
          else
            options = @options.merge(:type => @type)
            raise ColumnCreationWithDifferentOptions.new(column, options)
          end
        end
        table.define_column(@name,
                            Schema.normalize_type(@type),
                            @options)
      end

      private
      def same_column?(table_definition, column)
        context = table_definition.context
        # TODO: should check column type and other options.
        column.range == context[Schema.normalize_type(@type)]
      end
    end

    class ColumnRemoveDefinition # :nodoc:
      attr_accessor :name
      attr_reader :options

      def initialize(name, options={})
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @options = (options || {}).dup
      end

      def define(table_definition, table)
        table.column(@name).remove
      end
    end

    class IndexColumnDefinition # :nodoc:
      attr_accessor :name, :target_table, :target_column
      attr_reader :options

      def initialize(name, options={})
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @options = (options || {}).dup
        @target_table = nil
        @target_column = nil
      end

      def define(table_definition, table)
        target_name = "#{@target_table}.#{@target_column}"
        target_table = table_definition.context[@target_table]
        if target_table.nil? or
            !(@target_column == "_key" or
              target_table.have_column?(@target_column))
          raise UnknownIndexTarget.new(target_name)
        end
        index = table.column(@name)
        if index
          return index if same_index?(table_definition, index)
          if @options.delete(:force)
            index.remove
          else
            options = @options.merge(:type => :index,
                                     :target_name => target_name)
            raise ColumnCreationWithDifferentOptions.new(index, options)
          end
        end
        index = table.define_index_column(@name,
                                          @target_table,
                                          @options)
        index.source = target_table.column(@target_column)
        index
      end

      private
      def same_index?(table_definition, index)
        context = table_definition.context
        # TODO: should check column type and other options.
        return false if index.range.name != @target_table
        source_names = index.sources.collect do |source|
          if source.nil?
            "#{index.range.name}._key"
          else
            source.name
          end
        end
        source_names == ["#{@target_table}.#{@target_column}"]
      end
    end

    class Dumper # :nodoc:
      def initialize(options={})
        @options = (options || {}).dup
      end

      def dump
        context = @options[:context] || Groonga::Context.default
        database = context.database
        return nil if database.nil?

        reference_columns = []
        definitions = []
        database.each do |object|
          next unless object.is_a?(Groonga::Table)
          schema = "create_table(#{object.name.inspect}) do |table|\n"
          object.columns.sort_by {|column| column.local_name}.each do |column|
            if column.range.is_a?(Groonga::Table)
              reference_columns << column
            else
              type = column_method(column)
              name = column.local_name
              schema << "  table.#{type}(#{name.inspect})\n"
            end
          end
          schema << "end"
          definitions << schema
        end

        reference_columns.group_by do |column|
          column.table
        end.each do |table, columns|
          schema = "change_table(#{table.name.inspect}) do |table|\n"
          columns.each do |column|
            name = column.local_name
            reference = column.range
            schema << "  table.reference(#{name.inspect}, " +
                                        "#{reference.name.inspect})\n"
          end
          schema << "end"
          definitions << schema
        end

        if definitions.empty?
          ""
        else
          definitions.join("\n\n") + "\n"
        end
      end

      private
      def column_method(column)
        range = column.range
        case range.name
        when "Int32"
          "integer32"
        when "Int64"
          "integer64"
        when "UInt32"
          "unsigned_integer32"
        when "UInt64"
          "unsigned_integer64"
        when "Float"
          "float"
        when "Time"
          "time"
        when "ShortText"
          "short_text"
        when "Text"
          "text"
        when "LongText"
          "long_text"
        else
          raise ArgumentError, "unsupported column: #{column.inspect}"
        end
      end
    end
  end
end
