# Copyright (C) 2011-2012  Kouhei Sutou <kou@clear-code.com>
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

class TableDumperTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database, :before => :append

  def setup
    setup_tables
  end

  def setup_tables
    Groonga::Schema.define do |schema|
      schema.create_table("Users",
                          :type => :patricia_trie,
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

  def test_empty
    assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","author","created_at","n_goods","published","rank","tags","title"]
]
EOS
  end

  def test_with_records
    posts.add(:author => "mori",
              :created_at => Time.parse("2010-03-08 16:52 JST"),
              :n_goods => 4,
              :published => true,
              :rank => 10,
              :tags => ["search", "mori"],
              :title => "Why search engine find?")
    assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","author","created_at","n_goods","published","rank","tags","title"],
[1,"mori",1268034720.0,4,true,10,["search","mori"],"Why search engine find?"]
]
EOS
  end

  def test_patricia_trie_order_by_default
    users.add("s-yata", :name => "Susumu Yata")
    users.add("mori", :name => "mori daijiro")
    assert_equal(<<-EOS, dump("Users"))
load --table Users
[
[\"_key\",\"name\"],
[\"mori\",\"mori daijiro\"],
[\"s-yata\",\"Susumu Yata\"]
]
EOS
  end

  def test_patricia_trie_order_by_id
    users.add("s-yata", :name => "Susumu Yata")
    users.add("mori", :name => "mori daijiro")
    assert_equal(<<-EOS, dump("Users", :order_by => "id"))
load --table Users
[
[\"_key\",\"name\"],
[\"s-yata\",\"Susumu Yata\"],
[\"mori\",\"mori daijiro\"]
]
EOS
  end

  private
  def dump(table_name, options={})
    Groonga::TableDumper.new(context[table_name], options).dump
  end

  def users
    context["Users"]
  end

  def posts
    context["Posts"]
  end
end
