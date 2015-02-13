# Copyright (C) 2015  Masafumi Yokoyama <yokoyama@clear-code.com>
# Copyright (C) 2009-2013  Kouhei Sutou <kou@clear-code.com>
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

class TableGroupTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  class MaxNSubRecordsTest < self
    setup
    def setup_schema
      Groonga::Schema.define do |schema|
        schema.create_table("Bookmarks", :type => :hash) do |table|
          table.text("title")
        end

        schema.create_table("Comments", :type => :array) do |table|
          table.reference("bookmark")
          table.text("content")
          table.int32("rank")
        end
      end
    end

    setup
    def setup_data
      setup_bookmarks
      setup_comments
    end

    def setup_bookmarks
      @bookmarks = Groonga["Bookmarks"]
      @groonga = @bookmarks.add("http://groonga.org/", :title => "groonga")
      @ruby = @bookmarks.add("http://ruby-lang.org/", :title => "Ruby")
    end

    def setup_comments
      @comments = Groonga["Comments"]
      @comments.add(:bookmark => @groonga,
                    :content => "garbage comment1",
                    :rank => 0)
      @comments.add(:bookmark => @groonga,
                    :content => "garbage comment2",
                    :rank => 0)
      @comments.add(:bookmark => @groonga,
                    :content => "full-text search",
                    :rank => 1)
      @comments.add(:bookmark => @groonga,
                    :content => "column store",
                    :rank => 5)
      @comments.add(:bookmark => @ruby,
                    :content => "object oriented script language",
                    :rank => 100)
      @comments.add(:bookmark => @ruby,
                    :content => "multi paradigm programming language",
                    :rank => 80)
    end

    setup
    def setup_searched
      @records = @comments.select do |record|
        record.rank > 0
      end
    end

    def test_upper_limit
      grouped_records = @records.group("bookmark", :max_n_sub_records => 2)
      groups = grouped_records.collect do |record|
        sub_record_contents = record.sub_records.collect do |sub_record|
          sub_record.content
        end
        [record.title, sub_record_contents]
      end
      assert_equal([
                     [
                       "groonga",
                       [
                         "full-text search",
                         "column store",
                       ],
                     ],
                     [
                       "Ruby",
                       [
                         "object oriented script language",
                         "multi paradigm programming language",
                       ],
                     ],
                   ],
                   groups)
    end

    def test_less_than_limit
      sorted = @records.sort([{:key => "rank", :order => :descending}],
                             :limit => 3, :offset => 0)
      grouped_records = sorted.group("bookmark", :max_n_sub_records => 2)
      groups = grouped_records.collect do |record|
        sub_record_ranks = record.sub_records.collect do |sub_record|
          sub_record.rank
        end
        [record.title, sub_record_ranks]
      end
      assert_equal([
                     ["Ruby", [100, 80]],
                     ["groonga", [5]]
                   ],
                   groups)
    end
  end

  class KeyTest < self
    setup
    def setup_schema
      Groonga::Schema.define do |schema|
        schema.create_table("Bookmarks", :type => :hash) do |table|
          table.text("title")
        end

        schema.create_table("Comments", :type => :array) do |table|
          table.reference("bookmark")
          table.text("content")
          table.int32("rank")
        end
      end
    end

    setup
    def setup_data
      setup_bookmarks
      setup_comments
    end

    def setup_bookmarks
      @bookmarks = Groonga["Bookmarks"]
      @groonga = @bookmarks.add("http://groonga.org/", :title => "groonga")
      @ruby = @bookmarks.add("http://ruby-lang.org/", :title => "Ruby")
    end

    def setup_comments
      @comments = Groonga["Comments"]
      @comments.add(:bookmark => @groonga,
                    :content => "full-text search")
      @comments.add(:bookmark => @groonga,
                    :content => "column store")
      @comments.add(:bookmark => @ruby,
                    :content => "object oriented script language")
    end

    def test_string
      grouped_records = @comments.group("bookmark").collect do |record|
        bookmark = record.key
        [
          record.n_sub_records,
          bookmark["title"],
          bookmark.key,
        ]
      end
      assert_equal([
                     [2, "groonga", "http://groonga.org/"],
                     [1, "Ruby", "http://ruby-lang.org/"],
                   ],
                   grouped_records)
    end

    def test_array
      grouped_records = @comments.group(["bookmark"]).collect do |record|
        bookmark = record.key
        [
          record.n_sub_records,
          bookmark["title"],
          bookmark.key,
        ]
      end
      assert_equal([
                     [2, "groonga", "http://groonga.org/"],
                     [1, "Ruby", "http://ruby-lang.org/"],
                   ],
                   grouped_records)
    end

    def test_nonexistent
      message = "unknown group key: <\"nonexistent\">: <#{@comments.inspect}>"
      assert_raise(ArgumentError.new(message)) do
        @comments.group("nonexistent")
      end
    end
  end

  class MultipleKeyTest < self
    setup
    def setup_schema
      Groonga::Schema.define do |schema|
        schema.create_table("Memos", :type => :hash) do |table|
          table.short_text("tag")
          table.int64("priority")
        end
      end
    end

    setup
    def setup_data
      setup_memos
    end

    def setup_memos
      @memos = Groonga["Memos"]
      @memos.add("Groonga1",
                 :tag => "Groonga",
                 :priority => 10)
      @memos.add("Groonga2",
                 :tag => "Groonga",
                 :priority => 20)
      @memos.add("Mroonga1",
                 :tag => "Mroonga",
                 :priority => 10)
      @memos.add("Mroonga2",
                 :tag => "Mroonga",
                 :priority => 10)
    end

    def test_two_keys
      keys = ["tag", "priority"]
      grouped_records = @memos.group(keys).collect do |record|
        sub_record = record.sub_records.first
        [
          record.n_sub_records,
          sub_record.tag,
          sub_record.priority,
        ]
      end

      assert_equal([
                     [1, "Groonga", 10],
                     [1, "Groonga", 20],
                     [2, "Mroonga", 10],
                   ],
                   grouped_records)
    end
  end
end
