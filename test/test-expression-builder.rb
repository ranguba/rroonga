# -*- coding: utf-8 -*-
#
# Copyright (C) 2014-2015  Masafumi Yokoyama <yokoyama@clear-code.com>
# Copyright (C) 2009-2016  Kouhei Sutou <kou@clear-code.com>
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

class ExpressionBuilderTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database
  setup :setup_tables
  setup :setup_data

  def setup_tables
  end

  def setup_data
  end

  class EqualityTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
          table.short_text("name")
        end
      end

      @users = Groonga["Users"]
    end

    def define_users_name_index
      Groonga::Schema.define do |schema|
        schema.create_table("Terms",
                            :type => :patricia_trie,
                            :default_tokenizer => "TokenBigram",
                            :key_type => "ShortText") do |table|
          table.index("Users.name")
        end
      end
    end

    def setup_data
      @users.add("morita",      :name => "mori daijiro")
      @users.add("gunyara-kun", :name => "Tasuku SUENAGA")
      @users.add("yu",          :name => "Yutaro Shimamura")
    end

    class EqualTest < self
      def test_without_index
        result = @users.select do |record|
          record["name"] == "mori daijiro"
        end
        assert_equal(["morita"],
                     result.collect {|record| record.key.key})
      end

      def test_with_index
        define_users_name_index
        result = @users.select do |record|
          record["name"] == "mori daijiro"
        end
        assert_equal(["morita"],
                     result.collect {|record| record.key.key})
      end
    end

    class NotEqualTest < self
      def test_without_index
        result = @users.select do |record|
          record["name"] != "mori daijiro"
        end
        assert_equal(["gunyara-kun", "yu"],
                     result.collect {|record| record.key.key})
      end

      def test_with_index
        define_users_name_index
        result = @users.select do |record|
          record["name"] != "mori daijiro"
        end
        assert_equal(["gunyara-kun", "yu"],
                     result.collect {|record| record.key.key})
      end
    end
  end

  class RelationTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
          table.uint32("hp")
        end
      end

      @users = Groonga["Users"]
    end

    def setup_data
      @users.add("morita",      :hp => 100)
      @users.add("gunyara-kun", :hp => 150)
      @users.add("yu",          :hp => 200)
    end

    def test_less
      result = @users.select do |record|
        record["hp"] < 150
      end
      assert_equal(["morita"], result.collect {|record| record.key.key})
    end

    def test_less_equal
      result = @users.select do |record|
        record["hp"] <= 150
      end
      assert_equal(["morita", "gunyara-kun"],
                   result.collect {|record| record.key.key})
    end

    def test_greater
      result = @users.select do |record|
        record["hp"] > 150
      end
      assert_equal(["yu"], result.collect {|record| record.key.key})
    end

    def test_greater_equal
      result = @users.select do |record|
        record["hp"] >= 150
      end
      assert_equal(["gunyara-kun", "yu"],
                   result.collect {|record| record.key.key})
    end
  end

  class LogicalTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
          table.uint32("hp")
        end
      end

      @users = Groonga["Users"]
    end

    def setup_data
      @users.add("morita",      :hp => 100)
      @users.add("gunyara-kun", :hp => 150)
      @users.add("yu",          :hp => 200)
    end

    def test_and
      result = @users.select do |record|
        (record["hp"] > 100) & (record["hp"] <= 200)
      end
      assert_equal(["gunyara-kun", "yu"],
                   result.collect {|record| record.key.key})
    end

    def test_and_array
      result = @users.select do |record|
        conditions = []
        conditions << (record.hp > 100)
        conditions << (record.hp <= 200)
        conditions
      end
      assert_equal(["gunyara-kun", "yu"],
                   result.collect {|record| record.key.key})
    end

    def test_and_array_nested
      result = @users.select do |record|
        conditions = []
        conditions << [(record.hp > 100)]
        conditions << []
        conditions << [[[(record.hp < 200)]]]
        conditions << [[]]
        conditions
      end
      assert_equal(["gunyara-kun"],
                   result.collect {|record| record.key.key})
    end

    def test_or
      result = @users.select do |record|
        (record["hp"] == 150) | (record["hp"] > 150)
      end
      assert_equal(["gunyara-kun", "yu"],
                   result.collect {|record| record.key.key})
    end

    def test_and_not
      result = @users.select do |record|
        (record["hp"] > 100) - (record["hp"] == 150)
      end
      assert_equal(["yu"],
                   result.collect {|record| record.key.key})
    end
  end

  class FullTextSearchTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
          table.short_text("name")
        end

        schema.create_table("Terms",
                            :type => :patricia_trie,
                            :default_tokenizer => "TokenBigram",
                            :key_type => "ShortText") do |table|
          table.index("Users.name")
        end
      end

      @users = Groonga["Users"]
    end

    def setup_data
      @users.add("morita",      :name => "mori daijiro")
      @users.add("gunyara-kun", :name => "Tasuku SUENAGA")
      @users.add("yu",          :name => "Yutaro Shimamura")
    end

    def test_match
      result = @users.select do |record|
        record["name"] =~ "ro"
      end
      assert_equal(["morita", "yu"],
                   result.collect {|record| record.key.key})
    end

    def test_nil_match
      @users.select do |record|
        exception = ArgumentError.new("match word should not be nil: Users.name")
        assert_raise(exception) do
          record["name"] =~ nil
        end
        record["name"] == "dummy"
      end
    end

    def test_query_string
      result = @users.select("name:@ro")
      assert_equal(["morita", "yu"],
                   result.collect {|record| record.key.key})
    end
  end

  class RegexpSearchTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
          table.short_text("name")
        end

        schema.create_table("Terms",
                            :type => :patricia_trie,
                            :key_type => "ShortText",
                            :default_tokenizer => "TokenRegexp",
                            :normalizer => "NormalizerAuto") do |table|
          table.index("Users.name")
        end
      end

      @users = Groonga["Users"]
    end

    def setup_data
      @users.add("sato",   :name => "kazuki sato")
      @users.add("suzuki", :name => "Shiro SUZUKI")
      @users.add("ito",    :name => "Takashi Ito")
    end

    class BlockTest < self
      def test_match
        result = @users.select do |record|
          record["name"] =~ /sh/
        end
        assert_equal(["ito", "suzuki"],
                     result.collect {|record| record.key.key}.sort)
      end

      def test_not_match
        result = @users.select do |record|
          record["name"] =~ /abcabcabc/
        end
        assert_equal([],
                     result.collect {|record| record.key.key}.sort)
      end

      def test_beginning_of_text
        result = @users.select do |record|
          record["name"] =~ /\Ash/
        end
        assert_equal(["suzuki"],
                     result.collect {|record| record.key.key}.sort)
      end

      def test_end_of_text
        result = @users.select do |record|
          record["name"] =~ /ki\z/
        end
        assert_equal(["suzuki"],
                     result.collect {|record| record.key.key}.sort)
      end

      def test_option
        result = @users.select do |record|
          record["name"] =~ /Su/i
        end
        assert_equal(["suzuki"],
                     result.collect {|record| record.key.key}.sort)
      end
    end

    class QueryStringTest < self
      def test_match
        result = @users.select("name:~t")
        assert_equal(["ito", "sato"],
                     result.collect {|record| record.key.key}.sort)
      end

      def test_not_match
        result = @users.select("name:~x")
        assert_equal([],
                     result.collect {|record| record.key.key}.sort)
      end

      def test_beginning_of_text
        result = @users.select("name:~\\\\As")
        assert_equal(["suzuki"],
                     result.collect {|record| record.key.key}.sort)
      end
    end
  end

  class PrefixSearchTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Sections",
                            :type => :patricia_trie,
                            :key_type => "ShortText") do |table|
        end
      end

      @sections = Groonga["Sections"]
    end

    def setup_data
      @sections.add("search/core")
      @sections.add("suggest/all")
      @sections.add("search/all")
    end

    def test_match
      result = @sections.select do |record|
        record.key.prefix_search("search")
      end
      assert_equal(["search/all", "search/core"].sort,
                   result.collect {|record| record.key.key}.sort)
    end
  end

  class SuffixSearchTest < self
    def test_patricia_trie_use_index
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :patricia_trie,
                            :key_with_sis => true,
                            :key_type => "ShortText") do |table|
        end
      end

      users = Groonga["Users"]
      users.add("ひろゆき")
      users.add("まさゆき")
      users.add("ゆきひろ")

      result = users.select do |record|
        record.key.suffix_search("ゆき")
      end
      assert_equal(["ひろゆき", "まさゆき", "ろゆき", "さゆき", "ゆき"].sort,
                   result.collect {|record| record.key.key}.sort)
    end

    def test_patricia_trie_not_use_index
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :patricia_trie,
                            :key_type => "ShortText") do |table|
        end
      end

      users = Groonga["Users"]
      users.add("ひろゆき")
      users.add("まさゆき")
      users.add("ゆきひろ")

      result = users.select do |record|
        record.key.suffix_search("ゆき")
      end
      assert_equal(["ひろゆき", "まさゆき"].sort,
                   result.collect {|record| record.key.key}.sort)
    end

    def test_column
      Groonga::Schema.define do |schema|
        schema.create_table("Users") do |table|
          table.short_text(:name)
        end
      end

      users = Groonga["Users"]
      users.add(:name => "ひろゆき")
      users.add(:name => "まさゆき")
      users.add(:name => "ゆきひろ")

      result = users.select do |record|
        record.name.suffix_search("ゆき")
      end
      assert_equal(["ひろゆき", "まさゆき"].sort,
                   result.collect {|record| record.key.name}.sort)
    end
  end

  class SimilarSearchTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Documents",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
          table.text("content")
        end

        schema.create_table("Terms",
                            :type => :patricia_trie,
                            :key_type => "ShortText",
                            :default_tokenizer => "TokenBigram",
                            :key_normalize => true) do |table|
          table.index("Documents.content")
        end
      end

      @documents = Groonga["Documents"]
    end

    def setup_data
      @documents.add("Groonga overview", :content => <<-EOC)
Groonga is a fast and accurate full text search engine based on
inverted index. One of the characteristics of groonga is that a newly
registered document instantly appears in search results. Also, groonga
allows updates without read locks. These characteristics result in
superior performance on real-time applications.
EOC
      @documents.add("Full text search and Instant update", :content => <<-EOC)
In widely used DBMSs, updates are immediately processed, for example,
a newly registered record appears in the result of the next query. In
contrast, some full text search engines do not support instant
updates, because it is difficult to dynamically update inverted
indexes, the underlying data structure.
EOC
      @documents.add("Column store and aggregate query", :content => <<-EOC)
People can collect more than enough data in the Internet era. However,
it is difficult to extract informative knowledge from a large
database, and such a task requires a many-sided analysis through trial
and error. For example, search refinement by date, time and location
may reveal hidden patterns. Aggregate queries are useful to perform
this kind of tasks.
EOC
    end

    def test_table
      result = @documents.select do |record|
        record.content.similar_search("fast full text search real time")
      end
      assert_equal(["Groonga overview"],
                   result.collect {|record| record.key.key}.sort)
    end

    def test_column
      result = @documents.column("content").select do |content|
        content.similar_search("fast full text search real time")
      end
      assert_equal(["Groonga overview"],
                   result.collect {|record| record.key.key}.sort)
    end
  end

  class TermExtractTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Words",
                            :type => :patricia_trie,
                            :key_type => "ShortText",
                            :default_tokenizer => "TokenBigram",
                            :key_normalize => true) do |table|
          table.text("content")
        end
      end

      @words = Groonga["Words"]
    end

    def setup_data
      @words.add("groonga")
      @words.add("mroonga")
      @words.add("Senna")
      @words.add("Tritonn")
    end

    def test_table
      result = @words.select do |record|
        record.key.term_extract("Groonga is the successor project to Senna.")
      end
      assert_equal(["groonga", "senna"].sort,
                   result.collect {|record| record.key.key}.sort)
    end

    def test_not_key_column
      message = "term extraction supports _key column only: <content>"
      exception = ArgumentError.new(message)
      assert_raise(exception) do
        @words.select do |record|
          record.content.term_extract("Groonga is the successor project to Senna.")
        end
      end
    end
  end

  class CallTest < self
    class StringTest < self
      def setup_tables
        Groonga::Schema.define do |schema|
          schema.create_table("Shops",
                              :type => :hash,
                              :key_type => "ShortText") do |table|
            table.wgs84_geo_point("location")
          end

          schema.create_table("Locations",
                              :type => :patricia_trie,
                              :key_type => :wgs84_geo_point) do |table|
            table.index("Shops.location")
          end
        end

        @shops = Groonga["Shops"]
      end

      def setup_data
        @shops.add("Nezu no taiyaki", :location => "35.720253,139.762573")
        @shops.add("Taiyaki Kataoka", :location => "35.712521,139.715591")
        @shops.add("Taiyaki Sharaku", :location => "35.716969,139.794846")
      end

      def test_call_style
        result = @shops.select do |record|
          record.call("geo_in_rectangle",
                      record.location,
                      "35.7185,139.7912",
                      "35.7065,139.8069")
        end
        assert_equal(["Taiyaki Sharaku"],
                     result.collect(&:_key))
      end

      def test_method_style
        result = @shops.select do |record|
          record.location.geo_in_rectangle("35.7185,139.7912",
                                           "35.7065,139.8069")
        end
        assert_equal(["Taiyaki Sharaku"],
                     result.collect(&:_key))
      end
    end

    class IntegerTest < self
      def setup_tables
        Groonga::Schema.define do |schema|
          schema.create_table("Users",
                              :type => :hash,
                              :key_type => "ShortText") do |table|
            table.int32("age")
          end

          schema.create_table("Ages",
                              :type => :patricia_trie,
                              :key_type => :int32) do |table|
            table.index("Users.age")
          end
        end

        @users = Groonga["Users"]
      end

      def setup_data
        @users.add("Alice",  :age => 18)
        @users.add("Bob",    :age => 29)
        @users.add("Carlos", :age => 14)
      end

      def test_search
        result = @users.select do |record|
          record.call("between",
                      record.age,
                      18,
                      "include",
                      29,
                      "exclude")
        end
        assert_equal(["Alice"],
                     result.collect(&:_key))
      end
    end

    class TimeTest < self
      def setup_tables
        Groonga::Schema.define do |schema|
          schema.create_table("Logs",
                              :type => :array) do |table|
            table.time("timestamp")
          end

          schema.create_table("Times",
                              :type => :patricia_trie,
                              :key_type => :time) do |table|
            table.index("Logs.timestamp")
          end
        end

        @logs = Groonga["Logs"]
      end

      def setup_data
        @logs.add(:timestamp => Time.iso8601("2016-02-21T19:00:01Z"))
        @logs.add(:timestamp => Time.iso8601("2016-02-21T19:00:02Z"))
        @logs.add(:timestamp => Time.iso8601("2016-02-21T19:00:03Z"))
      end

      def test_search
        result = @logs.select do |record|
          record.call("between",
                      record.timestamp,
                      Time.iso8601("2016-02-21T19:00:01Z"),
                      "include",
                      Time.iso8601("2016-02-21T19:00:03Z"),
                      "exclude")
        end
        assert_equal([
                       Time.iso8601("2016-02-21T19:00:01Z"),
                       Time.iso8601("2016-02-21T19:00:02Z"),
                     ],
                     result.collect(&:timestamp))
      end
    end

    class HashTest < self
      def setup_tables
        Groonga::Schema.define do |schema|
          schema.create_table("Tags",
                              :type => :patricia_trie,
                              :key_type => :short_text) do |table|
          end
        end

        @tags = Groonga["Tags"]
      end

      def setup_data
        @tags.add("Tom")
        @tags.add("Tomy")
        @tags.add("Ken")
      end

      def test_search
        result = @tags.select do |record|
          record.key.fuzzy_search("Toym", :with_transposition => true)
        end
        sorted_result = result.sort(["_score", "_key"])
        assert_equal([
                       ["Tom", 1.0],
                       ["Tomy", 1.0],
                     ],
                     sorted_result.collect {|r| [r._key, r.score]})
      end
    end
  end

  class RecordTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
        end

        schema.create_table("Bookmarks") do |table|
          table.reference("user", "Users")
          table.short_text("uri")
        end
      end

      @users     = Groonga["Users"]
      @bookmarks = Groonga["Bookmarks"]
    end

    def setup_data
      @morita      = @users.add("morita")
      @gunyara_kun = @users.add("gunyara-kun")

      @bookmarks.add(:user => @morita, :uri => "http://groonga.org/")
      @bookmarks.add(:user => @morita, :uri => "http://ruby-lang.org/")
      @bookmarks.add(:user => @gunyara_kun,
                     :uri => "http://dic.nicovideo.jp/")
    end

    def test_object
      result = @bookmarks.select do |record|
        record["user"] == @morita
      end
      assert_equal(["http://groonga.org/", "http://ruby-lang.org/"],
                   result.collect {|record| record.key["uri"]})
    end

    def test_key
      result = @bookmarks.select do |record|
        record["user"] == @morita.key
      end
      assert_equal(["http://groonga.org/", "http://ruby-lang.org/"],
                   result.collect {|record| record.key["uri"]})
    end

    def test_record_like_object
      morita = Object.new
      morita_singleton_class = (class << morita; self; end)
      morita_key = @morita.key
      morita_singleton_class.send(:define_method, :record_id) do
        morita_key
      end
      result = @bookmarks.select do |record|
        record["user"] == morita
      end
      assert_equal(["http://groonga.org/", "http://ruby-lang.org/"],
                   result.collect {|record| record.key["uri"]})
    end

    def test_record_like_object_raw
      morita = Object.new
      morita_singleton_class = (class << morita; self; end)
      morita_id = @morita.id
      morita_singleton_class.send(:define_method, :record_raw_id) do
        morita_id
      end
      result = @bookmarks.select do |record|
        record["user"] == morita
      end
      assert_equal(["http://groonga.org/", "http://ruby-lang.org/"],
                   result.collect {|record| record.key["uri"]})
    end
  end

  class AccessorTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Sections",
                            :type => :patricia_trie,
                            :key_type => "ShortText") do |table|
        end

        schema.create_table("Users",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
          table.short_text("name")
        end

        schema.create_table("Bookmarks") do |table|
          table.reference("user", "Users")
          table.short_text("uri")
        end
      end

      @users = Groonga["Users"]
      @bookmarks = Groonga["Bookmarks"]
    end

    def setup_data
      @morita      = @users.add("morita",      :name => "mori daijiro")
      @gunyara_kun = @users.add("gunyara-kun", :name => "Tasuku SUENAGA")

      @bookmarks.add(:user => @morita,       :uri => "http://groonga.org/")
      @bookmarks.add(:user => @morita,       :uri => "http://ruby-lang.org/")
      @bookmarks.add(:user => @gunyara_kun , :uri => "http://dic.nicovideo.jp/")
    end

    def test_nested_column
      result = @bookmarks.select do |record|
        record["user.name"] == @morita["name"]
      end
      assert_equal(["http://groonga.org/", "http://ruby-lang.org/"],
                   result.collect {|record| record["uri"]})
    end

    def test_method_chain
      result = @bookmarks.select do |record|
        record.user.name == @morita["name"]
      end
      assert_equal(["http://groonga.org/", "http://ruby-lang.org/"],
                   result.collect {|record| record["uri"]})
    end

    def test_deep_method_chain
      Groonga::Schema.define do |schema|
        schema.create_table("Pets",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
          table.short_text("name")
        end

        schema.change_table("Users") do |table|
          table.reference("pet", "Pets")
        end
      end

      pets = Groonga["Pets"]
      pets.add("bob", :name => "morita Bob")
      @morita["pet"] = "bob"

      result = @bookmarks.select do |record|
        record.user.pet.name == "morita Bob"
      end
      assert_equal(["http://groonga.org/", "http://ruby-lang.org/"],
                   result.collect {|record| record["uri"]})
    end
  end

  class PseudoColumnTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Users",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
          table.short_text("name")
        end

        schema.create_table("Terms",
                            :type => :patricia_trie,
                            :default_tokenizer => "TokenBigram",
                            :key_type => "ShortText") do |table|
          table.index("Users.name")
        end
      end

      @users = Groonga["Users"]
    end

    def setup_data
      @users.add("morita",      :name => "mori daijiro")
      @users.add("gunyara-kun", :name => "Tasuku SUENAGA")
      @users.add("yu",          :name => "Yutaro Shimamura")
    end

    def test_id
      result = @users.select do |record|
        record.id == 1
      end
      assert_equal(["morita"],
                   result.collect {|record| record.key.key})
    end

    def test_key
      result = @users.select do |record|
        record.key == "morita"
      end
      assert_equal(["morita"],
                   result.collect {|record| record.key.key})
    end

    def test_score
      result = @users.select do |record|
        (record.name =~ "mori") |
          (record.name =~ "daijiro") |
          (record.name =~ "Tasuku")
      end
      result = result.select do |record|
        record.score == 2
      end
      assert_equal([["morita", 2]],
                   result.collect {|record| [record["_key"], record.key.score]})
    end
  end
end
