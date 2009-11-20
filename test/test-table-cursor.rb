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

class TableCursorTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database

    @bookmarks_path = @tables_dir + "table"
    @bookmarks = Groonga::PatriciaTrie.create(:name => "bookmarks",
                                              :path => @bookmarks_path.to_s)
    @groonga_bookmark = @bookmarks.add("groonga")
    @cutter_bookmark = @bookmarks.add("Cutter")
    @ruby_bookmark = @bookmarks.add("Ruby")
  end

  def test_open
    keys = []
    @bookmarks.open_cursor do |cursor|
      while cursor.next
        keys << cursor.key
      end
    end
    assert_equal(["Cutter", "Ruby", "groonga"],
                 keys)
  end

  def test_open_ascendent
    record_and_key_list = []
    @bookmarks.open_cursor(:order => :ascending) do |cursor|
      record_and_key_list = cursor.collect {|record| [record, cursor.key]}
    end
    assert_equal([[@cutter_bookmark, "Cutter"],
                  [@ruby_bookmark, "Ruby"],
                  [@groonga_bookmark, "groonga"]],
                 record_and_key_list)
  end

  def test_without_limit_and_offset
    bookmarks = create_bookmarks
    add_ids(bookmarks)
    results = []
    bookmarks.open_cursor do |cursor|
      cursor.each do |record|
        results << record["id"]
      end
    end

    assert_equal((100..199).to_a, results)
  end

  def test_with_limit
    bookmarks = create_bookmarks
    add_ids(bookmarks)
    results = []
    bookmarks.open_cursor(:limit => 20) do |cursor|
      cursor.each do |record|
        results << record["id"]
      end
    end

    assert_equal((100...120).to_a, results)
  end

  def test_with_offset
    bookmarks = create_bookmarks
    add_ids(bookmarks)
    results = []
    bookmarks.open_cursor(:offset => 20) do |cursor|
      cursor.each do |record|
        results << record["id"]
      end
    end

    assert_equal((120...200).to_a, results)
  end

  def test_with_limit_and_offset
    bookmarks = create_bookmarks
    add_ids(bookmarks)
    results = []
    bookmarks.open_cursor(:limit => 20, :offset => 20) do |cursor|
      cursor.each do |record|
        results << record["id"]
      end
    end

    assert_equal((120...140).to_a, results)
  end

  def test_delete
    bookmarks = create_bookmarks
    add_ids(bookmarks)

    bookmarks.open_cursor(:limit => 20) do |cursor|
      20.times do
        cursor.next
        cursor.delete
      end
    end

    results = []
    bookmarks.open_cursor do |cursor|
      cursor.each do |record|
        results << record["id"]
      end
    end

    assert_equal((120...200).to_a, results)
  end

  def test_patricia_trie_cursor_key
    bookmarks = Groonga::PatriciaTrie.create(:name => "patricia_trie_table")
    bookmarks.add("test")
    bookmarks.open_cursor do |cursor|
      cursor.next
      assert_equal("test", cursor.key)
    end
  end

  private
  def create_bookmarks
    bookmarks = Groonga::Array.create(:name => "<bookmarks>")
    bookmarks.define_column("id", "<int>")
    bookmarks
  end

  def add_ids(bookmarks)
    (0...100).to_a.each do |i|
      bookmark = bookmarks.add
      bookmark["id"] = i + 100
    end
    bookmarks
  end
end
