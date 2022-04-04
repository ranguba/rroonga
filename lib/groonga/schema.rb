# Copyright (C) 2009-2022  Sutou Kouhei <kou@clear-code.com>
# Copyright (C) 2014-2015  Masafumi Yokoyama <yokoyama@clear-code.com>
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

require 'fileutils'

module Groonga

  # groongaのスキーマ（データ構造）を管理するクラス。
  #
  # {Groonga::Schema} を使うことにより簡単にテーブルやカラムを
  # 追加・削除することができる。
  #
  # ![Sample schema](https://raw.githubusercontent.com/ranguba/rroonga/master/doc/images/sample-schema.png)
  #
  # @example 上図のようなスキーマを定義する場合は以下のようになる。
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

    # カラムが存在しないときに発生する。
    class ColumnNotExists < Error
      attr_reader :name
      def initialize(name)
        @name = name
        super("column doesn't exist: <#{@name}>")
      end
    end

    # すでに存在するテーブルと違うオプションでテーブルを作ろ
    # うとしたときに発生する。
    class TableCreationWithDifferentOptions < Error
      attr_reader :table, :options
      def initialize(table, options)
        @table = table
        @options = options
        super("creating table with different options: " +
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
        super("creating column with different option: " +
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

      # スキーマを定義する。ブロックには {Groonga::Schema} オブ
      # ジェクトがわたるので、そのオブジェクトを利用してスキー
      # マを定義する。以下の省略形。
      #
      # <pre>
      # !!!ruby
      # schema = Groonga::Scheme.new(options)
      # # ...
      # schema.define
      # </pre>
      #
      # @param [::Hash] options The name and value
      #   pairs. Omitted names are initialized as the default value.
      # @option options [Groonga::Context] :content (Groonga::Context.default) The context
      #
      #   スキーマ定義時に使用する {Groonga::Context} を指定する。
      def define(options={})
        schema = new(options)
        yield(schema)
        schema.define
      end

      # 名前が _name_ のテーブルを作成する。以下の省略形。
      #
      # <pre>
      # !!!ruby
      # Groonga::Schema.define do |schema|
      #   schema.create_table(name, options) do |table|
      #     # ...
      #   end
      # end
      # </pre>
      #
      # ブロックには {Groonga::Schema::TableDefinition} オブジェ
      # クトがわたるので、そのオブジェクトを利用してテーブル
      # の詳細を定義する。
      #
      # @!macro [new] schema.create_table.common.options
      #   @param [::Hash] options The name and value
      #     pairs. Omitted names are initialized as the default value.
      #
      #   @option options [:array, :hash, :patricia_trie, :double_array_trie] :type The table type
      #
      #     テーブルの型を指定する。
      #     `:array`, `:hash`, `:patricia_trie`,
      #     `:double_array_trie` のいずれかを指定する。
      #     (:key_typeの項も参照)
      #
      #   @option options [Boolean] :force (false)
      #
      #     `true`を指定すると既存の同名のテーブルが
      #     存在していても、強制的にテーブルを作成する。
      #
      #   @option options [Groonga::Context] :context (Groonga::Context.default) The context
      #
      #     スキーマ定義時に使用する {Groonga::Context} を指定する。
      #
      #   @option options :path The path
      #
      #     テーブルを保存するパスを指定する。
      #     パスを指定すると永続テーブルになる。
      #
      #   @option options :persistent (true) The persistent
      #
      #     テーブルを永続テーブルとする。`:path`を省略した場合は
      #     パス名は自動的に作成される。デフォルトでは永続テーブルとなる。
      #
      #   @option options :value_type (nil) The value_type
      #
      #     値の型を指定する。省略すると値のための領域を確保しない。
      #     値を保存したい場合は必ず指定すること。
      #
      #   @option options :sub_records The sub_records
      #
      #     `true`を指定すると{Groonga::Table#group}で
      #     グループ化したときに、{Groonga::Record#n_sub_records}でグループに
      #     含まれるレコードの件数を取得できる。
      #
      # @!macro [new] schema.create_table.key_support.options
      #   @option options :key_type The key_type
      #
      #     キーの種類を示すオブジェクトを指定する。キーの種類には型名
      #     （`"Int32"`や`"ShortText"`など）または {Groonga::Type}また
      #     はテーブル（ {Groonga::Array} 、 {Groonga::Hash} 、
      #     {Groonga::PatriciaTrie} 、 {Groonga::DoubleArrayTrie} のど
      #     れか）を指定する。 {Groonga::Type} を指定した場合は、その
      #     型が示す範囲の値をキーとして使用する。ただし、キーの最大サ
      #     イズは4096バイトであるため、 {Groonga::Type::TEXT} や
      #     {Groonga::Type::LONG_TEXT} は使用できない。テーブルを指定
      #     した場合はレコードIDをキーとして使用する。指定したテーブル
      #     の {Groonga::Record} をキーとして使用することもでき、その
      #     場合は自動的に {Groonga::Record} からレコードIDを取得する。
      #     省略した場合は文字列をキーとして使用する。この場合、4096バ
      #     イトまで使用可能である。
      #
      #   @option options :default_tokenizer The default_tokenizer
      #
      #     {Groonga::IndexColumn} で
      #     使用するトークナイザを指定する。デフォルトでは
      #     何も設定されていないので、テーブルに
      #     {Groonga::IndexColumn} を定義する場合は `"TokenBigram"`
      #     などを指定する必要がある。
      #
      #   @option options [::Array<String, Groonga::Procedure>, nil]
      #     :token_filters (nil) The token filters to be used
      #     in the table.
      #
      #     Here is an example to set two token filters.
      #
      #     ```ruby
      #     Groonga::Schema.define do |schema|
      #       schema.create_table("Terms",
      #                           # ...
      #                           :token_filters => [
      #                             "TokenFilterStem",
      #                             "TokenFilterStopWord",
      #                           ]) do |table|
      #         # ...
      #       end
      #     end
      #     ```
      #
      #   @option options :key_normalize (false) Keys are normalized
      #     if this value is `true`.
      #
      #     @deprecated Use `:normalizer => "NormalizerAuto"` instead.
      #
      #   @option options [String, Groonga::Procedure, nil] :normalizer
      #     The normalizer that is used by {Groonga::IndexColumn}. You
      #     can specify this by normalizer name as String such as
      #     `"NormalizerAuto"` or normalizer object.
      #
      # _options_ に指定可能な値は以下の通り。
      #
      # @overload create_table(name, options= {:type => :array}, &block)
      #
      #   @!macro [new] schema.create_table.array.description
      #     Create a table that manages records by ID when you specify
      #     `:array` to `:type`.
      #
      #     You can identify a record only by record ID. You can't use
      #     key because key doesn't exist in the table.
      #
      #     See description of `TABLE_NO_KEY` at [Groonga documentation
      #     of tables](http://groonga.org/docs/reference/tables.html)
      #     for details.
      #
      #   @!macro [new] schema.create_table.array.options
      #     @!macro schema.create_table.common.options
      #
      #   @!macro schema.create_table.array.description
      #   @!macro schema.create_table.array.options
      #
      # @overload create_table(name, options= {:type => :hash}, &block)
      #
      #   @!macro [new] schema.create_table.hash.description
      #     Create a table that manages record keys by hash table when
      #     you specify `:hash` to `:type`.
      #
      #     You can identify a record by key. The table is most fast
      #     table for finding a record by key. But the table doesn't
      #     support advanced key search features such as common prefix
      #     search and range search.
      #
      #     See description of `TABLE_HASH_KEY` at [Groonga documentation
      #     of tables](http://groonga.org/docs/reference/tables.html)
      #     for details.
      #
      #   @!macro [new] schema.create_table.hash.options
      #     @!macro schema.create_table.common.options
      #     @!macro schema.create_table.key_support.options
      #
      #   @!macro schema.create_table.hash.description
      #   @!macro schema.create_table.hash.options
      #
      # @overload create_table(name, options= {:type => :patricia_trie}, &block)
      #
      #   @!macro [new] schema.create_table.patricia_trie.description
      #     Create a table that manages record keys by patricia trie
      #     when you specify `:patricia_trie` to `:type`.
      #
      #     You can identify a record by key. The table is most small
      #     table. The table supports advanced key search features such
      #     as prefix search and range search. The table is suitable for
      #     lexicon of full-text search and index of range search.
      #
      #     See description of `TABLE_PAT_KEY` at [Groonga documentation
      #     of tables](http://groonga.org/docs/reference/tables.html)
      #     for details.
      #
      #   @!macro [new] schema.create_table.patricia_trie.options
      #     @!macro schema.create_table.common.options
      #     @!macro schema.create_table.key_support.options
      #
      #     @option options [Boolean] :key_with_sis (false)
      #
      #       +true+ を指定するとキーの文字列の
      #       全suffixが自動的に登録される。
      #
      #   @!macro schema.create_table.patricia_trie.description
      #   @!macro schema.create_table.patricia_trie.options
      #
      # @overload create_table(name, options= {:type => :double_array_trie})
      #
      #   @!macro [new] schema.create_table.double_array_trie.description
      #     Create a table that manages record keys by double array trie
      #     when you specify `:double_array_trie` to `:type`.
      #
      #     You can identify a record by key. The table is fast table
      #     for finding a record by key. But the table is large. The
      #     table supports advanced key search features such as prefix
      #     search and range search.
      #
      #     The table is the only table that supports renaming key. The
      #     table is used in Groonga database to manage object names
      #     such as `ShortText` and `TokenBigram`.
      #
      #     See description of `TABLE_DAT_KEY` at [Groonga documentation
      #     of tables](http://groonga.org/docs/reference/tables.html)
      #     for details.
      #
      #   @!macro [new] schema.create_table.double_array_trie.options
      #     @!macro schema.create_table.common.options
      #     @!macro schema.create_table.key_support.options
      #
      #   @!macro schema.create_table.double_array_trie.description
      #   @!macro schema.create_table.double_array_trie.options
      def create_table(name, options={}, &block)
        define do |schema|
          schema.create_table(name, options, &block)
        end
      end
      # 名前が _name_ のテーブルを削除する。
      # @param options [::Hash] The name and value
      #   pairs. Omitted names are initialized as the default value.
      # @option options :context (Groonga::Context.default)
      #
      #   スキーマ定義時に使用する {Groonga::Context} を指定する。
      def remove_table(name, options={})
        define do |schema|
          schema.remove_table(name, options)
        end
      end

      # 名前が _name_ のテーブルを変更する。以下の省略形。
      #
      # <pre>
      # !!!ruby
      # Groonga::Schema.define do |schema|
      #   schema.change_table(name, options) do |table|
      #     # ...
      #   end
      # end
      # </pre>
      #
      # ブロックには {Groonga::Schema::TableDefinition} オブジェ
      # クトがわたるので、そのオブジェクトを利用してテーブル
      # の詳細を定義する。
      #
      # @param options [::Hash] The name and value
      #   pairs. Omitted names are initialized as the default value.
      # @option options :context (Groonga::Context.default) The context
      #
      #   スキーマ定義時に使用する {Groonga::Context} を指定する。
      def change_table(name, options={}, &block)
        define do |schema|
          schema.change_table(name, options, &block)
        end
      end

      # (See Groonga::Schema#rename_table)
      #
      # This is a syntax sugar of the following code:
      #
      # <pre>
      # !!!ruby
      # Groonga::Schema.define do |schema|
      #   schema.rename_table(current_name, new_name, options)
      # end
      # </pre>
      def rename_table(current_name, new_name, options={})
        define do |schema|
          schema.rename_table(current_name, new_name, options)
        end
      end

      # 以下と同様:
      #
      # <pre>
      # !!!ruby
      # Groonga::Schema.change_table(table_name) do |table|
      #   table.remove_column(column_name)
      # end
      # </pre>
      def remove_column(table_name, column_name)
        change_table(table_name) do |table|
          table.remove_column(column_name)
        end
      end

      # This is a syntax sugar of the following:
      #
      # <pre>
      # !!!ruby
      # Groonga::Schema.define do |schema|
      #   schema.rename_column(table_name,
      #                        current_column_name, new_column_name)
      # end
      # </pre>
      def rename_column(table_name, current_column_name, new_column_name)
        define do |schema|
          schema.rename_column(table_name, current_column_name, new_column_name)
        end
      end

      # スキーマの内容を文字列をRubyスクリプト形式またはgrn式
      # 形式で返す。デフォルトはRubyスクリプト形式である。
      # Rubyスクリプト形式で返された値は
      # {Groonga::Schema.restore} することによりスキーマ内に組み
      # 込むことができる。
      #
      # dump.rb:
      #
      # <pre>
      # !!!ruby
      # File.open("/tmp/groonga-schema.rb", "w") do |schema|
      #   dumped_text = Groonga::Schema.dump
      # end
      # </pre>
      #
      # restore.rb:
      #
      # <pre>
      # !!!ruby
      # dumped_text = Groonga::Schema.dump
      # Groonga::Database.create(:path => "/tmp/new-db.grn")
      # Groonga::Schema.restore(dumped_text)
      # </pre>
      #
      # grn式形式で返された値はgroongaコマンドで読み込むこと
      # ができる。
      #
      # dump.rb:
      #
      # <pre>
      # !!!ruby
      # File.open("/tmp/groonga-schema.grn", "w") do |schema|
      #   dumped_text = Groonga::Schema.dump(:syntax => :command)
      # end
      # </pre>
      #
      # <pre>
      # !!!text
      # % groonga db/path < /tmp/groonga-schema.grn
      # </pre>
      #
      # @param options [::Hash] The name and value
      #   pairs. Omitted names are initialized as the default value.
      # @option options :context (Groonga::Context.default) The context
      #
      #   スキーマ定義時に使用する {Groonga::Context} を指定する。
      # @option options :syntax The syntax
      #
      #   スキーマの文字列の形式を指定する。指定可能な値は以下の通り。
      #   +:ruby+ Rubyスクリプト形式。省略した場合、+nil+ の場合も
      #   Rubyスクリプト形式になる。
      #   +:command+ groongaコマンド形式。groongaコマンドで読み込む
      #   ことができる。
      def dump(options={})
        schema = new(:context => options[:context],
                     :syntax => options[:syntax])
        schema.dump
      end

      # {Groonga::Schema.dump} で文字列化したスキーマを組み込む。
      def restore(dumped_text, options={})
        define(options) do |schema|
          schema.restore(dumped_text)
        end
      end

      # @private
      NORMALIZE_TYPE_TABLE = {
        "short_text" => "ShortText",
        "string" => "ShortText",
        "text" => "Text",
        "binary" => "LongText",
        "long_text" => "LongText",
        "int8" => "Int8",
        "integer8" => "Int8",
        "int16" => "Int16",
        "integer16" => "Int16",
        "int" => "Int32",
        "integer" => "Int32",
        "int32" => "Int32",
        "integer32" => "Int32",
        "decimal" => "Int64",
        "int64" => "Int64",
        "integer64" => "Int64",
        "uint8" => "UInt8",
        "unsigned_integer8" => "UInt8",
        "uint16" => "UInt16",
        "unsigned_integer16" => "UInt16",
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
        "tokyo_geo_point" => "TokyoGeoPoint",
        "geo_point" => "WGS84GeoPoint",
        "wgs84_geo_point" => "WGS84GeoPoint",
        "delimit" => "TokenDelimit",
        "token_delimit" => "TokenDelimit",
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
        "regexp" => "TokenRegexp",
        "token_regexp"=> "TokenRegexp",
      }
      # @private
      def normalize_type(type, options={})
        return type if type.nil?
        return type if type.is_a?(Groonga::Object)
        type = type.to_s if type.is_a?(Symbol)
        return type if (options[:context] || Groonga::Context.default)[type]
        NORMALIZE_TYPE_TABLE[type] || type
      end
    end

    # スキーマ定義を開始する。
    #
    # @param options [::Hash] The name and value
    #   pairs. Omitted names are initialized as the default value.
    # @option options :context (Groonga::Context.default) The context
    #   スキーマ定義時に使用する {Groonga::Context} を指定する。
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

    # {Groonga::Schema#dump} で返されたスキーマの内容を読み込む。
    #
    # 読み込まれた内容は {#define} を呼び出すまでは実行されない
    # ことに注意すること。
    def restore(dumped_text)
      instance_eval(dumped_text)
    end
    # for backward compatibility.
    # TODO: remove this at the next major release.
    alias_method :load, :restore

    # スキーマの内容を文字列で返す。返された値は
    # {Groonga::Schema#restore} することによりスキーマ内に組み込むことができる。
    def dump
      dumper = SchemaDumper.new(:context => @options[:context],
                                :syntax => @options[:syntax] || :ruby)
      dumper.dump
    end

    # 名前が _name_ のテーブルを作成する。
    #
    # テーブルの作成は {#define} を呼び出すまでは実行されないこ
    # とに注意すること。
    #
    # @overload create_table(name, options= {:type => :array}, &block)
    #   @!macro schema.create_table.array.description
    #   @!macro schema.create_table.array.options
    # @overload create_table(name, options= {:type => :hash}, &block)
    #   @!macro schema.create_table.hash.description
    #   @!macro schema.create_table.hash.options
    # @overload create_table(name, options= {:type => :patricia_trie}, &block)
    #   @!macro schema.create_table.patricia_trie.description
    #   @!macro schema.create_table.patricia_trie.options
    # @overload create_table(name, options= {:type => :double_array_trie})
    #   @!macro schema.create_table.double_array_trie.description
    #   @!macro schema.create_table.double_array_trie.options
    def create_table(name, options={})
      definition = TableDefinition.new(name, @options.merge(options || {}))
      yield(definition) if block_given?
      @definitions << definition
    end

    def create_lexicon(name,
                       use_case: :full_text_search,
                       **options, &block)
      case use_case
      when :full_text_search
        options[:type] ||= :patricia_trie
        options[:key_type] ||= :short_text
        options[:default_tokenizer] ||= "TokenBigram"
        options[:normalizer] ||= "NormalizerAuto"
      end
      create_table(name, options, &block)
    end

    # 名前が _name_ のテーブルを削除する。
    #
    # テーブルの削除は# {define} を呼び出すまでは実行されないこ
    # とに注意すること。
    #
    # @param options [::Hash] The name and value
    #   pairs. Omitted names are initialized as the default value.
    # @option options :context (Groonga::Context.default) The context
    #
    #   スキーマ定義時に使用する {Groonga::Context} を指定する。
    def remove_table(name, options={})
      definition = TableRemoveDefinition.new(name, @options.merge(options || {}))
      @definitions << definition
    end

    # 名前が _name_ のテーブルを変更する。
    #
    # テーブルの変更は {#define} を呼び出すまでは実行されないこ
    # とに注意すること。
    #
    # @param options [::Hash] The name and value
    #   pairs. Omitted names are initialized as the default value.
    # @option options :context (Groonga::Context.default) The context
    #
    #   スキーマ定義時に使用する {Groonga::Context} を指定する。
    def change_table(name, options={})
      options = @options.merge(options || {}).merge(:change => true)
      definition = TableDefinition.new(name, options)
      yield(definition)
      @definitions << definition
    end

    # Renames _current_name_ table to _new_name.
    #
    # Note that table renaming will will not be performed
    # until {#define} is called.
    #
    # @param options [::Hash] The name and value
    #   pairs. Omitted names are initialized as the default value.
    # @option options :context (Groonga::Context.default)
    #   The {Groonga::Context} to be used in renaming.
    def rename_table(current_name, new_name, options={})
      options = @options.merge(options || {})
      definition = TableRenameDefinition.new(current_name, new_name, options)
      @definitions << definition
    end

    # 以下と同様:
    #
    # <pre>
    # !!!ruby
    # schema.change_table(table_name) do |table|
    #   table.remove_column(column_name)
    # end
    # </pre>
    def remove_column(table_name, column_name)
      change_table(table_name) do |table|
        table.remove_column(column_name)
      end
    end

    # It is a syntax sugar of the following:
    #
    # <pre>
    # !!!ruby
    # schema.change_table(table_name) do |table|
    #   table.rename_column(current_column_name, new_column_name)
    # end
    # </pre>
    def rename_column(table_name, current_column_name, new_column_name)
      change_table(table_name) do |table|
        table.rename_column(current_column_name, new_column_name)
      end
    end

    # @private
    def context
      @options[:context] || Groonga::Context.default
    end

    # @private
    module Path
      def tables_directory_path(database)
        "#{database.path}.tables"
      end

      def columns_directory_path(table)
        "#{table.path}.columns"
      end

      def rmdir_if_available(dir)
        begin
          Dir.rmdir(dir)
        rescue SystemCallError
        end
      end

      private
      # @private
      def use_named_path?
        @options[:named_path]
      end
    end

    # スキーマ定義時に {Groonga::Schema.create_table} や
    # {Groonga::Schema#create_table} からブロックに渡されてくる
    # オブジェクト
    class TableDefinition
      include Path

      # テーブルの名前
      attr_reader :name

      # @private
      def initialize(name, options)
        @name = name
        @name = @name.to_s if @name.is_a?(Symbol)
        @definitions = []
        validate_options(options)
        @options = options
        @table_type = table_type
      end

      # @private
      def define
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

      # 名前が _name_ で型が _type_ のカラムを作成する。
      #
      # @param options [::Hash] The name and value
      #   pairs. Omitted names are initialized as the default value.
      # @option options :force
      #
      #   +true+ を指定すると既存の同名のカラムが
      #   存在していても、強制的に新しいカラムを作成する。
      # @option options :path
      #
      #   カラムを保存するパス。
      # @option options :persistent
      #
      #   +true+ を指定すると永続カラムとなる。 +:path+ を省略
      #   した場合は自動的にパスが付加される。
      # @option options :type (:scalar)
      #
      #   カラムの値の格納方法について指定する。
      #
      #   - :scalar := スカラ値(単独の値)を格納する。
      #   - :vector := 値の配列を格納する。
      # @option options [Boolean] :with_weight (false)
      #    It specifies whether making the column weight vector column or not.
      #    Weight vector column can store weight for each element.
      #
      #    You can't use this option for scalar column.
      # @option options :compress
      #
      #   値の圧縮方法を指定する。省略した場合は、圧縮しない。
      #
      #   * `:zlib`: Compressed by zlib.
      #   * `:lz4`: Compzressed by LZ4.
      #   * `:zstd`: Compressed by Zstandard.
      #   * `:zstandard`: Compressed by Zstandard.
      # @option options [Boolean] :weight_float32 (false)
      #    It specifies whether the column uses 32 bit float for weight or not.
      #
      #    You can't use this option for scalar column.
      #
      #   @since 12.0.2
      # @option options [:add, :ignore, :nil, nil] :missing_mode (nil)
      #   It specifies how to process missing value.
      #
      #   * `:add`, `nil`: Correspond to `MISSING_ADD`
      #   * `:ignore`: Correspond to `MISSING_IGNORE`
      #   * `:nil`: Correspond to `MISSING_NIL`
      #
      #   See
      #   https://groonga.org/docs/reference/commands/column_create.html#column-create-missing-mode
      #   for each `MISSING_*` values.
      #
      #   @since 12.0.2
      # @option options [:error, :warn, :ignore, nil] :invalid_mode (nil)
      #   It specifies how to process invalid value.
      #
      #   * `:add`, `nil`: Correspond to `INVALID_ERROR`
      #   * `:warn`: Correspond to `INVALID_WARN`
      #   * `:ignore`: Correspond to `INVALID_IGNORE`
      #
      #   See
      #   https://groonga.org/docs/reference/commands/column_create.html#column-create-invalid-mode
      #   for each `INVALID_*` values.
      #
      #   @since 12.0.2
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

      # 名前が _name_ のカラムを削除します。
      #
      # _options_ に指定可能な値はありません(TODO _options_ は不要?)。
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

      # Renames _current_name_ column to _new_name_ column.
      #
      # @param [::Hash] options The name and value
      #   pairs. Omitted names are initialized as the default
      #   value.
      # @option options [Groonga:Context] :context (Groonga::Context.default)
      #   The context to be used in renaming.
      def rename_column(current_name, new_name, options={})
        definition = self[name, ColumnRenameDefinition]
        if definition.nil?
          definition = ColumnRenameDefinition.new(current_name, new_name,
                                                  options)
          update_definition(name, ColumnRenameDefinition, definition)
        end
        definition.options.merge!(options)
        self
      end

      # _target_table_ の _target_column_ を対象とするインデック
      # スカラムを作成します。複数のカラムを指定することもで
      # きます。
      #
      # _target_column_full_name_ で指定するときはテーブル名
      # とカラム名を"."でつなげます。
      #
      # 例えば、「Users」テーブルの「name」カラムのインデックスカラムを
      # 指定する場合はこうなります。
      #
      # @example
      #   table.index("Users.name")
      #
      # @param [Array] args
      #   インデックスカラム作成時に指定できるオプション。
      #   ハッシュを使って次の要素を指定することができる。
      #
      #   - :name :=
      #     インデックスカラムのカラム名を任意に指定する。 =:
      #   - :force :=
      #     +true+ を指定すると既存の同名のカラムが
      #     存在していても、強制的に新しいカラムを作成する。 =:
      #   - :path :=
      #     カラムを保存するパス。 =:
      #   - :persistent :=
      #     +true+ を指定すると永続カラムとなる。
      #     +:path+ を省略した場合は自動的にパスが付加される。 =:
      #   - :with_section :=
      #     +true+ を指定すると転置索引にsection(段落情報)を
      #     合わせて格納する。未指定または +nil+ を指定した場合、
      #     複数のカラムを指定すると自動的に有効になる。 =:
      #   - :with_weight :=
      #     +true+ を指定すると転置索引にweight情報を合わせて
      #     格納する。 =:
      #   - :with_position :=
      #     +true+ を指定すると転置索引に出現位置情報を合わせて
      #     格納する。未指定または +nil+ を指定した場合、テーブル
      #     がトークナイザー利用している場合は自動的に有効になる。
      #     +TokenDelimit+ など全文検索用ではないトークナイザーを
      #     使う場合は明示的に +false+ を指定することで使用リソース
      #     を少なくできる。=:
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

      # _target_table_ の _target_column_ を対象とするインデッ
      # クスカラムを削除します。
      #
      # _target_column_full_name_ で指定するときはテーブル名
      # とカラム名を"."でつなげます。
      #
      # 例えば、「Users」テーブルの「name」カラムのインデックスカラムを
      # 削除する場合はこうなります。
      #
      # @example
      #   table.remove_index("Users.name")
      #
      # @param [::Hash] args { :name => target_column }と指定す
      #   ることでインデックスカラムの任意のカラム名を指定することができる。
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

      # Defines a 8 bit signed integer column named @name@.
      #
      # @param [String or Symbol] name the column name
      # @param [Hash] options ({}) the options
      # @see #column #column for available options.
      def integer8(name, options={})
        column(name, "Int8", options)
      end
      alias_method :int8, :integer8

      # Defines a 16 bit signed integer column named @name@.
      #
      # @param [String or Symbol] name the column name
      # @param [Hash] options ({}) the options
      # @see #column #column for available options.
      def integer16(name, options={})
        column(name, "Int16", options)
      end
      alias_method :int16, :integer16

      # 名前が _name_ の32bit符号付き整数のカラムを作成する。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def integer32(name, options={})
        column(name, "Int32", options)
      end
      alias_method :integer, :integer32
      alias_method :int32, :integer32

      # 名前が _name_ の64bit符号付き整数のカラムを作成する。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def integer64(name, options={})
        column(name, "Int64", options)
      end
      alias_method :int64, :integer64

      # Defines a 8 bit unsigned integer column named @name@.
      #
      # @param [String or Symbol] name the column name
      # @param [Hash] options ({}) the options
      # @see #column #column for available options.
      def unsigned_integer8(name, options={})
        column(name, "UInt8", options)
      end
      alias_method :uint8, :unsigned_integer8

      # Defines a 16 bit unsigned integer column named @name@.
      #
      # @param [String or Symbol] name the column name
      # @param [Hash] options ({}) the options
      # @see #column #column for available options.
      def unsigned_integer16(name, options={})
        column(name, "UInt16", options)
      end
      alias_method :uint16, :unsigned_integer16

      # 名前が _name_ の32bit符号なし整数のカラムを作成する。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def unsigned_integer32(name, options={})
        column(name, "UInt32", options)
      end
      alias_method :unsigned_integer, :unsigned_integer32
      alias_method :uint32, :unsigned_integer32

      # 名前が _name_ の64bit符号なし整数のカラムを作成する。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def unsigned_integer64(name, options={})
        column(name, "UInt64", options)
      end
      alias_method :uint64, :unsigned_integer64

      # 名前が _name_ のieee754形式の64bit浮動小数点数のカラム
      # を作成する。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def float(name, options={})
        column(name, "Float", options)
      end

      # 名前が _name_ の64bit符号付き整数で1970年1月1日0時0分
      # 0秒からの経過マイクロ秒数を格納するカラムを作成する。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def time(name, options={})
        column(name, "Time", options)
      end

      # 以下と同様:
      # <pre>
      # !!!ruby
      # table.time("updated_at")
      # table.time("created_at")
      # </pre>
      def timestamps(options={})
        time("created_at", options)
        time("updated_at", options)
      end

      # 名前が _name_ の4Kbyte以下の文字列を格納できるカラムを
      # 作成する。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def short_text(name, options={})
        column(name, "ShortText", options)
      end
      alias_method :string, :short_text

      # 名前が _name_ の64Kbyte以下の文字列を格納できるカラムを
      # 作成する。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def text(name, options={})
        column(name, "Text", options)
      end

      # 名前が _name_ の2Gbyte以下の文字列を格納できるカラムを
      # 作成する。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def long_text(name, options={})
        column(name, "LongText", options)
      end

      # 名前が _name_ で _table_ のレコードIDを格納する参照カラ
      # ムを作成する。
      #
      # _table_ が省略された場合は _name_ の複数形が使われる。
      # 例えば、 _name_ が"user"な場合は _table_ は"users"になる。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def reference(name, table=nil, options={})
        table ||= lambda {|context| guess_table_name(context, name)}
        column(name, table, options)
      end

      # 名前が _name_ の真偽値を格納できるカラムを作成する。
      #
      # _options_ に指定可能な値は
      # {Groonga::Schema::TableDefinition#column} を参照。
      def boolean(name, options={})
        column(name, "Bool", options)
      end
      alias_method :bool, :boolean

      # Defines a geo point in Tokyo geodetic system column
      # named @name@.
      #
      # @param [String or Symbol] name the column name
      # @param [Hash] options ({}) the options
      # @see #column #column for available options.
      def tokyo_geo_point(name, options={})
        column(name, "TokyoGeoPoint", options)
      end

      # Defines a geo point in WGS 84 (World Geodetic System) column
      # named @name@.
      #
      # @param [String or Symbol] name the column name
      # @param [Hash] options ({}) the options
      # @see #column #column for available options.
      def wgs84_geo_point(name, options={})
        column(name, "WGS84GeoPoint", options)
      end
      alias_method :geo_point, :wgs84_geo_point

      # @private
      def [](name, definition_class=nil)
        @definitions.find do |definition|
          definition.name.to_s == name.to_s and
            (definition_class.nil? or definition.is_a?(definition_class))
        end
      end

      # @private
      def context
        @options[:context] || Groonga::Context.default
      end

      private
      # @private
      def update_definition(key, definition_class, definition)
        old_definition = self[key, definition_class]
        if old_definition
          index = @definitions.index(old_definition)
          @definitions[index] = definition
        else
          @definitions << definition
        end
      end

      # @private
      AVAILABLE_OPTION_KEYS = [:context, :change, :force,
                               :type, :path, :persistent,
                               :key_type, :value_type, :sub_records,
                               :default_tokenizer,
                               :token_filters,
                               :key_normalize, :key_with_sis,
                               :named_path,
                               :normalizer]
      # @private
      def validate_options(options)
        return if options.nil?
        unknown_keys = options.keys - AVAILABLE_OPTION_KEYS
        unless unknown_keys.empty?
          raise UnknownOptions.new(options, unknown_keys, AVAILABLE_OPTION_KEYS)
        end
      end

      # @private
      def table_type
        type = @options[:type]
        if type.nil?
          if @options[:key_type]
            type = :hash
          else
            type = :array
          end
        end

        case type
        when :array
          Groonga::Array
        when :hash
          Groonga::Hash
        when :patricia_trie
          Groonga::PatriciaTrie
        when :double_array_trie
          Groonga::DoubleArrayTrie
        else
          supported_types = [
            nil,
            :array,
            :hash,
            :patricia_trie,
            :double_array_trie,
          ]
          raise UnknownTableType.new(type, supported_types)
        end
      end

      # @private
      def create_options
        common = {
          :name => @name,
          :path => path,
          :persistent => persistent?,
          :value_type => @options[:value_type],
          :context => context,
          :sub_records => @options[:sub_records],
        }
        token_filters = @options[:token_filters]
        if token_filters
          token_filters = token_filters.collect do |token_filter|
            normalize_type(token_filter)
          end
        end
        key_support_table_common = {
          :key_type => normalize_key_type(@options[:key_type] || "ShortText"),
          :key_normalize => @options[:key_normalize],
          :default_tokenizer => normalize_type(@options[:default_tokenizer]),
          :token_filters => token_filters,
          :normalizer => normalize_type(@options[:normalizer]),
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
        elsif @table_type == Groonga::DoubleArrayTrie
          common.merge(key_support_table_common)
        end
      end

      def path
        user_path = @options[:path]
        return user_path if user_path
        return nil unless use_named_path?
        tables_dir = tables_directory_path(context.database)
        FileUtils.mkdir_p(tables_dir)
        File.join(tables_dir, @name)
      end

      # @private
      def column_options
        {:persistent => persistent?, :named_path => use_named_path?}
      end

      # @private
      def persistent?
        @options[:persistent].nil? ? true : @options[:persistent]
      end

      def parse_index_argument(target_table_or_target_column_full_name, *args)
        options = nil
        options = args.pop if args.last.is_a?(::Hash)
        options ||= {}
        if args.empty?
          target_column_full_name = target_table_or_target_column_full_name
          if target_column_full_name.is_a?(Groonga::Column)
            target_column_full_name = target_column_full_name.name
          end
          target_table, target_column = target_column_full_name.split(/\./, 2)
          target_columns = [target_column].compact
          key = [target_table, target_columns]
        else
          target_table_name = target_table_or_target_column_full_name
          target_table = lambda do |context|
            guess_table_name(context, target_table_name)
          end
          target_columns = args
          key = [target_table_name, target_columns]
        end
        if target_columns.size > 1 and options[:with_section].nil?
          options[:with_section] = true
        end
        [key, target_table, target_columns, options]
      end

      def same_table?(table, options)
        return false unless table.class == @table_type
        return false unless table.range == resolve_name(options[:value_type])
        sub_records = options[:sub_records]
        sub_records = false if sub_records.nil?
        return false unless table.have_n_sub_records_space? == sub_records

        case table
        when Groonga::Array
          true
        when Groonga::Hash, Groonga::PatriciaTrie, Groonga::DoubleArrayTrie
          key_type = normalize_key_type(options[:key_type])
          return false unless table.domain == resolve_name(key_type)

          default_tokenizer = normalize_type(options[:default_tokenizer])
          default_tokenizer = resolve_name(default_tokenizer)
          return false unless table.default_tokenizer == default_tokenizer

          token_filters = options[:token_filters] || []
          token_filters = token_filters.collect do |token_filter|
            resolve_name(normalize_type(token_filter))
          end
          return false unless table.token_filters == token_filters

          normalizer = normalize_type(options[:normalizer])
          normalizer ||= default_normalizer_name if options[:key_normalize]
          normalizer = resolve_name(normalizer)
          return false unless table.normalizer == normalizer

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

      def default_normalizer_name
        "NormalizerAuto"
      end

      def normalize_key_type(key_type)
        normalize_type(key_type || "ShortText")
      end

      def normalize_type(type)
        Schema.normalize_type(type, :context => context)
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

    # @private
    class TableRemoveDefinition
      include Path

      def initialize(name, options={})
        @name = name
        @options = options
      end

      def define
        table = removed_table
        tables_dir = tables_directory_path(context.database)
        columns_dir = columns_directory_path(table)
        result = table.remove
        rmdir_if_available(columns_dir)
        rmdir_if_available(tables_dir)
        result
      end

      private
      def context
        @options[:context]
      end

      def removed_table
        table = context[@name]
        raise TableNotExists.new(@name) if table.nil?
        table
      end
    end

    # @private
    class TableRenameDefinition
      include Path

      def initialize(current_name, new_name, options={})
        @current_name = current_name
        @new_name = new_name
        @options = options
      end

      def define
        table = current_table
        table.rename(@new_name)
      end

      private
      def context
        @options[:context]
      end

      def current_table
        table = context[@current_name]
        raise TableNotExists.new(@current_name) if table.nil?
        table
      end
    end

    # @private
    class ColumnDefinition
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
                            resolved_type(context),
                            options)
      end

      private
      def resolved_type(context)
        return @type if @type.is_a?(Groonga::Object)
        if @type.respond_to?(:call)
          resolved_type_name = @type.call(context)
        else
          resolved_type_name = @type
        end
        normalized_type_name = Schema.normalize_type(resolved_type_name,
                                                     :context => context)
        context[normalized_type_name]
      end

      def same_column?(context, column)
        return false unless column.range == resolved_type(context)
        if column.scalar?
          [nil, :scalar].include?(@options[:type])
        else
          return false unless @options[:type] == :vector
          with_weight = @options[:with_weight]
          with_weight = false if with_weight.nil?
          column.with_weight? == with_weight
        end
      end

      def define_options(context, table)
        {
          :path => path(context, table),
          :type => @options[:type],
          :with_weight => @options[:with_weight],
          :compress => @options[:compress],
          :weight_float32 => @options[:weight_float32],
          :missing_mode => @options[:missing_mode],
          :invalid_mode => @options[:invalid_mode],
        }
      end

      def path(context, table)
        user_path = @options[:path]
        return user_path if user_path
        return nil unless use_named_path?
        columns_dir = columns_directory_path(table)
        FileUtils.mkdir_p(columns_dir)
        File.join(columns_dir, @name)
      end
    end

    # @private
    class ColumnRemoveDefinition
      include Path

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
        column = table.column(name)

        if column.nil?
          raise ColumnNotExists.new(name)
        end

        result = column.remove
        columns_dir = columns_directory_path(table)
        rmdir_if_available(columns_dir)
        result
      end
    end

    # @private
    class ColumnRenameDefinition
      include Path

      attr_accessor :current_name, :new_name
      attr_reader :options

      def initialize(current_name, new_name, options={})
        @current_name = current_name
        @current_name = @current_name.to_s if @current_name.is_a?(Symbol)
        @new_name = new_name
        @new_name = @new_name.to_s if @new_name.is_a?(Symbol)
        @options = (options || {}).dup
      end

      def define(table_definition, table)
        if @current_name.respond_to?(:call)
          current_name = @current_name.call(table_definition.context)
        else
          current_name = @current_name
        end
        column = table.column(current_name)

        if column.nil?
          raise ColumnNotExists.new(name)
        end

        column.rename(@new_name)
      end
    end

    # @private
    class IndexColumnDefinition
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
          :size => @options[:size],
        }
      end

      def path(context, table, name)
        user_path = @options[:path]
        return user_path if user_path
        return nil unless use_named_path?
        columns_dir = "#{table.path}.columns"
        FileUtils.mkdir_p(columns_dir)
        File.join(columns_dir, name)
      end
    end
  end
end
