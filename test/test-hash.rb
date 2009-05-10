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

class HashTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_delete
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :path => bookmarks_path.to_s)

    groonga = bookmarks.add("groonga")
    google = bookmarks.add("Google")
    cutter = bookmarks.add("Cutter")

    assert_equal(["groonga", "Google", "Cutter"],
                 bookmarks.collect {|bookmark| bookmark.key})

    bookmarks.delete(google.id)
    assert_equal(["groonga", "Cutter"],
                 bookmarks.collect {|bookmark| bookmark.key})

    bookmarks.delete(cutter.key)
    assert_equal(["groonga"],
                 bookmarks.collect {|bookmark| bookmark.key})
  end

  def test_array_reference
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :path => bookmarks_path.to_s,
                                     :key_type => "<shorttext>")
    bookmarks["http://google.com/"] = "groonga"
    assert_equal("groonga", bookmarks["http://google.com/"])
  end

  def test_lookup
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :path => bookmarks_path.to_s,
                                     :key_type => "<shorttext>")
    bookmark = bookmarks.add("http://google.com/")
    assert_equal(bookmark, bookmarks.lookup("http://google.com/"))
  end
end
