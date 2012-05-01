# Copyright (C) 2009-2012  Kouhei Sutou <kou@clear-code.com>
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

class ExpressionBuilderTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database
  setup :setup_tables
  setup :setup_data

  def setup_tables
    Groonga::Schema.define do |schema|
      schema.create_table("Pets",
                          :type => :hash,
                          :key_type => "ShortText") do |table|
        table.short_text("name")
      end

      schema.create_table("Sections",
                          :type => :patricia_trie,
                          :key_type => "ShortText") do |table|
      end

      schema.create_table("Users",
                          :type => :hash,
                          :key_type => "ShortText") do |table|
        table.short_text("name")
        table.uint32("hp")
        table.reference("pet", "Pets")
        table.reference("section", "Sections")
      end

      schema.create_table("Bookmarks") do |table|
        table.reference("user", "Users")
        table.short_text("uri")
      end

      schema.change_table("Sections") do |table|
        table.index("Users.section")
      end
    end

    define_users_name_index

    @pets = Groonga["Pets"]
    @users = Groonga["Users"]
    @bookmarks = Groonga["Bookmarks"]
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
    @morita = @users.add("morita",
                         :name => "mori daijiro",
                         :hp => 100,
                         :section => "search/core")
    @gunyara_kun = @users.add("gunyara-kun",
                              :name => "Tasuku SUENAGA",
                              :hp => 150,
                              :section => "suggest/all")
    @yu = @users.add("yu",
                     :name => "Yutaro Shimamura",
                     :hp => 200,
                     :section => "search/all")

    @groonga = @bookmarks.add(:user => @morita, :uri => "http://groonga.org/")
    @ruby = @bookmarks.add(:user => @morita, :uri => "http://ruby-lang.org/")
    @nico_dict = @bookmarks.add(:user => @gunyara_kun,
                                :uri => "http://dic.nicovideo.jp/")
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

    def setup_data
      @users.add("morita",      :name => "mori daijiro")
      @users.add("gunyara-kun", :name => "Tasuku SUENAGA")
      @users.add("yu",          :name => "Yutaro Shimamura")
    end

    class EqualTest < self
      def test_equal_without_index
        result = @users.select do |record|
          record["name"] == "mori daijiro"
        end
        assert_equal(["morita"],
                     result.collect {|record| record.key.key})
      end

      def test_equal_with_index
        define_users_name_index
        result = @users.select do |record|
          record["name"] == "mori daijiro"
        end
        assert_equal(["morita"],
                     result.collect {|record| record.key.key})
      end
    end

    class NotEqualTest < self
      setup :only_ruby19

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

    def test_or
      result = @users.select do |record|
        (record["hp"] == 150) | (record["hp"] > 150)
      end
      assert_equal(["gunyara-kun", "yu"],
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

  class XfixSearchTest < self
    def setup_tables
    end

    def setup_data
    end

    def test_prefix_saerch
      Groonga::Schema.define do |schema|
        schema.create_table("Sections",
                            :type => :patricia_trie,
                            :key_type => "ShortText") do |table|
        end
      end

      sections = Groonga["Sections"]
      sections.add("search/core")
      sections.add("suggest/all")
      sections.add("search/all")
      result = sections.select do |record|
        record.key.prefix_search("search")
      end
      assert_equal(["search/all", "search/core"].sort,
                   result.collect {|record| record.key.key}.sort)
    end

    def test_suffix_search
      Groonga::Schema.define do |schema|
        schema.create_table("Sections",
                            :type => :patricia_trie,
                            :key_with_sis => true,
                            :key_type => "ShortText") do |table|
        end
      end

      sections = Groonga["Sections"]
      sections.add("search/core")
      sections.add("suggest/all")
      sections.add("search/all")

      result = sections.select do |record|
        record.key.suffix_search("all")
      end
      assert_equal(["suggest/all", "search/all"].sort,
                   result.collect {|record| record.key.key}.sort)
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

    def test_id
      result = @bookmarks.select do |record|
        record["user"] == @morita.id
      end
      assert_equal(["http://groonga.org/", "http://ruby-lang.org/"],
                   result.collect {|record| record.key["uri"]})
    end

    def test_record_like_object
      morita = Object.new
      morita_singleton_class = (class << morita; self; end)
      morita_id = @morita.id
      morita_singleton_class.send(:define_method, :record_id) do
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

    def test_n_sub_records
      result = @users.select do |record|
        (record.name =~ "o") | (record.key == "yu")
      end
      result = result.select do |record|
        record.n_sub_records > 1
      end
      matched_records = result.collect do |record|
        [record["_key"], record.key.n_sub_records]
      end
      assert_equal([["yu", 2]], matched_records)
    end
  end
end
