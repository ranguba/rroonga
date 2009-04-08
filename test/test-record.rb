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

class RecordTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database

    @bookmarks_path = @tables_dir + "table"
    @bookmarks = Groonga::PatriciaTrie.create(:name => "bookmarks",
                                              :path => @bookmarks_path.to_s)

    @uri_column_path = @columns_dir + "uri"
    @bookmarks_uri = @bookmarks.define_column("uri", "<shorttext>",
                                              :path => @uri_column_path.to_s)

    @comment_column_path = @columns_dir + "comment"
    @bookmarks_comment =
      @bookmarks.define_column("comment", "<text>",
                               :path => @comment_column_path.to_s)

    @content_column_path = @columns_dir + "content"
    @bookmarks_content =
      @bookmarks.define_column("content", "<longtext>",
                               :type => "index",
                               :with_section => true,
                               :with_weight => true,
                               :with_position => true,
                               :path => @content_column_path.to_s)
  end

  def test_column_accessor
    groonga = @bookmarks.add("groonga")

    groonga["uri"] = "http://groonga.org/"
    assert_equal("http://groonga.org/", groonga["uri"])

    groonga["comment"] = "fulltext search engine"
    assert_equal("fulltext search engine", groonga["comment"])
  end

  def test_have_column?
    groonga = @bookmarks.add("groonga")
    assert_true(groonga.have_column?(:uri))
    assert_false(groonga.have_column?(:nonexistent))
  end

  def test_get_nonexistent_column
    groonga = @bookmarks.add("groonga")
    assert_nil(groonga["nonexistent"])
  end

  def test_set_nonexistent_column
    groonga = @bookmarks.add("groonga")
    assert_raise(Groonga::Error) do
      groonga["nonexistent"] = "value"
    end
  end

  priority :must
  def test_update_index_column
    groonga = @bookmarks.add("groonga")
    groonga["content"] = "<html><body>groonga</body></html>"
    assert_equal([], groonga.search("content", "groonga")) # FIXME
  end
end
