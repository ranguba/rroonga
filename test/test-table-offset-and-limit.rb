# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
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

module TestOffsetAndLimitSupport
  def test_zero_and_positive_offset
    assert_equal(((100 + 0)...200).to_a, ids(:offset => 0))
    assert_equal(((100 + 32)...200).to_a, ids(:offset => 32))
    assert_equal(((100 + 99)...200).to_a, ids(:offset => 99))
    assert_raise(Groonga::TooLargeOffset) do
      ids(:offset => 100)
    end
  end

  def test_negative_offset
    assert_equal(((200 - 1)...200).to_a, ids(:offset => -1))
    assert_equal(((200 - 32)...200).to_a, ids(:offset => -32))
    assert_equal(((200 - 100)...200).to_a, ids(:offset => -100))
    assert_equal(((200 - 100)...101).to_a, ids(:offset => -199))
    assert_equal(((200 - 100)...100).to_a, ids(:offset => -200))
    assert_raise(Groonga::TooSmallOffset) do
      ids(:offset => -201)
    end
  end

  def test_zero_and_positive_limit
    all_ids = (100...200).to_a
    assert_equal(all_ids[0, 0], ids(:limit => 0))
    assert_equal(all_ids[0, 32], ids(:limit => 32))
    assert_equal(all_ids[0, 100], ids(:limit => 100))
    assert_nothing_raised do
      ids(:limit => 101)
    end
  end

  def test_negative_limit
    all_ids = (100...200).to_a
    assert_equal(all_ids[0..-1], ids(:limit => -1))
    assert_equal(all_ids[0..-32], ids(:limit => -32))
    assert_equal(all_ids[0..-100], ids(:limit => -100))
    assert_equal([], ids(:limit => -101))
  end

  private
  def create_bookmarks
    @bookmarks = Groonga::Array.create(:name => "Bookmarks")
    @bookmarks.define_column("id", "Int32")
    @bookmarks
  end

  def add_ids
    100.times do |i|
      bookmark = @bookmarks.add
      bookmark["id"] = i + 100
    end
    @bookmarks
  end
end

class TestOffsetAndLimit < Test::Unit::TestCase
  include GroongaTestUtils
  def setup
    setup_database
    @bookmarks = create_bookmarks
    add_ids
  end
end

class TestTableCursorOffsetAndLimit < TestOffsetAndLimit
  include TestOffsetAndLimitSupport

  private
  def ids(options={})
    @bookmarks.open_cursor(options) do |cursor|
      cursor.collect do |record|
        record["id"]
      end
    end
  end
end

class TestTableSortOffsetAndLimit < TestOffsetAndLimit
  include TestOffsetAndLimitSupport

  private
  def ids(options={})
    @bookmarks.sort([:key => "id", :order => :asc], options).collect do |record|
      record["id"]
    end
  end
end
