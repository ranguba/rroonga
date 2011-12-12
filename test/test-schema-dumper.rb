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

  def define_reference_schema
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

  def define_index_schema
    Groonga::Schema.define do |schema|
      schema.create_table("Items",
                          :type => :hash,
                          :key_type => "ShortText") do |table|
        table.short_text("title")
      end

      schema.create_table("Terms",
                          :type => :patricia_trie,
                          :key_type => "ShortText",
                          :key_normalize => true,
                          :default_tokenizer => "TokenBigram") do |table|
        table.index("Items", "_key")
        table.index("Items", "title")
      end
    end
  end

  class RubySyntaxSchemaDumperTest < SchemaDumperTest
    def test_simple
      define_simple_schema
      assert_equal(<<-EOS, dump)
create_table("Posts",
             :force => true) do |table|
  table.short_text("comments", :type => :vector)
  table.short_text("title")
end
EOS
    end

    def test_built_in_types
      define_built_in_types_schema
      assert_equal(<<-EOS, dump)
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
EOS
    end

    def test_reference
      define_reference_schema
      assert_equal(<<-EOS, dump)
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
EOS
    end

    def test_index
      define_index_schema
      assert_equal(<<-EOS, dump)
create_table("Items",
             :type => :hash,
             :key_type => "ShortText",
             :force => true) do |table|
  table.short_text("title")
end

create_table("Terms",
             :type => :patricia_trie,
             :key_type => "ShortText",
             :key_normalize => true,
             :default_tokenizer => "TokenBigram",
             :force => true) do |table|
end

change_table("Terms") do |table|
  table.index("Items", "_key", :name => "Items__key")
  table.index("Items", "title", :name => "Items_title")
end
EOS
    end

    private
    def dump
      Groonga::Schema.dump
    end
  end

  class CommandSyntaxSchemaDumperTest < SchemaDumperTest
    def test_simple
      define_simple_schema
      assert_equal(<<-EOS, dump)
table_create Posts TABLE_NO_KEY
column_create Posts comments COLUMN_VECTOR ShortText
column_create Posts title COLUMN_SCALAR ShortText
EOS
    end

    def test_reference
      define_reference_schema
      assert_equal(<<-EOS, dump)
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
EOS
    end

    def test_index
      define_index_schema
      assert_equal(<<-EOS, dump)
table_create Items TABLE_HASH_KEY --key_type ShortText
column_create Items title COLUMN_SCALAR ShortText

table_create Terms TABLE_PAT_KEY|KEY_NORMALIZE --key_type ShortText --default_tokenizer TokenBigram

column_create Terms Items__key COLUMN_INDEX|WITH_POSITION Items _key
column_create Terms Items_title COLUMN_INDEX|WITH_POSITION Items title
EOS
    end

    private
    def dump
      Groonga::Schema.dump(:syntax => :command)
    end
  end
end
