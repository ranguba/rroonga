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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

module TestOffsetAndLimitSupport
  def test_zero_and_positive_offset
    assert_equal(((100+0)...200).to_a, ids(:offset => 0))
    assert_equal(((100+32)...200).to_a, ids(:offset => 32))
    assert_equal(((100+99)...200).to_a, ids(:offset => 99))
    assert_raise(Groonga::InvalidArgument) do
      ids(:offset => 100)
    end
  end

  def test_negative_offset
    assert_equal(((200-1)...200).to_a, ids(:offset => -1))
    assert_equal(((200-32)...200).to_a, ids(:offset => -32))
    assert_equal(((200-100)...200).to_a, ids(:offset => -100))
    assert_raise(Groonga::InvalidArgument) do
      ids(:offset => -101)
    end
  end

  def test_zero_and_positive_limit
    assert_equal((100...200).to_a[0, 0], ids(:limit => 0))
    assert_equal((100...200).to_a[0, 32], ids(:limit => 32))
    assert_equal((100...200).to_a[0, 100], ids(:limit => 100))
    assert_nothing_raised do
      ids(:limit => 101)
    end
  end

  def test_negative_limit
    assert_equal((100...200).to_a[0..-1], ids(:limit => -1))
    assert_equal((100...200).to_a[0..-32], ids(:limit => -32))
    assert_equal((100...200).to_a[0..-100], ids(:limit => -100))
    assert_raise(Groonga::InvalidArgument) do
      ids(:offset => -101)
    end
  end

  private
  def create_bookmarks
    @bookmarks = Groonga::Array.create(:name => "<bookmarks>")
    @bookmarks.define_column("id", "<int>")
    @bookmarks
  end

  def add_ids
    (0...100).to_a.each do |i|
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
    ids = []
    @bookmarks.open_cursor(options) do |cursor|
      cursor.each do |record|
        ids << record["id"]
      end
    end
    ids
  end
end

class TestTableSortOffsetAndLimit < TestOffsetAndLimit
  include TestOffsetAndLimitSupport

  private
  def ids(options={})
    ids = []
    @bookmarks.sort([:key => "id", :order => :asc], options).each do |record|
      ids << record["id"]
    end
    ids
  end
end

