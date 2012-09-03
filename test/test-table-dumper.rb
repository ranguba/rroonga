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

  class TextTest < self
    class ScalarTest < self
      def setup
        Groonga::Schema.define do |schema|
          schema.create_table("Users") do |table|
            table.text("name")
          end
        end
      end

      def test_empty
        assert_equal(<<-EOS, dump("Users"))
load --table Users
[
["_id","name"]
]
EOS
      end

      def test_with_records
        users.add(:name => "mori")
        assert_equal(<<-EOS, dump("Users"))
load --table Users
[
["_id","name"],
[1,"mori"]
]
EOS
      end
    end

    class VectorTest < self
      def setup
        Groonga::Schema.define do |schema|
          schema.create_table("Posts") do |table|
            table.text("tags", :type => :vector)
          end
        end
      end

      def test_empty
        assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","tags"]
]
EOS
      end

      def test_with_records
        posts.add(:tags => ["search", "mori"])
        assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","tags"],
[1,["search","mori"]]
]
EOS
      end
    end
  end

  class ReferenceTest < self
    def setup
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :patricia_trie,
                            :key_type => "ShortText") do |table|
          table.text("name")
        end

        schema.create_table("Posts") do |table|
          table.reference("author", "Users")
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
["_id","author"]
]
EOS
    end

    def test_with_records
      posts.add(:author => "mori")
      assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","author"],
[1,"mori"]
]
EOS
    end
  end

  class TimeTest < self
    def setup
      Groonga::Schema.define do |schema|
        schema.create_table("Posts") do |table|
          table.time("created_at")
        end
      end
    end

    def test_empty
      assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","created_at"]
]
EOS
    end

    def test_with_records
      posts.add(:created_at => Time.parse("2010-03-08 16:52 +0900"))
      assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","created_at"],
[1,1268034720.0]
]
EOS
    end
  end

  class IntegerTest < self
    def setup
      Groonga::Schema.define do |schema|
        schema.create_table("Posts") do |table|
          table.integer("rank")
        end
      end
    end

    def test_empty
      assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","rank"]
]
EOS
    end

    def test_with_records
      posts.add(:rank => 10)
      assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","rank"],
[1,10]
]
EOS
    end
  end

  class UnsignedIntegerTest < self
    def setup
      Groonga::Schema.define do |schema|
        schema.create_table("Posts") do |table|
          table.unsigned_integer("n_goods")
        end
      end
    end

    def test_empty
      assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","n_goods"]
]
EOS
    end

    def test_with_records
      posts.add(:n_goods => 4)
      assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","n_goods"],
[1,4]
]
EOS
    end
  end

  class WGS84GeoPointTest < self
    def setup
      Groonga::Schema.define do |schema|
        schema.create_table("Posts") do |table|
          table.wgs84_geo_point("location")
        end
      end
    end

    def test_empty
      assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","location"]
]
EOS
    end

    def test_with_records
      posts.add(:location => "40.689167x-74.044444")
      assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","location"],
[1,"146481001x-266559998"]
]
EOS
    end
  end

  class PatriciaTrieTest < self
    def setup
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :patricia_trie,
                            :key_type => "ShortText") do |table|
          table.text("name")
        end
      end
    end

    def test_order_by_default
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
  end
end
