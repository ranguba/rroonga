# Copyright (C) 2011-2014  Kouhei Sutou <kou@clear-code.com>
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

class DatabaseDumperTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database, :before => :append

  setup
  def setup_tables
    Groonga::Schema.define do |schema|
      schema.create_table("Tags",
                          :type => :hash,
                          :key_type => :short_text,
                          :default_tokenizer => :delimit) do |table|
        table.text("name")
      end

      schema.create_table("Users",
                          :type => :hash,
                          :key_type => "ShortText") do |table|
        table.text("name")
      end

      schema.create_table("Posts") do |table|
        table.text("title")
        table.reference("author", "Users")
        table.integer("rank")
        table.unsigned_integer("n_goods")
        table.short_text("tag_text")
        table.reference("tags", "Tags", :type => :vector)
        table.boolean("published")
        table.time("created_at")
      end

      schema.change_table("Users") do |table|
        table.index("Posts.author")
      end

      schema.change_table("Tags") do |table|
        table.index("Posts.tag_text", :with_position => false)
      end
    end
  end

  private
  def dump(options={})
    Groonga::DatabaseDumper.new(options).dump
  end

  def posts
    context["Posts"]
  end

  def users
    context["Users"]
  end

  def dumped_schema
    <<-SCHEMA
#{dumped_schema_tables}

#{dumped_schema_reference_columns}

#{dumped_schema_index_columns}
    SCHEMA
  end

  def dumped_schema_tables
    <<-SCHEMA.chomp
table_create Posts TABLE_NO_KEY
column_create Posts created_at COLUMN_SCALAR Time
column_create Posts n_goods COLUMN_SCALAR UInt32
column_create Posts published COLUMN_SCALAR Bool
column_create Posts rank COLUMN_SCALAR Int32
column_create Posts tag_text COLUMN_SCALAR ShortText
column_create Posts title COLUMN_SCALAR Text

table_create Tags TABLE_HASH_KEY ShortText --default_tokenizer TokenDelimit
column_create Tags name COLUMN_SCALAR Text

table_create Users TABLE_HASH_KEY ShortText
column_create Users name COLUMN_SCALAR Text
    SCHEMA
  end

  def dumped_schema_reference_columns
    <<-SCHEMA.chomp
column_create Posts author COLUMN_SCALAR Users
column_create Posts tags COLUMN_VECTOR Tags
    SCHEMA
  end

  def dumped_schema_index_columns
    <<-SCHEMA.chomp
column_create Tags Posts_tag_text COLUMN_INDEX Posts tag_text

column_create Users Posts_author COLUMN_INDEX Posts author
    SCHEMA
  end

  class EmptyTest < self
    def test_default
      assert_equal(dumped_schema, dump)
    end
  end

  class HaveDataTest < self
    setup
    def setup_data
      posts.add(:author => "mori",
                :created_at => Time.parse("2010-03-08 16:52 +0900"),
                :n_goods => 4,
                :published => true,
                :rank => 10,
                :tag_text => "search mori",
                :tags => ["search", "mori"],
                :title => "Why search engine find?")
    end

    def dumped_table_posts
      <<-TABLE.chomp
load --table Posts
[
["_id","author","created_at","n_goods","published","rank","tag_text","tags","title"],
[1,"mori",1268034720.0,4,true,10,"search mori",["search","mori"],"Why search engine find?"]
]
      TABLE
    end

    def dumped_table_tags
      <<-TABLE.chomp
load --table Tags
[
["_key","name"],
["search",""],
["mori",""]
]
      TABLE
    end

    def dumped_table_users
      <<-TABLE.chomp
load --table Users
[
["_key","name"],
["mori",""]
]
      TABLE
    end

    def dumped_tables
      <<-TABLES.chomp
#{dumped_table_posts}

#{dumped_table_tags}

#{dumped_table_users}
      TABLES
    end

    def test_default
      assert_equal(<<-DUMP, dump)
#{dumped_schema_tables}

#{dumped_schema_reference_columns}

#{dumped_tables}

#{dumped_schema_index_columns}
      DUMP
    end

    def test_limit_tables
      assert_equal(<<-DUMP, dump(:tables => ["Posts"]))
#{dumped_schema_tables}

#{dumped_schema_reference_columns}

#{dumped_table_posts}

#{dumped_schema_index_columns}
      DUMP
    end

    def test_limit_tables_with_regexp
      assert_equal(<<-DUMP, dump(:tables => [/Posts?/]))
#{dumped_schema_tables}

#{dumped_schema_reference_columns}

#{dumped_table_posts}

#{dumped_schema_index_columns}
      DUMP
    end

    def test_exclude_tables
      dump_options = {
        :exclude_tables => ["Posts"],
      }
      assert_equal(<<-DUMP, dump(dump_options))
#{dumped_schema_tables}

#{dumped_schema_reference_columns}

#{dumped_table_tags}

#{dumped_table_users}

#{dumped_schema_index_columns}
      DUMP
    end

    def test_exclude_tables_with_regexp
      dump_options = {
        :exclude_tables => [/Posts?/],
      }
      assert_equal(<<-DUMP, dump(dump_options))
#{dumped_schema_tables}

#{dumped_schema_reference_columns}

#{dumped_table_tags}

#{dumped_table_users}

#{dumped_schema_index_columns}
      DUMP
    end

    def test_tables_combination
      dump_options = {
        :exclude_tables => ["Posts"],
        :tables => ["Posts", "Users"],
      }
      assert_equal(<<-DUMP, dump(dump_options))
#{dumped_schema_tables}

#{dumped_schema_reference_columns}

#{dumped_table_users}

#{dumped_schema_index_columns}
      DUMP
    end

    def test_no_schema
      assert_equal(<<-DUMP, dump(:dump_schema => false))
#{dumped_tables}

#{dumped_schema_index_columns}
      DUMP
    end

    def test_no_schema_no_indexes
      assert_equal(<<-DUMP, dump(:dump_schema => false, :dump_indexes => false))
#{dumped_tables}
      DUMP
    end

    def test_no_tables
      assert_equal(dumped_schema, dump(:dump_tables => false))
    end
  end

  class PluginTest < self
    def test_standard_plugin
      Groonga::Plugin.register("suggest/suggest")
      assert_equal("register suggest/suggest\n" +
                   "\n" +
                   dumped_schema,
                   dump)
    end
  end

  class IndexTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Posts") do |table|
          table.text("title")
          table.short_text("tag_text")
        end

        schema.create_table("Tags",
                            :type => :hash,
                            :key_type => :short_text,
                            :default_tokenizer => :delimit) do |table|
          table.index("Posts.tag_text", :with_position => false)
        end
      end
    end

    setup
    def setup_data
      posts.add(:title => "Why search engine find?",
                :tag_text => "search mori")
    end

    def test_index_column_only
      assert_equal(<<-COMMAND, dump)
table_create Posts TABLE_NO_KEY
column_create Posts tag_text COLUMN_SCALAR ShortText
column_create Posts title COLUMN_SCALAR Text

table_create Tags TABLE_HASH_KEY ShortText --default_tokenizer TokenDelimit

load --table Posts
[
["_id","tag_text","title"],
[1,"search mori","Why search engine find?"]
]

column_create Tags Posts_tag_text COLUMN_INDEX Posts tag_text
COMMAND
    end
  end

  class NoColumnTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :patricia_trie,
                            :key_type => "ShortText") do |table|
        end
      end
    end

    setup
    def setup_data
      users.add("s-yata")
      users.add("mori")
    end

    def test_have_records
      assert_equal(<<-DUMP, dump)
table_create Users TABLE_PAT_KEY ShortText

load --table Users
[
[\"_key\"],
[\"mori\"],
[\"s-yata\"]
]
      DUMP
    end
  end
end
