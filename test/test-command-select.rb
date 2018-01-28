# Copyright (C) 2010-2018  Kouhei Sutou <kou@clear-code.com>
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

class CommandSelectTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database
  setup :setup_tables
  setup :setup_data

  def setup_data
    Groonga::Schema.define do |schema|
      schema.create_table("Books",
                          :type => :hash, :key_type => "ShortText") do |table|
        table.time("published")
      end

      schema.create_table("Users",
                          :type => :hash, :key_type => "ShortText") do |table|
        table.reference("book", "Books")
      end

      schema.create_table("Bigram",
                          :type => :patricia_trie,
                          :key_type => "ShortText",
                          :default_tokenizer => "TokenBigramSplitSymbolAlpha") do |table|
        table.index("Books._key")
      end
    end

    @books = Groonga["Books"]
    @users = Groonga["Users"]

    @books.add("the groonga book", :published => Time.parse("2010/04/01"))
    @books.add("the groonga book (2)", :published => Time.parse("2011/04/01"))

    @users.add("morita", :book => "the groonga book")
    @users.add("gunyara-kun", :book => "the groonga book")
    @users.add("yu")
    @users.add("ryoqun", :book => "the groonga book (2)")
  end

  def test_no_option
    result = context.select(@users)
    assert_equal([4,
                  [{"_id" => 1, "_key" => "morita",
                     "book" => "the groonga book"},
                   {"_id" => 2, "_key" => "gunyara-kun",
                     "book" => "the groonga book"},
                   {"_id" => 3, "_key" => "yu",
                     "book" => ""},
                   {"_id" => 4, "_key" => "ryoqun",
                     "book" => "the groonga book (2)"}]],
                 [result.n_hits, result.records])
  end

  def test_output_columns
    result = context.select(@users, :output_columns => ["_key"])
    assert_equal([4,
                  [{"_key" => "morita"},
                   {"_key" => "gunyara-kun"},
                   {"_key" => "yu"},
                   {"_key" => "ryoqun"}]],
                 [result.n_hits, result.records])
  end

  def test_drilldowns
    result = context.select(@users,
                            :output_columns => ["_key"],
                            :drill_down => ["_key", "book"],
                            :drill_down_output_columns => "_key",
                            :drill_down_limit => 10)
    drilldowns = normalize_drilldowns(result.drilldowns)
    assert_equal({
                    "_key" => [4, [{"_key" => "morita"},
                                   {"_key" => "gunyara-kun"},
                                   {"_key" => "yu"},
                                   {"_key" => "ryoqun"}]],
                    "book" => [2, [{"_key" => "the groonga book"},
                                   {"_key" => "the groonga book (2)"}]],
                 },
                 drilldowns)
  end

  def test_drilldowns_with_no_hit
    result = context.select(@users,
                            :filter => "_key == \"no\\ hit\"",
                            :output_columns => ["_key"],
                            :drill_down => ["_key", "book"],
                            :drill_down_output_columns => "_key",
                            :drill_down_limit => 10)
    drilldowns = normalize_drilldowns(result.drilldowns)
    assert_equal({
                   "_key" => [0, []],
                   "book" => [0, []],
                 },
                 drilldowns)
  end

  def test_time
    result = context.select(@books)
    assert_equal([{
                    "_id" => 1,
                    "_key" => "the groonga book",
                    "published" => Time.parse("2010/04/01"),
                  },
                  {
                    "_id" => 2,
                    "_key" => "the groonga book (2)",
                    "published" => Time.parse("2011/04/01"),
                  }],
                 result.records)
  end

  def test_invalid
    assert_raise(Groonga::SyntaxError) do
      context.select(@books, :query => "<")
    end
  end

  class QueryFlagsTest < self
    def setup_tables
      Groonga::Schema.define do |schema|
        schema.create_table("Entries",
                            :type => :hash,
                            :key_type => "ShortText") do |table|
          table.text("content")
        end

        schema.create_table("Terms",
                            :type => :patricia_trie,
                            :default_tokenizer => "TokenBigram",
                            :key_type => "ShortText",
                            :key_normalize => true) do |table|
          table.index("Entries.content",
                      :name => "entries_content_index", :with_position => true)
        end
      end

      @entries = Groonga["Entries"]
    end

    class TestAllowLeadingNot < self
      def setup_data
        @first_post =
          @entries.add("The first post!",
                       "content" => "Welcome! This is my first post!")
        @groonga =
          @entries.add("Groonga",
                       "content" => "I started to use groonga. It's very fast!")
        @mroonga =
          @entries.add("Mroonga",
                       "content" => "I also started to use mroonga. " +
                                      "It's also very fast! Really fast!")
      end

      def test_true
        result = @entries.select do |record|
          record[:content].match("-mroonga", :allow_leading_not => true)
        end
        assert_equal_select_result([@first_post, @groonga],
                                   result)
      end

      def test_false
        assert_raise(Groonga::SyntaxError) do
          @entries.select do |record|
            record[:content].match("-mroonga", :allow_leading_not => false)
          end
        end
      end

      def test_default
        assert_raise(Groonga::SyntaxError) do
          @entries.select do |record|
            record[:content].match("-mroonga")
          end
        end
      end
    end

    class TestNoSyntaxError < self
      def setup_data
        @paren =
          @entries.add("Have paren",
                       "content" => "(hello)")
        @no_paren =
          @entries.add("Not have paren",
                       "content" => "hello")
      end

      def test_true
        result = @entries.select do |record|
          record[:content].match("(", :no_syntax_error => true)
        end
        assert_equal_select_result([@paren],
                                   result)
      end

      def test_false
        assert_raise(Groonga::SyntaxError) do
          @entries.select do |record|
            record[:content].match("(", :no_syntax_error => false)
          end
        end
      end

      def test_default
        assert_raise(Groonga::SyntaxError) do
          @entries.select do |record|
            record[:content].match("(")
          end
        end
      end
    end
  end

  private
  def normalize_drilldowns(drilldowns)
    normalized_drilldowns = {}
    drilldowns.each do |drilldown|
      normalized_drilldowns[drilldown.key] = [drilldown.n_hits, drilldown.items]
    end
    normalized_drilldowns
  end

  class EscapeTest < self
    def test_backslash
      key = "the \\ book"
      @books.add(key, :published => Time.parse("2011/04/01"))

      result = context.select(@books,
                              :filter => "_key == \"the \\\\ book\"",
                              :output_columns => ["_key", "published"])

      assert_equal([{
                     "_key" => key,
                     "published" => Time.parse("2011/04/01"),
                   }],
                   result.records)
    end

    def test_double_quote
      key = "the \"best\" book"
      @books.add(key, :published => Time.parse("2011/04/01"))

      result = context.select(@books,
                              :filter => "_key == \"the \\\"best\\\" book\"",
                              :output_columns => ["_key", "published"])

      assert_equal([{
                     "_key" => key,
                     "published" => Time.parse("2011/04/01"),
                   }],
                   result.records)
    end
  end
end
