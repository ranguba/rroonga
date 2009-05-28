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
    value = "groonga"
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :path => bookmarks_path.to_s,
                                     :key_type => "<shorttext>",
                                     :value_size => value.size)
    bookmarks["http://google.com/"] = value
    assert_equal(value, bookmarks["http://google.com/"])
  end

  def test_lookup
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :path => bookmarks_path.to_s,
                                     :key_type => "<shorttext>")
    bookmark = bookmarks.add("http://google.com/")
    assert_equal(bookmark, bookmarks.lookup("http://google.com/"))
  end


  def test_inspect_anonymous
    path = @tables_dir + "anoymous.groonga"
    anonymous_table = Groonga::Hash.create(:path => path.to_s)
    assert_equal("#<Groonga::Hash " +
                 "id: <#{anonymous_table.id}>, " +
                 "name: (anonymous), " +
                 "path: <#{path}>, " +
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 anonymous_table.inspect)
  end

  def test_inspect_anonymous_temporary
    anonymous_table = Groonga::Hash.create
    assert_equal("#<Groonga::Hash " +
                 "id: <#{anonymous_table.id}>, " +
                 "name: (anonymous), " +
                 "path: (temporary), " +
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 anonymous_table.inspect)
  end

  def test_inspect_named
    path = @tables_dir + "named.groonga"
    named_table = Groonga::Hash.create(:name => "name", :path => path.to_s)
    assert_equal("#<Groonga::Hash " +
                 "id: <#{named_table.id}>, " +
                 "name: <name>, " +
                 "path: <#{path}>, " +
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 named_table.inspect)
  end

  def test_inspect_named_temporary
    named_table = Groonga::Hash.create(:name => "name")
    assert_equal("#<Groonga::Hash " +
                 "id: <#{named_table.id}>, " +
                 "name: <name>, " +
                 "path: (temporary), " +
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 named_table.inspect)
  end

  def test_encoding
    assert_equal(Groonga::Encoding.default,
                 Groonga::Hash.create.encoding)
  end
end
