# Copyright (C) 2013  Kouhei Sutou <kou@clear-code.com>
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

class SubRecordsTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
    setup_schema
    setup_data
    setup_searched
  end

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

  def setup_searched
    @records = @comments.select do |record|
      record.rank > 0
    end
  end

  def test_array_reference
    grouped_records = @records.group("bookmark", :max_n_sub_records => 2)
    groups = grouped_records.collect do |record|
      [record.title, record.sub_records[0].content]
    end
    assert_equal([
                   [
                     "groonga",
                     "full-text search",
                   ],
                   [
                     "Ruby",
                     "object oriented script language",
                   ],
                 ],
                 groups)
  end

  def test_to_ary
    grouped_records = @records.group("bookmark", :max_n_sub_records => 2)
    sub_record1, sub_record2 = grouped_records.first.sub_records
    assert_equal([
                   "full-text search",
                   "column store",
                 ],
                 [
                   sub_record1.content,
                   sub_record2.content,
                 ])
  end
end
