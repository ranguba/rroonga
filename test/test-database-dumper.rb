# Copyright (C) 2011  Kouhei Sutou <kou@clear-code.com>
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

class DatabaseDumperTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database, :before => :append

  setup
  def setup_tables
    Groonga::Schema.define do |schema|
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
        table.text("tags", :type => :vector)
        table.boolean("published")
        table.time("created_at")
      end

      schema.change_table("Users") do |table|
        table.index("Posts.author")
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

  class EmptyTest < DatabaseDumperTest
    def test_default
      assert_equal(<<-EOS, dump)
table_create Posts TABLE_NO_KEY
column_create Posts created_at COLUMN_SCALAR Time
column_create Posts n_goods COLUMN_SCALAR UInt32
column_create Posts published COLUMN_SCALAR Bool
column_create Posts rank COLUMN_SCALAR Int32
column_create Posts tags COLUMN_VECTOR Text
column_create Posts title COLUMN_SCALAR Text

table_create Users TABLE_HASH_KEY --key_type ShortText
column_create Users name COLUMN_SCALAR Text

column_create Posts author COLUMN_SCALAR Users

column_create Users Posts_author COLUMN_INDEX Posts author
EOS
    end
  end

  class HaveDataTest < DatabaseDumperTest
    setup
    def setup_data
      posts.add(:author => "mori",
                :created_at => Time.parse("2010-03-08 16:52 JST"),
                :n_goods => 4,
                :published => true,
                :rank => 10,
                :tags => ["search", "mori"],
                :title => "Why search engine find?")
    end

    def test_default
      assert_equal(<<-EOS, dump)
table_create Posts TABLE_NO_KEY
column_create Posts created_at COLUMN_SCALAR Time
column_create Posts n_goods COLUMN_SCALAR UInt32
column_create Posts published COLUMN_SCALAR Bool
column_create Posts rank COLUMN_SCALAR Int32
column_create Posts tags COLUMN_VECTOR Text
column_create Posts title COLUMN_SCALAR Text

table_create Users TABLE_HASH_KEY --key_type ShortText
column_create Users name COLUMN_SCALAR Text

column_create Posts author COLUMN_SCALAR Users

column_create Users Posts_author COLUMN_INDEX Posts author

load --table Posts
[
["_id","author","created_at","n_goods","published","rank","tags","title"],
[1,"mori",1268034720.0,4,true,10,["search","mori"],"Why search engine find?"]
]

load --table Users
[
["_key","name"],
["mori",""]
]
EOS
    end

    def test_limit_tables
      assert_equal(<<-EOS, dump(:tables => ["Posts"]))
table_create Posts TABLE_NO_KEY
column_create Posts created_at COLUMN_SCALAR Time
column_create Posts n_goods COLUMN_SCALAR UInt32
column_create Posts published COLUMN_SCALAR Bool
column_create Posts rank COLUMN_SCALAR Int32
column_create Posts tags COLUMN_VECTOR Text
column_create Posts title COLUMN_SCALAR Text

table_create Users TABLE_HASH_KEY --key_type ShortText
column_create Users name COLUMN_SCALAR Text

column_create Posts author COLUMN_SCALAR Users

column_create Users Posts_author COLUMN_INDEX Posts author

load --table Posts
[
["_id","author","created_at","n_goods","published","rank","tags","title"],
[1,"mori",1268034720.0,4,true,10,["search","mori"],"Why search engine find?"]
]
EOS
    end

    def test_no_schema
      assert_equal(<<-EOS, dump(:dump_schema => false))
load --table Posts
[
["_id","author","created_at","n_goods","published","rank","tags","title"],
[1,"mori",1268034720.0,4,true,10,["search","mori"],"Why search engine find?"]
]

load --table Users
[
["_key","name"],
["mori",""]
]
EOS
    end
  end
end
