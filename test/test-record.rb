# Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>
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

    setup_addresses_table
    setup_users_table
    setup_bookmarks_table
    setup_indexes
  end

  def setup_addresses_table
    @addresses_path = @tables_dir + "addresses"
    @addresses = Groonga::Array.create(:name => "Addresses",
                                       :path => @addresses_path.to_s)

    @addresses_mail_column_path = @columns_dir + "mail"
    @addresses_mail_column =
      @addresses.define_column("mail", "ShortText",
                               :path => @addresses_mail_column_path.to_s)
  end

  def setup_users_table
    @users_path = @tables_dir + "users"
    @users = Groonga::Array.create(:name => "Users",
                                   :path => @users_path.to_s)

    @users_name_column_path = @columns_dir + "name"
    @users_name_column =
      @users.define_column("name", "ShortText",
                           :path => @users_name_column_path.to_s)

    @users_addresses_column_path = @columns_dir + "addresses"
    @users_addresses_column =
      @users.define_column("addresses", @addresses,
                           :path => @users_addresses_column_path.to_s,
                           :type => "vector")
  end

  def setup_bookmarks_table
    @bookmarks_path = @tables_dir + "bookmarks"
    value_type = Groonga::Type.new("Text512", :size => 512)
    @bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                       :value_type => value_type,
                                       :path => @bookmarks_path.to_s)

    @uri_column_path = @columns_dir + "uri"
    @bookmarks_uri = @bookmarks.define_column("uri", "ShortText",
                                              :path => @uri_column_path.to_s)

    @rate_column_path = @columns_dir + "rate"
    @bookmarks_rate = @bookmarks.define_column("rate", "Int32",
                                               :path => @rate_column_path.to_s)

    @comment_column_path = @columns_dir + "comment"
    @bookmarks_comment =
      @bookmarks.define_column("comment", "Text",
                               :path => @comment_column_path.to_s)

    @content_column_path = @columns_dir + "content"
    @bookmarks_content = @bookmarks.define_column("content", "LongText")

    @user_column_path = @columns_dir + "user"
    @bookmarks_user = @bookmarks.define_column("user", @users)
  end

  def setup_indexes
    @bookmarks_index_path = @tables_dir + "bookmarks-index"
    @bookmarks_index =
      Groonga::PatriciaTrie.create(:name => "BookmarksIndex",
                                   :path => @bookmarks_index_path.to_s)
    @bookmarks_index.default_tokenizer = "TokenBigram"

    @content_index_column_path = @columns_dir + "content-index"
    @bookmarks_content_index =
      @bookmarks_index.define_index_column("content", @bookmarks,
                                           :with_section => true,
                                           :with_weight => true,
                                           :with_position => true,
                                           :path => @content_index_column_path.to_s)
    @bookmarks_content_index.source = @bookmarks_content

    @uri_index_column_path = @columns_dir + "uri-index"
    @bookmarks_uri_index =
      @bookmarks_index.define_index_column("uri", @bookmarks,
                                           :with_position => true,
                                           :path => @uri_index_column_path.to_s)
    @bookmarks_uri_index.source = @bookmarks_uri
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
    assert_raise(Groonga::NoSuchColumn) do
      groonga["nonexistent"]
    end
  end

  def test_set_nonexistent_column
    groonga = @bookmarks.add
    assert_raise(Groonga::NoSuchColumn) do
      groonga["nonexistent"] = "value"
    end
  end

  def test_set_object_id
    groonga = @bookmarks.add
    daijiro = @users.add
    daijiro["name"] = "daijiro"
    assert_nil(groonga["user"])
    groonga["user"] = daijiro.id
    assert_equal(daijiro, groonga["user"])
  end

  def test_set_nil
    groonga = @bookmarks.add
    groonga["content"] = nil
    assert_nil(groonga["content"])
  end

  def test_delete
    bookmark1 = @bookmarks.add
    bookmark2 = @bookmarks.add
    bookmark3 = @bookmarks.add

    assert_equal(3, @bookmarks.size)
    bookmark2.delete
    assert_equal(2, @bookmarks.size)
  end

  def test_key
    documents = Groonga::PatriciaTrie.create(:name => "Documents",
                                             :key_type => "ShortText")
    reference = documents.add("reference")
    assert_equal("reference", reference.key)
  end

  def test_value
    bookmark = @bookmarks.add
    assert_equal("", bookmark.value.split(/\0/, 2)[0])
    bookmark.value = "http://groonga.org/\0"
    assert_equal("http://groonga.org/", bookmark.value.split(/\0/, 2)[0])
  end

  def test_reference_column?
    bookmark = @bookmarks.add
    assert_false(bookmark.reference_column?("uri"))
    assert_true(bookmark.reference_column?("user"))
  end

  def test_score
    groonga = @bookmarks.add
    groonga["content"] = "full text search search search engine."

    google = @bookmarks.add
    google["content"] = "Web search engine."

    results = @bookmarks_content_index.search("search")
    assert_equal([[groonga.id, 3], [google.id, 1]],
                 results.collect do |record|
                   [record.id, record.score]
                 end)
  end

  def test_increment!
    groonga = @bookmarks.add
    assert_equal(0, groonga["rate"])
    groonga.increment!("rate")
    assert_equal(1, groonga["rate"])
    groonga.increment!("rate", 2)
    assert_equal(3, groonga["rate"])
    groonga.increment!("rate", -2)
    assert_equal(1, groonga["rate"])
  end

  def test_decrement!
    groonga = @bookmarks.add
    assert_equal(0, groonga["rate"])
    groonga.decrement!("rate")
    assert_equal(-1, groonga["rate"])
    groonga.decrement!("rate", 2)
    assert_equal(-3, groonga["rate"])
    groonga.decrement!("rate", -2)
    assert_equal(-1, groonga["rate"])
  end

  def test_lock
    groonga = @bookmarks.add

    assert_not_predicate(groonga, :locked?)
    groonga.lock
    assert_predicate(groonga, :locked?)
    groonga.unlock
    assert_not_predicate(groonga, :locked?)
  end

  def test_lock_failed
    groonga = @bookmarks.add

    groonga.lock
    assert_raise(Groonga::ResourceDeadlockAvoided) do
      groonga.lock
    end
  end

  def test_lock_block
    groonga = @bookmarks.add

    assert_not_predicate(groonga, :locked?)
    groonga.lock do
      assert_predicate(groonga, :locked?)
    end
    assert_not_predicate(groonga, :locked?)
  end

  def test_clear_lock
    groonga = @bookmarks.add

    assert_not_predicate(groonga, :locked?)
    groonga.lock
    assert_predicate(groonga, :locked?)
    groonga.clear_lock
    assert_not_predicate(groonga, :locked?)
  end

  def test_attributes
    values = {
      "uri" => "http://groonga.org/",
      "rate" => 5,
      "comment" => "Grate!"
    }
    groonga = @bookmarks.add(values)
    assert_equal(values.merge("id" => groonga.id,
                              "content" => nil,
                              "user" => nil),
                 groonga.attributes)
  end
end
