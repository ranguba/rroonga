# Copyright (C) 2016-2022  Sutou Kouhei <kou@clear-code.com>
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
    def test_sort_keys
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

  sub_test_case "#apply_expression" do
    def test_simple
      Groonga::Schema.define do |schema|
        schema.create_table("Comments") do |table|
          table.uint32("base")
          table.uint32("plus1")
        end
      end
      comments = Groonga["Comments"]
      plus1 = Groonga["Comments.plus1"]

      3.times do |i|
        comments.add(:base => i)
      end

      plus1.apply_expression do |record|
        record.base + 1
      end
      assert_equal([
                     [0, 1],
                     [1, 2],
                     [2, 3],
                   ],
                   comments.collect {|comment| [comment.base, comment.plus1]})
    end
  end

  sub_test_case "#missing_mode" do
    def test_add
      Groonga::Schema.define do |schema|
        schema.create_table("Tags",
                            type: :hash,
                            key_type: :short_text) do |table|
        end
        schema.create_table("Memos") do |table|
          table.reference("tags",
                          "Tags",
                          type: :vector,
                          missing_mode: :add)
        end
      end
      memos = Groonga["Memos"]
      memos_tags = Groonga["Memos.tags"]
      tags = Groonga["Tags"]

      record = memos.add(tags: ["nonexistent"])

      assert_equal({
                     missing_mode: :add,
                     missing_add: true,
                     missing_ignore: false,
                     missing_nil: false,
                     values: [tags["nonexistent"]],
                   },
                   {
                     missing_mode: memos_tags.missing_mode,
                     missing_add: memos_tags.missing_add?,
                     missing_ignore: memos_tags.missing_ignore?,
                     missing_nil: memos_tags.missing_nil?,
                     values: record.tags,
                   })
    end

    def test_ignore
      Groonga::Schema.define do |schema|
        schema.create_table("Tags",
                            type: :hash,
                            key_type: :short_text) do |table|
        end
        schema.create_table("Memos") do |table|
          table.reference("tags",
                          "Tags",
                          type: :vector,
                          missing_mode: :ignore)
        end
      end
      memos = Groonga["Memos"]
      memos_tags = Groonga["Memos.tags"]

      record = memos.add(tags: ["nonexistent"])

      assert_equal({
                     missing_mode: :ignore,
                     missing_add: false,
                     missing_ignore: true,
                     missing_nil: false,
                     values: [],
                   },
                   {
                     missing_mode: memos_tags.missing_mode,
                     missing_add: memos_tags.missing_add?,
                     missing_ignore: memos_tags.missing_ignore?,
                     missing_nil: memos_tags.missing_nil?,
                     values: record.tags,
                   })
    end

    def test_nil
      Groonga::Schema.define do |schema|
        schema.create_table("Tags",
                            type: :hash,
                            key_type: :short_text) do |table|
        end
        schema.create_table("Memos") do |table|
          table.reference("tags",
                          "Tags",
                          type: :vector,
                          missing_mode: :nil,
                          invalid_mode: :ignore)
        end
      end
      memos = Groonga["Memos"]
      memos_tags = Groonga["Memos.tags"]

      record = memos.add(tags: ["nonexistent"])

      assert_equal({
                     missing_mode: :nil,
                     missing_add: false,
                     missing_ignore: false,
                     missing_nil: true,
                     values: [nil],
                   },
                   {
                     missing_mode: memos_tags.missing_mode,
                     missing_add: memos_tags.missing_add?,
                     missing_ignore: memos_tags.missing_ignore?,
                     missing_nil: memos_tags.missing_nil?,
                     values: record.tags,
                   })
    end
  end

  sub_test_case "#invalid_mode" do
    def test_error
      Groonga::Schema.define do |schema|
        schema.create_table("Memos") do |table|
          table.uint32("count", invalid_mode: :error)
        end
      end
      memos = Groonga["Memos"]
      memos_count = Groonga["Memos.count"]

      record = memos.add
      assert_raise(Groonga::InvalidArgument) do
        record.count = "invalid"
      end

      assert_equal({
                     invalid_mode: :error,
                     invalid_error: true,
                     invalid_warn: false,
                     invalid_ignore: false,
                     value: 0,
                   },
                   {
                     invalid_mode: memos_count.invalid_mode,
                     invalid_error: memos_count.invalid_error?,
                     invalid_warn: memos_count.invalid_warn?,
                     invalid_ignore: memos_count.invalid_ignore?,
                     value: record.count,
                   })
    end

    def test_warn
      Groonga::Schema.define do |schema|
        schema.create_table("Memos") do |table|
          table.uint32("count", invalid_mode: :warn)
        end
      end
      memos = Groonga["Memos"]
      memos_count = Groonga["Memos.count"]

      record = memos.add
      record.count = "invalid"

      assert_equal({
                     invalid_mode: :warn,
                     invalid_error: false,
                     invalid_warn: true,
                     invalid_ignore: false,
                     value: 0,
                   },
                   {
                     invalid_mode: memos_count.invalid_mode,
                     invalid_error: memos_count.invalid_error?,
                     invalid_warn: memos_count.invalid_warn?,
                     invalid_ignore: memos_count.invalid_ignore?,
                     value: record.count,
                   })
    end

    def test_ignore
      Groonga::Schema.define do |schema|
        schema.create_table("Memos") do |table|
          table.uint32("count", invalid_mode: :ignore)
        end
      end
      memos = Groonga["Memos"]
      memos_count = Groonga["Memos.count"]

      record = memos.add
      record.count = "invalid"

      assert_equal({
                     invalid_mode: :ignore,
                     invalid_error: false,
                     invalid_warn: false,
                     invalid_ignore: true,
                     value: 0,
                   },
                   {
                     invalid_mode: memos_count.invalid_mode,
                     invalid_error: memos_count.invalid_error?,
                     invalid_warn: memos_count.invalid_warn?,
                     invalid_ignore: memos_count.invalid_ignore?,
                     value: record.count,
                   })
    end
  end
end
