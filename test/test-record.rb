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
    @addresses = Groonga::Array.create(:name => "Addresses")
    @addresses.define_column("mail", "ShortText")
  end

  def setup_users_table
    @users = Groonga::Hash.create(:name => "Users",
                                  :key_type => "ShortText")
    @users.define_column("addresses", @addresses, :type => "vector")
  end

  def setup_bookmarks_table
    @bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                       :value_type => "Int32")
    @bookmarks.define_column("uri", "ShortText")
    @bookmarks.define_column("rate", "Int32")
    @bookmarks.define_column("comment", "Text")
    @bookmarks.define_column("content", "LongText")
    @bookmarks.define_column("user", @users)
  end

  def setup_indexes
    @bookmarks_index =  Groonga::PatriciaTrie.create(:name => "BookmarksIndex")
    @bookmarks_index.default_tokenizer = "TokenBigram"

    @bookmarks_content_index =
      @bookmarks_index.define_index_column("content", @bookmarks,
                                           :with_section => true,
                                           :with_weight => true,
                                           :with_position => true,
                                           :source => "Bookmarks.content")
    @bookmarks_index.define_index_column("uri", @bookmarks,
                                         :with_position => true,
                                         :source => "Bookmarks.uri")
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
    daijiro = @users.add("daijiro")
    assert_nil(groonga["user"])
    groonga["user"] = daijiro
    assert_equal(daijiro, groonga["user"])
  end

  def test_set_nil
    groonga = @bookmarks.add(:content => "groonga")
    assert_equal("groonga", groonga["content"])
    groonga["content"] = nil
    assert_nil(groonga["content"])
  end

  def test_set_empty_string
    groonga = @bookmarks.add(:content => "groonga")
    assert_equal("groonga", groonga["content"])
    groonga["content"] = ""
    assert_nil(groonga["content"])
  end

  def test_set_nil_reference
    groonga = @bookmarks.add(:user => "daijiro",
                             :uri => "http://groonga.org/")
    groonga.user = nil
    assert_nil(groonga.user)
  end

  def test_set_empty_key_reference
    groonga = @bookmarks.add(:user => "daijiro",
                             :uri => "http://groonga.org/")
    assert_equal(@users["daijiro"], groonga.user)
    groonga["user"] = ""
    assert_nil(groonga.user)
  end

  def test_set_string_to_integer_column
    groonga = @bookmarks.add(:content => "groonga")
    groonga.rate = "100"
    assert_equal(100, groonga.rate)
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
    assert_equal(0, bookmark.value)
    bookmark.value = 100
    assert_equal(100, bookmark.value)
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

  def test_dynamic_accessor
    groonga = @bookmarks.add
    assert_equal([],
                 ["uri", "uri="] - groonga.methods.collect {|name| name.to_s})
    assert_equal([true, true],
                 [groonga.respond_to?(:uri),
                  groonga.respond_to?(:uri=)])
    groonga.uri = "http://groonga.org/"
    assert_equal("http://groonga.org/", groonga.uri)
  end

  def test_method_chain
    morita = @users.add("morita")
    groonga = @bookmarks.add(:user => morita, :uri => "http://groonga.org")
    assert_equal("morita", groonga.user.key)
  end

  def test_support_sub_records
    morita = @users.add("morita")
    assert_not_predicate(morita, :support_sub_records?)
    users = @users.select {|record| record.key == "morita"}
    assert_predicate(users.to_a[0], :support_sub_records?)
  end
end
