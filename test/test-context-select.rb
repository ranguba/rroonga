# Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>
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

class ContextSelectTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database
  setup
  def setup_data
    @books = Groonga::Hash.create(:name => "Books", :key_type => "ShortText")
    @books.define_column("published", "Time")
    @users = Groonga::Hash.create(:name => "Users", :key_type => "ShortText")
    @users.define_column("book", "Books")

    @books.add("the groonga book", :published => Time.parse("2010/04/01"))
    @users.add("morita", :book => "the groonga book")
    @users.add("gunyara-kun", :book => "the groonga book")
    @users.add("yu")
  end

  def test_no_option
    result = context.select(@users)
    assert_equal([3,
                  [{"_id" => 1, "_key" => "morita",
                     "book" => "the groonga book"},
                   {"_id" => 2, "_key" => "gunyara-kun",
                     "book" => "the groonga book"},
                   {"_id" => 3, "_key" => "yu",
                     "book" => ""}]],
                 [result.n_hits, result.records])
  end

  def test_success
    result = context.select(@users)
    assert_equal([true, 0, nil],
                 [result.success?, result.return_code, result.error_message])
  end

  def test_output_columns
    result = context.select(@users, :output_columns => ["_key"])
    assert_equal([3,
                  [{"_key" => "morita"},
                   {"_key" => "gunyara-kun"},
                   {"_key" => "yu"}]],
                 [result.n_hits, result.records])
  end

  def test_drill_down
    result = context.select(@users,
                            :output_columns => ["_key"],
                            :drill_down => ["_key", "book"],
                            :drill_down_output_columns => "_key",
                            :drill_down_limit => 10)
    drill_down = normalize_drill_down(result.drill_down)
    assert_equal([3,
                  [{"_key" => "morita"},
                   {"_key" => "gunyara-kun"},
                   {"_key" => "yu"}],
                  {
                    "_key" => [3, [{"_key" => "morita"},
                                   {"_key" => "gunyara-kun"},
                                   {"_key" => "yu"}]],
                    "book" => [1, [{"_key" => "the groonga book"}]],
                  },
                 ],
                 [result.n_hits, result.records, drill_down])
  end

  def test_drill_down_with_no_hit
    result = context.select(@users,
                            :filter => "_key == \"no hit\"",
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
                  }],
                 result.records)
  end

  private
  def normalize_drill_down(drill_down)
    normalized_drill_down = {}
    drill_down.each do |key, drill_down|
      normalized_drill_down[key] = [drill_down.n_hits, drill_down.records]
    end
    normalized_drill_down
  end
end
