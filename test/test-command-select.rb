# Copyright (C) 2010-2012  Kouhei Sutou <kou@clear-code.com>
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

class CommandSelectTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database
  setup
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

  def test_drill_down
    result = context.select(@users,
                            :output_columns => ["_key"],
                            :drill_down => ["_key", "book"],
                            :drill_down_output_columns => "_key",
                            :drill_down_limit => 10)
    drill_down = normalize_drill_down(result.drill_down)
    assert_equal([4,
                  [{"_key" => "morita"},
                   {"_key" => "gunyara-kun"},
                   {"_key" => "yu"},
                   {"_key" => "ryoqun"}],
                  {
                    "_key" => [4, [{"_key" => "morita"},
                                   {"_key" => "gunyara-kun"},
                                   {"_key" => "yu"},
                                   {"_key" => "ryoqun"}]],
                    "book" => [2, [{"_key" => "the groonga book"},
                                   {"_key" => "the groonga book (2)"}]],
                  },
                 ],
                 [result.n_hits, result.records, drill_down])
  end

  def test_drill_down_with_no_hit
    result = context.select(@users,
                            :filter => "_key == \"no\\ hit\"",
                            :output_columns => ["_key"],
                            :drill_down => ["_key", "book"],
                            :drill_down_output_columns => "_key",
                            :drill_down_limit => 10)
    drill_down = normalize_drill_down(result.drill_down)
    assert_equal([0, [],
                  {
                    "_key" => [0, []],
                    "book" => [0, []],
                  },
                 ],
                 [result.n_hits, result.records, drill_down])
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

  private
  def normalize_drill_down(drill_down)
    normalized_drill_down = {}
    drill_down.each do |key, drill|
      normalized_drill_down[key] = [drill.n_hits, drill.records]
    end
    normalized_drill_down
  end
end
