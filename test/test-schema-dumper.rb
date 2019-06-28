# Copyright (C) 2009-2016  Kouhei Sutou <kou@clear-code.com>
# Copyright (C) 2014-2016  Masafumi Yokoyama <yokoyama@clear-code.com>
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

class SchemaDumperTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  private
  def define_simple_schema
    Groonga::Schema.define do |schema|
      schema.create_table("Posts") do |table|
        table.short_text :title
        table.short_text :comments, :type => :vector
      end
    end
  end

  def define_built_in_types_schema
    Groonga::Schema.define do |schema|
      schema.create_table("Posts") do |table|
        table.boolean :public
        table.int8 :int8
        table.uint8 :uint8
        table.int16 :int16
        table.uint16 :uint16
        table.int32 :int32
        table.uint32 :uint32
        table.int64 :int64
        table.uint64 :uint64
        table.float :vote_average
        table.time :published_at
        table.short_text :title
        table.text :content
        table.long_text :attachment
        table.tokyo_geo_point :location_tokyo
        table.wgs84_geo_point :location_wgs84
      end
    end
  end

  def define_reference_table_schema
    Groonga::Schema.define do |schema|
      schema.create_table("Terms",
                          :type => :hash,
                          :key_type => :short_text) do |table|
      end

      schema.create_table("IndexTerms",
                          :type => :hash,
                          :key_type => "Terms") do |table|
      end
    end
  end

  def define_reference_column_schema
    Groonga::Schema.define do |schema|
      schema.create_table("Items") do |table|
        table.short_text("title")
      end

      schema.create_table("Users") do |table|
        table.short_text("name")
      end

      schema.create_table("Comments") do |table|
        table.reference("item", "Items")
        table.reference("author", "Users")
        table.reference("children", "Items", :type => :vector)
        table.text("content")
        table.time("issued")
      end
    end
  end

  def define_index_schema(options={})
    context.register_plugin("token_filters/stop_word")
    Groonga::Schema.define do |schema|
      schema.create_table("Items",
                          :type => :hash,
                          :key_type => "ShortText") do |table|
        table.short_text("title")
      end

      schema.create_table("Terms",
                          :type => :patricia_trie,
                          :key_type => "ShortText",
                          :default_tokenizer => "TokenBigram",
                          :token_filters => ["TokenFilterStopWord"],
                          :normalizer => "NormalizerAuto") do |table|
        table.index("Items", "_key", options[:key_index_options] || {})
        table.index("Items", "title", options[:title_index_options] || {})
      end
    end
  end

  def define_weight_vector_schema
    Groonga::Schema.define do |schema|
      schema.create_table("Memos",
                          :type => :patricia_trie,
                          :key_type => "ShortText") do |table|
        table.short_text("tags",
                         :type => :vector,
                         :with_weight => true)
      end
    end
  end

  def define_double_array_trie_schema
    Groonga::Schema.define do |schema|
      schema.create_table("Accounts",
                          :type => :double_array_trie,
                          :key_type => "ShortText") do |table|
        table.short_text("name")
      end
    end
  end

  class RubySyntaxSchemaDumperTest < SchemaDumperTest
    def test_simple
      define_simple_schema
      assert_equal(<<-SCHEMA, dump)
create_table("Posts",
             :force => true) do |table|
  table.short_text("comments", :type => :vector)
  table.short_text("title")
end
      SCHEMA
    end

    def test_built_in_types
      define_built_in_types_schema
      assert_equal(<<-SCHEMA, dump)
create_table("Posts",
             :force => true) do |table|
  table.long_text("attachment")
  table.text("content")
  table.integer16("int16")
  table.integer32("int32")
  table.integer64("int64")
  table.integer8("int8")
  table.tokyo_geo_point("location_tokyo")
  table.wgs84_geo_point("location_wgs84")
  table.boolean("public")
  table.time("published_at")
  table.short_text("title")
  table.unsigned_integer16("uint16")
  table.unsigned_integer32("uint32")
  table.unsigned_integer64("uint64")
  table.unsigned_integer8("uint8")
  table.float("vote_average")
end
      SCHEMA
    end

    def test_reference_table
      define_reference_table_schema
      assert_equal(<<-SCHEMA, dump)
create_table("Terms",
             :type => :hash,
             :key_type => "ShortText",
             :force => true) do |table|
end

create_table("IndexTerms",
             :type => :hash,
             :key_type => "Terms",
             :force => true) do |table|
end
      SCHEMA
    end

    def test_reference_column
      define_reference_column_schema
      assert_equal(<<-SCHEMA, dump)
create_table("Comments",
             :force => true) do |table|
  table.text("content")
  table.time("issued")
end

create_table("Items",
             :force => true) do |table|
  table.short_text("title")
end

create_table("Users",
             :force => true) do |table|
  table.short_text("name")
end

change_table("Comments") do |table|
  table.reference("author", "Users")
  table.reference("children", "Items", :type => :vector)
  table.reference("item", "Items")
end
      SCHEMA
    end

    def test_index
      define_index_schema
      assert_equal(<<-SCHEMA, dump)
create_table("Items",
             :type => :hash,
             :key_type => "ShortText",
             :force => true) do |table|
  table.short_text("title")
end

create_table("Terms",
             :type => :patricia_trie,
             :key_type => "ShortText",
             :default_tokenizer => "TokenBigram",
             :token_filters => ["TokenFilterStopWord"],
             :normalizer => "NormalizerAuto",
             :force => true) do |table|
end

change_table("Terms") do |table|
  table.index("Items", "_key", :name => "Items__key", :with_position => true)
  table.index("Items", "title", :name => "Items_title", :with_position => true)
end
      SCHEMA
    end

    def test_small_index
      define_index_schema(:title_index_options => {:size => :small})
      assert_equal(<<-SCHEMA, dump)
create_table("Items",
             :type => :hash,
             :key_type => "ShortText",
             :force => true) do |table|
  table.short_text("title")
end

create_table("Terms",
             :type => :patricia_trie,
             :key_type => "ShortText",
             :default_tokenizer => "TokenBigram",
             :token_filters => ["TokenFilterStopWord"],
             :normalizer => "NormalizerAuto",
             :force => true) do |table|
end

change_table("Terms") do |table|
  table.index("Items", "_key", :name => "Items__key", :with_position => true)
  table.index("Items", "title", :name => "Items_title", :with_position => true, :size => :small)
end
      SCHEMA
    end

    def test_medium_index
      define_index_schema(:title_index_options => {:size => :medium})
      assert_equal(<<-SCHEMA, dump)
create_table("Items",
             :type => :hash,
             :key_type => "ShortText",
             :force => true) do |table|
  table.short_text("title")
end

create_table("Terms",
             :type => :patricia_trie,
             :key_type => "ShortText",
             :default_tokenizer => "TokenBigram",
             :token_filters => ["TokenFilterStopWord"],
             :normalizer => "NormalizerAuto",
             :force => true) do |table|
end

change_table("Terms") do |table|
  table.index("Items", "_key", :name => "Items__key", :with_position => true)
  table.index("Items", "title", :name => "Items_title", :with_position => true, :size => :medium)
end
      SCHEMA
    end

    def test_weight_vector
      define_weight_vector_schema
      assert_equal(<<-SCHEMA, dump)
create_table("Memos",
             :type => :patricia_trie,
             :key_type => "ShortText",
             :force => true) do |table|
  table.short_text("tags", :type => :vector, :with_weight => true)
end
      SCHEMA
    end

    def test_double_array_trie
      define_double_array_trie_schema
      assert_equal(<<-SCHEMA, dump)
create_table("Accounts",
             :type => :double_array_trie,
             :key_type => "ShortText",
             :force => true) do |table|
  table.short_text("name")
end
      SCHEMA
    end

    private
    def dump
      Groonga::Schema.dump
    end
  end

  class CommandSyntaxSchemaDumperTest < SchemaDumperTest
    def test_simple
      define_simple_schema
      assert_equal(<<-SCHEMA, dump)
table_create Posts TABLE_NO_KEY
column_create Posts comments COLUMN_VECTOR ShortText
column_create Posts title COLUMN_SCALAR ShortText
      SCHEMA
    end

    def test_reference_table
      define_reference_table_schema
      assert_equal(<<-SCHEMA, dump)
table_create Terms TABLE_HASH_KEY ShortText

table_create IndexTerms TABLE_HASH_KEY Terms
      SCHEMA
    end

    def test_reference_column
      define_reference_column_schema
      assert_equal(<<-SCHEMA, dump)
table_create Comments TABLE_NO_KEY
column_create Comments content COLUMN_SCALAR Text
column_create Comments issued COLUMN_SCALAR Time

table_create Items TABLE_NO_KEY
column_create Items title COLUMN_SCALAR ShortText

table_create Users TABLE_NO_KEY
column_create Users name COLUMN_SCALAR ShortText

column_create Comments author COLUMN_SCALAR Users
column_create Comments children COLUMN_VECTOR Items
column_create Comments item COLUMN_SCALAR Items
      SCHEMA
    end

    def test_index
      define_index_schema
      assert_equal(<<-SCHEMA, dump)
table_create Items TABLE_HASH_KEY ShortText
column_create Items title COLUMN_SCALAR ShortText

table_create Terms TABLE_PAT_KEY ShortText --default_tokenizer TokenBigram --token_filters TokenFilterStopWord --normalizer NormalizerAuto

column_create Terms Items__key COLUMN_INDEX|WITH_POSITION Items _key
column_create Terms Items_title COLUMN_INDEX|WITH_POSITION Items title
      SCHEMA
    end

    def test_small_index
      define_index_schema(:title_index_options => {:size => :small})
      assert_equal(<<-SCHEMA, dump)
table_create Items TABLE_HASH_KEY ShortText
column_create Items title COLUMN_SCALAR ShortText

table_create Terms TABLE_PAT_KEY ShortText --default_tokenizer TokenBigram --token_filters TokenFilterStopWord --normalizer NormalizerAuto

column_create Terms Items__key COLUMN_INDEX|WITH_POSITION Items _key
column_create Terms Items_title COLUMN_INDEX|WITH_POSITION|INDEX_SMALL Items title
      SCHEMA
    end

    def test_medium_index
      define_index_schema(:title_index_options => {:size => :medium})
      assert_equal(<<-SCHEMA, dump)
table_create Items TABLE_HASH_KEY ShortText
column_create Items title COLUMN_SCALAR ShortText

table_create Terms TABLE_PAT_KEY ShortText --default_tokenizer TokenBigram --token_filters TokenFilterStopWord --normalizer NormalizerAuto

column_create Terms Items__key COLUMN_INDEX|WITH_POSITION Items _key
column_create Terms Items_title COLUMN_INDEX|WITH_POSITION|INDEX_MEDIUM Items title
      SCHEMA
    end

    def test_large_index
      define_index_schema(:title_index_options => {:size => :large})
      assert_equal(<<-SCHEMA, dump)
table_create Items TABLE_HASH_KEY ShortText
column_create Items title COLUMN_SCALAR ShortText

table_create Terms TABLE_PAT_KEY ShortText --default_tokenizer TokenBigram --token_filters TokenFilterStopWord --normalizer NormalizerAuto

column_create Terms Items__key COLUMN_INDEX|WITH_POSITION Items _key
column_create Terms Items_title COLUMN_INDEX|WITH_POSITION|INDEX_LARGE Items title
      SCHEMA
    end

    def test_weight_vector
      define_weight_vector_schema
      assert_equal(<<-SCHEMA, dump)
table_create Memos TABLE_PAT_KEY ShortText
column_create Memos tags COLUMN_VECTOR|WITH_WEIGHT ShortText
      SCHEMA
    end

    def test_double_array_trie
      define_double_array_trie_schema
      assert_equal(<<-SCHEMA, dump)
table_create Accounts TABLE_DAT_KEY ShortText
column_create Accounts name COLUMN_SCALAR ShortText
      SCHEMA
    end

    class ColumnCompressionTest < self
      def test_zlib
        define_column_compression_zlib_schema
        flags = "COLUMN_SCALAR"
        flags << "|COMPRESS_ZLIB" if context.support_zlib?
        assert_equal(<<-SCHEMA, dump)
table_create Posts TABLE_NO_KEY
column_create Posts title #{flags} ShortText
        SCHEMA
      end

      def test_lz4
        define_column_compression_lz4_schema
        flags = "COLUMN_SCALAR"
        flags << "|COMPRESS_LZ4" if context.support_lz4?
        assert_equal(<<-SCHEMA, dump)
table_create Posts TABLE_NO_KEY
column_create Posts title #{flags} ShortText
        SCHEMA
      end

      def test_zstd
        define_column_compression_zstd_schema
        flags = "COLUMN_SCALAR"
        flags << "|COMPRESS_ZSTD" if context.support_zstd?
        assert_equal(<<-SCHEMA, dump)
table_create Posts TABLE_NO_KEY
column_create Posts title #{flags} ShortText
        SCHEMA
      end

      def test_with_weight_vector
        define_column_compression_with_weight_vector_schema
        flags = "COLUMN_VECTOR|WITH_WEIGHT"
        flags << "|COMPRESS_ZLIB" if context.support_zlib?
        assert_equal(<<-SCHEMA, dump)
table_create Posts TABLE_NO_KEY
column_create Posts comments #{flags} ShortText
        SCHEMA
      end

      private
      def define_column_compression_zlib_schema
        Groonga::Schema.define do |schema|
          schema.create_table("Posts") do |table|
            table.short_text("title", :compress => :zlib)
          end
        end
      end

      def define_column_compression_lz4_schema
        Groonga::Schema.define do |schema|
          schema.create_table("Posts") do |table|
            table.short_text("title", :compress => :lz4)
          end
        end
      end

      def define_column_compression_zstd_schema
        Groonga::Schema.define do |schema|
          schema.create_table("Posts") do |table|
            table.short_text("title", :compress => :zstd)
          end
        end
      end

      def define_column_compression_with_weight_vector_schema
        Groonga::Schema.define do |schema|
          schema.create_table("Posts") do |table|
            table.short_text("comments",
                             :type => :vector,
                             :with_weight => true,
                             :compress => :zlib)
          end
        end
      end
    end

    private
    def dump
      Groonga::Schema.dump(:syntax => :command)
    end
  end
end
