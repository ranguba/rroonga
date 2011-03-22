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

require 'fileutils'

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

    # 未知のインデックス対象テーブルを指定したときに発生する。
    class UnknownIndexTargetTable < Error
      attr_reader :table
      def initialize(table)
        @table = table
        super("unknown index target table: <#{@table.inspect}>")
      end
    end

    # 未知のインデックス対象を指定したときに発生する。
    class UnknownIndexTarget < Error
      attr_reader :table, :targets
      def initialize(table, targets)
        @table = table
        @targets = targets
        super("unknown index target: <#{@table.inspect}>: <#{@targets.inspect}>")
      end
    end

    # 未知のオプションを指定したときに発生する。
    class UnknownOptions < Error
      attr_reader :options, :unknown_keys, :available_keys
      def initialize(options, unknown_keys, available_keys)
        @options = options
        @unknown_keys = unknown_keys
        @available_keys = available_keys
        message = "unknown keys are specified: #{@unknown_keys.inspect}"
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

    # 参照先のテーブルを推測できないときに発生する。
    class UnguessableReferenceTable < Error
      attr_reader :name, :tried_table_names
      def initialize(name, tried_table_names)
        @name = name
        @tried_table_names = tried_table_names
        super("failed to guess referenced table name " +
              "for reference column: #{@name.inspect}: " +
              "tried table names: #{@tried_table_names.inspect}")
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
      # 以下は+:type+に+:hash+または+:patricia_trie+を指定し
      # た時に指定可能。
      #
      # [+:key_normalize+]
      #   +true+を指定するとキーを正規化する。
      #
      # 以下は+:type+に+:patricia_trie+を指定した時に指定可能。
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

      # call-seq:
      #   Groonga::Schema.remove_column(table_name, column_name)
      #
      # 以下と同様:
      #
      #   Groonga::Schema.change_table(table_name) do |table|
      #     table.remove_column(column_name)
      #   end
      def remove_column(table_name, column_name)
        change_table(table_name) do |table|
          table.remove_column(column_name)
        end
      end

      # スキーマの内容を文字列をRubyスクリプト形式またはgrn式
      # 形式で返す。デフォルトはRubyスクリプト形式である。
      # Rubyスクリプト形式で返された値は
      # Groonga::Schema.restoreすることによりスキーマ内に組み
      # 込むことができる。
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
      # grn式形式で返された値はgroongaコマンドで読み込むこと
      # ができる。
      #
      #   dump.rb:
      #     File.open("/tmp/groonga-schema.grn", "w") do |schema|
      #       dumped_text = Groonga::Schema.dump(:syntax => :command)
      #     end
      #
      #   % groonga db/path < /tmp/groonga-schema.grn
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:context+]
      #   スキーマ定義時に使用するGroonga::Contextを指定する。
      #   省略した場合はGroonga::Context.defaultを使用する。
      #
      # [+:syntax+]
      #   スキーマの文字列の形式を指定する。指定可能な値は以
      #   下の通り。
      #
      #   [+:ruby+]
      #     Rubyスクリプト形式。省略した場合、+nil+の場合も
      #     Rubyスクリプト形式になる。
      #
      #   [+:command+]
      #     grn式形式。groongaコマンドで読み込むことができる。
      def dump(options={})
        schema = new(:context => options[:context],
                     :syntax => options[:syntax])
        schema.dump
      end

      # Groonga::Schema.dumpで文字列化したスキーマを組み込む。
      def restore(dumped_text, options={})
        define(options) do |schema|
          schema.restore(dumped_text)
	end
      end

      NORMALIZE_TYPE_TABLE = {
        "short_text" => "ShortText",
        "string" => "ShortText",
        "text" => "Text",
        "binary" => "LongText",
        "long_text" => "LongText",
        "int" => "Int32",
        "integer" => "Int32",
        "int32" => "Int32",
        "integer32" => "Int32",
        "decimal" => "Int64",
        "int64" => "Int64",
        "integer64" => "Int64",
        "uint" => "UInt32",
        "unsigned_integer" => "UInt32",
        "uint32" => "UInt32",
        "unsigned_integer32" => "UInt32",
        "uint64" => "UInt64",
        "unsigned_integer64" => "UInt64",
        "float" => "Float",
        "datetime" => "Time",
        "timestamp" => "Time",
        "time" => "Time",
        "date" => "Time",
        "boolean" => "Bool",
        "unigram" => "TokenUnigram",
        "token_unigram" => "TokenUnigram",
        "bigram" => "TokenBigram",
        "token_bigram" => "TokenBigram",
        "bigram_split_symbol" => "TokenBigramSplitSymbol",
        "token_bigram_split_symbol" => "TokenBigramSplitSymbol",
        "bigram_split_symbol_alpha" => "TokenBigramSplitSymbolAlpha",
        "token_bigram_split_symbol_alpha" => "TokenBigramSplitSymbolAlpha",
        "bigram_split_symbol_alpha_digit" => "TokenBigramSplitSymbolAlphaDigit",
        "token_bigram_split_symbol_alpha_digit" =>
          "TokenBigramSplitSymbolAlphaDigit",
        "bigram_ignore_blank" => "TokenBigramIgnoreBlank",
        "token_bigram_ignore_blank" => "TokenBigramIgnoreBlank",
        "bigram_ignore_blank_split_symbol" =>
          "TokenBigramIgnoreBlankSplitSymbol",
        "token_bigram_ignore_blank_split_symbol" =>
          "TokenBigramIgnoreBlankSplitSymbol",
        "bigram_ignore_blank_split_symbol_alpha" =>
          "TokenBigramIgnoreBlankSplitSymbolAlpha",
        "token_bigram_ignore_blank_split_symbol_alpha" =>
          "TokenBigramIgnoreBlankSplitSymbolAlpha",
        "bigram_ignore_blank_split_symbol_alpha_digit" =>
          "TokenBigramIgnoreBlankSplitSymbolAlphaDigit",
        "token_bigram_ignore_blank_split_symbol_alpha_digit" =>
          "TokenBigramIgnoreBlankSplitSymbolAlphaDigit",
        "trigram" => "TokenTrigram",
        "token_trigram" => "TokenTrigram",
        "mecab" => "TokenMecab",
        "token_mecab"=> "TokenMecab",
      }
      def normalize_type(type) # :nodoc:
        return type if type.nil?
        return type if type.is_a?(Groonga::Object)
        type = type.to_s if type.is_a?(Symbol)
        NORMALIZE_TYPE_TABLE[type] || type
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
      @options[:context] ||= Groonga::Context.default
      @definitions = []
    end

    # 定義されたスキーマ定義を実際に実行する。
    def define
      @definitions.each do |definition|
        definition.define
      end
    end

    # Groonga::Schema#dumpで返されたスキーマの内容を読み込む。
    #
    # 読み込まれた内容は#defineを呼び出すまでは実行されない
    # ことに注意すること。
    def restore(dumped_text)
      instance_eval(dumped_text)
    end
    # for backward compatibility.
    # TODO: remove this at the next major release.
    alias_method :load, :restore

    # スキーマの内容を文字列で返す。返された値は
    # Groonga::Schema#restoreすることによりスキーマ内に組み込むことができる。
    def dump
      dumper = SchemaDumper.new(:context => @options[:context],
                                :syntax => @options[:syntax] || :ruby)
      dumper.dump
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
      yield(definition) if block_given?
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

    # call-seq:
    #   schema.remove_column(table_name, column_name)
    #
    # 以下と同様:
    #
    #   schema.change_table(table_name) do |table|
    #     table.remove_column(column_name)
    #   end
    def remove_column(table_name, column_name)
      change_table(table_name) do |table|
        table.remove_column(column_name)
      end
    end

    def context # :nodoc:
      @options[:context] || Groonga::Context.default
    end

    module Path # :nodoc:
      def tables_directory_path(database)
        "#{database.path}.tables"
      end

      def columns_directory_path(table)
        "#{table.path}.columns"
      end

      def rmdir_if_dir_exists(dir)
        begin
          Dir.rmdir(dir)
        rescue Errno::ENOENT
        end
      end
    end

    # スキーマ定義時にGroonga::Schema.create_tableや
    # Groonga::Schema#create_tableからブロックに渡されてくる
    # オブジェクト
    class TableDefinition
      include Path

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

      # 名前が_name_のカラムを削除します。
      #
      # _options_に指定可能な値はありません(TODO _options_は不要?)。
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
      #   table.index(target_table, target_column1, target_column2, ..., options={})
      #
      # _target_table_の_target_column_を対象とするインデック
      # スカラムを作成します。複数のカラムを指定することもで
      # きます。
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
      #   +true+を指定すると転置索引にsection(段落情報)を合
      #   わせて格納する。未指定または+nil+を指定した場合、
      #   複数のカラムを指定すると自動的に有効になる。
      #
      # [+:with_weight+]
      #   +true+を指定すると転置索引にweight情報を合わせて格
      #   納する。
      #
      # [+:with_position+]
      #   +true+を指定すると転置索引に出現位置情報を合わせて
      #   格納する。未指定または+nil+を指定した場合、テーブ
      #   ルがN-gram系のトークナイザーを利用している場合は自
      #   動的に有効になる。
      def index(target_table_or_target_column_full_name, *args)
        key, target_table, target_columns, options =
          parse_index_argument(target_table_or_target_column_full_name, *args)

        name = options.delete(:name)
        definition = self[key, IndexColumnDefinition]
        if definition.nil?
          definition = IndexColumnDefinition.new(name, options)
          update_definition(key, IndexColumnDefinition, definition)
        end
        definition.target_table = target_table
        definition.target_columns = target_columns
        definition.options.merge!(column_options.merge(options))
        self
      end

      # call-seq:
      #   table.remove_index(target_column_full_name, options={})
      #   table.remove_index(target_table, target_column, options={})
      #   table.remove_index(target_table, target_column1, target_column2, ..., options={})
      #
      # _target_table_の_target_column_を対象とするインデッ
      # クスカラムを削除します。
      #
      # _target_column_full_name_で指定するときはテーブル名
      # とカラム名を"."でつなげます。例えば、「Users」テーブ
      # ルの「name」カラムのインデックスカラムを削除する場合
      # はこうなります。
      #
      #   table.remove_index("Users.name")
      #
      # _options_に指定可能な値は以下の通り。
      #
      # [+:name+]
      #   インデックスカラムのカラム名を任意に指定する。
      def remove_index(target_table_or_target_column_full_name, *args)
        key, target_table, target_columns, options =
          parse_index_argument(target_table_or_target_column_full_name, *args)

        name = options.delete(:name)
        name ||= lambda do |context|
          IndexColumnDefinition.column_name(context,
                                            target_table,
                                            target_columns)
        end
        definition = self[key, ColumnRemoveDefinition]
        if definition.nil?
          definition = ColumnRemoveDefinition.new(name, options)
          update_definition(key, ColumnRemoveDefinition, definition)
        end
        definition.options.merge!(options)
        self
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

      # 以下と同様:
      #   table.time("updated_at")
      #   table.time("created_at")
      def timestamps(options={})
        time("created_at", options)
        time("updated_at", options)
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
      # _table_が省略された場合は_name_の複数形が使われる。
      # 例えば、_name_が"user"な場合は_table_は"users"になる。
      #
      # _options_に指定可能な値は
      # Groonga::Schema::TableDefinition#columnを参照。
      def reference(name, table=nil, options={})
        table ||= lambda {|context| guess_table_name(context, name)}
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
      def update_definition(key, definition_class, definition) # :nodoc:
        old_definition = self[key, definition_class]
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
          :path => path,
          :persistent => persistent?,
          :value_type => @options[:value_type],
          :context => context,
          :sub_records => @options[:sub_records],
        }
        key_support_table_common = {
          :key_type => normalize_key_type(@options[:key_type] || "ShortText"),
          :key_normalize => @options[:key_normalize],
          :default_tokenizer => @options[:default_tokenizer],
        }

        if @table_type == Groonga::Array
          common
        elsif @table_type == Groonga::Hash
          common.merge(key_support_table_common)
        elsif @table_type == Groonga::PatriciaTrie
          options = {
            :key_with_sis => @options[:key_with_sis],
          }
          common.merge(key_support_table_common).merge(options)
        end
      end

      def path
        user_path = @options[:path]
        return user_path if user_path
        tables_dir = tables_directory_path(context.database)
        FileUtils.mkdir_p(tables_dir)
        File.join(tables_dir, @name)
      end

      def column_options # :nodoc:
        {:persistent => persistent?}
      end

      def persistent? # :nodoc:
        @options[:persistent].nil? ? true : @options[:persistent]
      end

      def parse_index_argument(target_table_or_target_column_full_name, *args)
        options = nil
        options = args.pop if args.last.is_a?(::Hash)
        if args.empty?
          target_column_full_name = target_table_or_target_column_full_name
          if target_column_full_name.is_a?(Groonga::Column)
            target_column_full_name = target_column_full_name.name
          end
          target_table, target_column = target_column_full_name.split(/\./, 2)
          target_columns = [target_column]
          key = [target_table, target_columns]
        else
          target_table_name = target_table_or_target_column_full_name
          target_table = lambda do |context|
            guess_table_name(context, target_table_name)
          end
          target_columns = args
          key = [target_table_name, target_columns]
        end
        [key, target_table, target_columns, options || {}]
      end

      def same_table?(table, options)
        return false unless table.class == @table_type
        return false unless table.range == resolve_name(options[:value_type])
        sub_records = options[:sub_records]
        sub_records = false if sub_records.nil?
        return false unless table.support_sub_records? == sub_records
        path = options[:path]
        return false if path and table.path != path

        case table
        when Groonga::Array
          true
        when Groonga::Hash, Groonga::PatriciaTrie
          key_type = normalize_key_type(options[:key_type])
          return false unless table.domain == resolve_name(key_type)
          default_tokenizer = resolve_name(options[:default_tokenizer])
          return false unless table.default_tokenizer == default_tokenizer
          key_normalize = options[:key_normalize]
          key_normalize = false if key_normalize.nil?
          return false unless table.normalize_key? == key_normalize
          if table.is_a?(Groonga::PatriciaTrie)
            key_with_sis = options[:key_with_sis]
            key_with_sis = false if key_with_sis.nil?
            return false unless table.register_key_with_sis? == key_with_sis
          end
          true
        else
          false
        end
      end

      def normalize_key_type(key_type)
        Schema.normalize_type(key_type || "ShortText")
      end

      def resolve_name(type)
        if type.is_a?(String)
          context[type]
        else
          type
        end
      end

      def guess_table_name(context, name)
        original_name = name
        name = name.to_s
        candidate_names = [name]
        if name.respond_to?(:pluralize)
          pluralized_name = name.pluralize
        else
          pluralized_name = "#{name}s"
        end
        candidate_names << pluralized_name
        if pluralized_name.respond_to?(:camelize)
          candidate_names << pluralized_name.camelize
        else
          candidate_names << pluralized_name.split(/_/).collect do |word|
            word.capitalize
          end.join
        end
        candidate_names.each do |table_name|
          return table_name if context[table_name]
        end
        raise UnguessableReferenceTable.new(original_name, candidate_names)
      end
    end

    class TableRemoveDefinition # :nodoc:
      include Path

      def initialize(name, options={})
        @name = name
        @options = options
      end

      def define
        table = removed_table
        dir = columns_directory_path(table)
        result = table.remove
        rmdir_if_dir_exists(dir)
        result
      end

      private
      def removed_table
        context = @options[:context]
        table = context[@name]
        raise TableNotExists.new(@name) if table.nil?
        table
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
      include Path

      attr_accessor :name, :type
      attr_reader :options

      def initialize(name, options={})
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @options = (options || {}).dup
        @type = nil
      end

      def define(table_definition, table)
        context = table_definition.context
        column = table.column(@name)
        options = define_options(context, table)
        if column
          return column if same_column?(context, column)
          if @options[:force]
            column.remove
          else
            raise ColumnCreationWithDifferentOptions.new(column, options)
          end
        end
        table.define_column(@name,
                            normalize_type(context),
                            options)
      end

      private
      def normalize_type(context)
        if @type.respond_to?(:call)
          resolved_type = @type.call(context)
        else
          resolved_type = @type
        end
        Schema.normalize_type(resolved_type)
      end

      def same_column?(context, column)
        # TODO: should check column type and other options.
        column.range == context[normalize_type(context)]
      end

      def define_options(context, table)
        {
          :path => path(context, table),
          :type => @options[:type],
          :compress => @options[:compress],
        }
      end

      def path(context, table)
        user_path = @options[:path]
        return user_path if user_path
        columns_dir = columns_directory_path(table)
        FileUtils.mkdir_p(columns_dir)
        File.join(columns_dir, @name)
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
        if @name.respond_to?(:call)
          name = @name.call(table_definition.context)
        else
          name = @name
        end
        table.column(name).remove
      end
    end

    class IndexColumnDefinition # :nodoc:
      include Path

      class << self
        def column_name(context, target_table, target_columns)
          target_table = resolve(context, target_table)
          "#{target_table.name}_#{target_columns.join('_')}"
        end

        def resolve(context, object)
          return object if object.is_a?(Groonga::Object)
          if object.respond_to?(:call)
            object = object.call(context)
          end
          return nil if object.nil?
          context[object]
        end
      end

      attr_accessor :name, :target_table, :target_columns
      attr_reader :options

      def initialize(name, options={})
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @options = (options || {}).dup
        @target_table = nil
        @target_columns = nil
      end

      def define(table_definition, table)
        context = table_definition.context
        target_table = resolve_target_table(context)
        if target_table.nil?
          raise UnknownIndexTargetTable.new(@target_table)
        end
        nonexistent_columns = nonexistent_columns(target_table)
        unless nonexistent_columns.empty?
          raise UnknownIndexTarget.new(target_table, nonexistent_columns)
        end
        name = @name || self.class.column_name(context,
                                               target_table,
                                               @target_columns)
        index = table.column(name)
        if index
          return index if same_index?(context, index, target_table)
          if @options[:force]
            index.remove
          else
            options = @options.merge(:type => :index,
                                     :target_table => target_table,
                                     :target_columns => @target_columns)
            raise ColumnCreationWithDifferentOptions.new(index, options)
          end
        end
        index = table.define_index_column(name,
                                          target_table,
                                          define_options(context, table, name))
        index.sources = @target_columns.collect do |column|
          target_table.column(column)
        end
        index
      end

      private
      def same_index?(context, index, target_table)
        # TODO: should check column type and other options.
        range = index.range
        return false if range != target_table
        source_names = index.sources.collect do |source|
          if source == range
            "_key"
          else
            source.local_name
          end
        end
        source_names.sort == @target_columns.sort
      end

      def nonexistent_columns(target_table)
        @target_columns.reject do |column|
          column == "_key" or target_table.have_column?(column)
        end
      end

      def resolve_target_table(context)
        self.class.resolve(context, @target_table)
      end

      def define_options(context, table, name)
        {
          :path => path(context, table, name),
          :with_section => @options[:with_section],
          :with_weight => @options[:with_weight],
          :with_position => @options[:with_position],
        }
      end

      def path(context, table, name)
        user_path = @options[:path]
        return user_path if user_path
        columns_dir = "#{table.path}.columns"
        FileUtils.mkdir_p(columns_dir)
        File.join(columns_dir, name)
      end
    end
  end
end
