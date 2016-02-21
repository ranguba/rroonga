# -*- coding: utf-8 -*-
#
# Copyright (C) 2011-2015  Kouhei Sutou <kou@clear-code.com>
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

  class TableTest < self
    class ArrayTest < self
      class OrderByTest < self
        def setup
          Groonga::Schema.define do |schema|
            schema.create_table("Users") do |table|
              table.text("name")
            end
          end

          @users = Groonga["Users"]
          @users.add(:name => "Chris")
          @users.add(:name => "Bob")
          @users.add(:name => "Alice")
        end

        def test_id
          assert_equal(<<-DUMP, dump("Users", :order_by => :id))
load --table Users
[
["_id","name"],
[1,"Chris"],
[2,"Bob"],
[3,"Alice"]
]
          DUMP
        end

        def test_key
          assert_equal(<<-DUMP, dump("Users", :order_by => :key))
load --table Users
[
["_id","name"],
[1,"Chris"],
[2,"Bob"],
[3,"Alice"]
]
          DUMP
        end
      end
    end

    class HashTest < self
      class OrderByTest < self
        def setup
          Groonga::Schema.define do |schema|
            schema.create_table("Users",
                                :type => :hash,
                                :key_type => "ShortText") do |table|
            end
          end

          @users = Groonga["Users"]
          @users.add("Chris")
          @users.add("Bob")
          @users.add("Alice")
        end

        def test_id
          assert_equal(<<-DUMP, dump("Users", :order_by => :id))
load --table Users
[
["_key"],
["Chris"],
["Bob"],
["Alice"]
]
          DUMP
        end

        def test_key
          assert_equal(<<-DUMP, dump("Users", :order_by => :key))
load --table Users
[
["_key"],
["Chris"],
["Bob"],
["Alice"]
]
          DUMP
        end
      end
    end

    class PatriciaTrieTest < self
      class OrderByTest < self
        def setup
          Groonga::Schema.define do |schema|
            schema.create_table("Users",
                                :type => :patricia_trie,
                                :key_type => "ShortText") do |table|
            end
          end

          @users = Groonga["Users"]
          @users.add("Chris")
          @users.add("Bob")
          @users.add("Alice")
        end

        def test_id
          assert_equal(<<-DUMP, dump("Users", :order_by => :id))
load --table Users
[
["_key"],
["Chris"],
["Bob"],
["Alice"]
]
          DUMP
        end

        def test_key
          assert_equal(<<-DUMP, dump("Users", :order_by => :key))
load --table Users
[
["_key"],
["Alice"],
["Bob"],
["Chris"]
]
          DUMP
        end
      end
    end

    class DoubleArrayTrieTest < self
      class OrderByTest < self
        def setup
          Groonga::Schema.define do |schema|
            schema.create_table("Users",
                                :type => :double_array_trie,
                                :key_type => "ShortText") do |table|
            end
          end

          @users = Groonga["Users"]
          @users.add("Chris")
          @users.add("Bob")
          @users.add("Alice")
        end

        def test_id
          assert_equal(<<-DUMP, dump("Users", :order_by => :id))
load --table Users
[
["_key"],
["Chris"],
["Bob"],
["Alice"]
]
          DUMP
        end

        def test_key
          assert_equal(<<-DUMP, dump("Users", :order_by => :key))
load --table Users
[
["_key"],
["Alice"],
["Bob"],
["Chris"]
]
          DUMP
        end
      end
    end
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

      def test_invalid_utf8
        need_encoding

        users.add(:name => "森\xff大二郎")
        error_output = StringIO.new
        assert_equal(<<-EOS, dump("Users", :error_output => error_output))
load --table Users
[
["_id","name"],
[1,"森大二郎"]
]
EOS
        assert_equal("warning: ignore invalid encoding character: " +
                       "<Users[1].name>: <0xff>: before: <森>\n",
                     error_output.string)
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

      def test_ascii
        posts.add(:tags => ["search", "mori"])
        assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","tags"],
[1,["search","mori"]]
]
EOS
      end

      def test_non_ascii
        posts.add(:tags => ["検索", "森"])
        assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","tags"],
[1,["検索","森"]]
]
EOS
      end
    end
  end

  class ReferenceTest < self
    class ScalarTest < self
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

      def test_ascii
        posts.add(:author => "mori")
        assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","author"],
[1,"mori"]
]
EOS
      end

      def test_non_ascii
        posts.add(:author => "森")
        assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","author"],
[1,"森"]
]
EOS
      end
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

  class TokyoGeoPointTest < self
    def setup
      Groonga::Schema.define do |schema|
        schema.create_table("Posts") do |table|
          table.tokyo_geo_point("location")
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
      posts.add(:location => "146481001x-266559998")
      assert_equal(<<-EOS, dump("Posts"))
load --table Posts
[
["_id","location"],
[1,"146481001x-266559998"]
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
["_key","name"],
["mori","mori daijiro"],
["s-yata","Susumu Yata"]
]
EOS
    end
  end

  class WeightVectorIndexTest < self
    def setup
      Groonga::Schema.define do |schema|
        schema.create_table("Products",
                            :type => :patricia_trie,
                            :key_type => "ShortText") do |table|
          table.short_text("tags",
                           :type => :vector,
                           :with_weight => true)
        end
      end
    end

    def products
      Groonga["Products"]
    end

    def test_weight
      products.add("Groonga", :tags => [
                     {
                       :value  => "groonga",
                       :weight => 100,
                     },
                   ])
      products.add("Mroonga", :tags => [
                     {
                       :value  => "mroonga",
                       :weight => 100,
                     },
                     {
                       :value  => "groonga",
                       :weight => 10,
                     },
                   ])
      assert_equal(<<-COMMAND, dump("Products"))
load --table Products
[
["_key","tags"],
["Groonga",{"groonga":100}],
["Mroonga",{"groonga":10,"mroonga":100}]
]
      COMMAND
    end
  end

  class MaxRecordsTest < self
    def setup
      Groonga::Schema.define do |schema|
        schema.create_table("Products",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
        end
      end
    end

    def products
      Groonga["Products"]
    end

    def test_small
      products.add("Groonga")
      products.add("Mroonga")
      products.add("Rroonga")
      assert_equal(<<-COMMAND, dump("Products", :max_records => 2))
load --table Products
[
["_key"],
["Groonga"],
["Mroonga"]
]
      COMMAND
    end
  end
end
