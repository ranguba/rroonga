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

    @bookmarks_path = @tables_dir + "bookmarks"
    @bookmarks = Groonga::Array.create(:name => "bookmarks",
                                       :path => @bookmarks_path.to_s)

    @uri_column_path = @columns_dir + "uri"
    @bookmarks_uri = @bookmarks.define_column("uri", "<shorttext>",
                                              :path => @uri_column_path.to_s)

    @comment_column_path = @columns_dir + "comment"
    @bookmarks_comment =
      @bookmarks.define_column("comment", "<text>",
                               :path => @comment_column_path.to_s)

    @content_column_path = @columns_dir + "content"
    @bookmarks_content = @bookmarks.define_column("content", "<longtext>")

    @bookmarks_index_path = @tables_dir + "bookmarks-index"
    @bookmarks_index =
      Groonga::PatriciaTrie.create(:name => "bookmarks-index",
                                   :path => @bookmarks_index_path.to_s)
    @content_index_column_path = @columns_dir + "content-index"
    @bookmarks_index_content =
      @bookmarks_index.define_column("content", "<longtext>",
                                     :type => "index",
                                     :with_section => true,
                                     :with_weight => true,
                                     :with_position => true,
                                     :path => @content_index_column_path.to_s)

    @content_index_id_column_path = @columns_dir + "content-index-id"
    @bookmarks_content_index_id =
      @bookmarks.define_column("content-index-id", @bookmarks_index,
                               :path => @content_index_id_column_path.to_s)
  end

  def test_column_accessor
    groonga = @bookmarks.add

    groonga["uri"] = "http://groonga.org/"
    assert_equal("http://groonga.org/", groonga["uri"])

    groonga["comment"] = "fulltext search engine"
    assert_equal("fulltext search engine", groonga["comment"])
  end

  def test_have_column?
    groonga = @bookmarks.add
    assert_true(groonga.have_column?(:uri))
    assert_false(groonga.have_column?(:nonexistent))
  end

  def test_get_nonexistent_column
    groonga = @bookmarks.add
    assert_nil(groonga["nonexistent"])
  end

  def test_set_nonexistent_column
    groonga = @bookmarks.add
    assert_raise(Groonga::Error) do
      groonga["nonexistent"] = "value"
    end
  end

  def test_update_index_column
    groonga = @bookmarks_index.add("groonga")
    groonga["content"] = "<html><body>groonga</body></html>"

    ruby = @bookmarks_index.add("ruby")
    ruby["content"] = "<html><body>ruby</body></html>"

    assert_index_search([groonga.id],
                        groonga.search("content", "groonga").records)

    assert_index_search([ruby.id, groonga.id],
                        groonga.search("content", "html").records)
  end

  def test_set_object_id
    groonga = @bookmarks.add
    index = @bookmarks_index.add("groonga")
    groonga["content-index-id"] = index.id
    assert_equal(index.id, groonga["content-index-id"])
  end

  def test_set_nil
    groonga = @bookmarks.add
    groonga["content"] = nil
    assert_nil(groonga["content"])
  end

  def test_range
    assert_equal(Groonga::Type::SHORT_TEXT, @bookmarks_uri.range)
    assert_equal(Groonga::Type::TEXT, @bookmarks_comment.range)
    assert_equal(Groonga::Type::LONG_TEXT, @bookmarks_content.range)
    assert_equal(Groonga::Type::LONG_TEXT, @bookmarks_index_content.range)
    assert_equal(@bookmarks_index,
                 Groonga::Context.default[@bookmarks_content_index_id.range])
  end

  def test_delete
    bookmark1 = @bookmarks.add
    bookmark2 = @bookmarks.add
    bookmark3 = @bookmarks.add

    assert_equal(3, @bookmarks.size)
    bookmark2.delete
    assert_equal(2, @bookmarks.size)
  end

  def test_value
    bookmark = @bookmarks.add
    assert_equal("", bookmark.value.delete("\0"))
    bookmark.value = "http://groonga.org/"
    assert_equal("http://groonga.org/", bookmark.value.delete("\0"))
  end

  private
  def assert_index_search(expected_ids, records)
    ids = records.collect do |record|
      record.key.unpack("i")[0]
    end
    assert_equal(expected_ids, ids)
  end
end
