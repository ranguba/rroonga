# Copyright (C) 2016-2017  Kouhei Sutou <kou@clear-code.com>
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

class DataColumnTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
  end

  sub_test_case "#apply_window_function" do
    def test_no_argument
      Groonga::Schema.define do |schema|
        schema.create_table("Comments") do |table|
          table.uint32("nth")
        end
      end
      comments = Groonga["Comments"]
      nth = Groonga["Comments.nth"]

      5.times do
        comments.add
      end

      options = {
        :sort_keys => [["_id", "desc"]],
      }
      nth.apply_window_function(options) do |record|
        record.call("record_number")
      end
      assert_equal([
                     [1, 5],
                     [2, 4],
                     [3, 3],
                     [4, 2],
                     [5, 1],
                   ],
                   comments.collect {|comment| [comment.id, comment.nth]})
    end

    def test_group_keys
      Groonga::Schema.define do |schema|
        schema.create_table("Comments") do |table|
          table.uint32("nth")
          table.short_text("category")
        end
      end
      comments = Groonga["Comments"]
      nth = Groonga["Comments.nth"]

      3.times do
        comments.add(:category => "a")
      end
      3.times do
        comments.add(:category => "b")
      end

      options = {
        :sort_keys => [["_id", "desc"]],
        :group_keys => ["category"],
      }
      nth.apply_window_function(options) do |record|
        record.call("record_number")
      end
      values = comments.collect do |comment|
        [
          comment.id,
          comment.nth,
          comment.category,
        ]
      end
      assert_equal([
                     [1,  3, "a"],
                     [2,  2, "a"],
                     [3,  1, "a"],
                     [4,  3, "b"],
                     [5,  2, "b"],
                     [6,  1, "b"],
                   ],
                   values)
    end
  end
end
