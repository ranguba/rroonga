# Copyright (C) 2009-2019  Kouhei Sutou <kou@clear-code.com>
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

  def test_have_column_id
    groonga = @bookmarks.add
    assert_true(groonga.have_column?(:_id))
  end

  def test_have_column_key_hash
    mori = @users.add("mori")
    assert_true(mori.have_column?(:_key))
  end

  def test_have_column_key_array_with_value_type
    groonga = @bookmarks.add
    assert_true(groonga.have_column?(:_key))
  end

  def test_have_column_key_array_without_value_type
    groonga_ml = @addresses.add
    assert_false(groonga_ml.have_column?(:_key))
  end

  def test_have_column_nsubrecs_existent
    @users.add("mori")
    grouped_users = @users.group("_key")
    assert_true(grouped_users.first.have_column?(:_nsubrecs))
  end

  def test_have_column_nsubrecs_nonexistent
    groonga = @bookmarks.add
    assert_false(groonga.have_column?(:_nsubrecs))
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
    bookmark_records = []
    bookmark_records << @bookmarks.add
    bookmark_records << @bookmarks.add
    bookmark_records << @bookmarks.add

    assert_equal(3, @bookmarks.size)
    bookmark_records[1].delete
    assert_equal(2, @bookmarks.size)
  end

  def test_rename
    users = Groonga::DoubleArrayTrie.create(:name => "DATUsers",
                                            :key_type => "ShortText")
    bob = users.add("Bob")

    bob.rename("Alice")
    assert_equal("Alice", bob._key)
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

  def test_index_column?
    bookmark = @bookmarks.add
    assert_false(bookmark.index_column?("uri"))
    index = @bookmarks_index.add("token")
    assert_true(index.index_column?("content"))
  end

  def test_vector_column?
    bookmark = @bookmarks.add
    assert_false(bookmark.vector_column?("uri"))
    morita = @users.add("morita")
    assert_true(morita.vector_column?("addresses"))
  end

  def test_scalar_column?
    bookmark = @bookmarks.add
    assert_true(bookmark.scalar_column?("uri"))
    morita = @users.add("morita")
    assert_false(morita.scalar_column?("addresses"))
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

  def test_score=
    groonga = @bookmarks.add
    groonga["content"] = "full text search search search engine."

    google = @bookmarks.add
    google["content"] = "Web search engine."

    results = @bookmarks_content_index.search("search")
    results.each do |record|
      record.score *= 10
    end
    assert_equal([[groonga.id, 30], [google.id, 10]],
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

  class AttributesTest < self
    def test_basic
      values = {
        "uri" => "http://groonga.org/",
        "rate" => 5,
        "comment" => "Great!"
      }
      groonga = @bookmarks.add(values)
      assert_equal(values.merge("_id" => groonga.id,
                                "_value" => 0,
                                "content" => nil,
                                "user" => nil),
                   groonga.attributes)
    end

    def test_recursive
      need_self_recursive_equal

      @bookmarks.define_column("next", @bookmarks)

      top_page_record = @bookmarks.add(top_page)
      doc_page_record = @bookmarks.add(doc_page)

      top_page_record["next"] = doc_page_record
      doc_page_record["next"] = top_page_record

      expected = {
        "_id" => 1,
        "_value" => 0,
        "user" => nil,
        "uri" => "http://groonga.org/",
        "rate" => 5,
        "next" => {
          "_id" => 2,
          "_value" => 0,
          "user" => nil,
          "uri" => "http://groonga.org/document.html",
          "rate" => 8,
          "content" => nil,
          "comment" => "Informative"
        },
        "content" => nil,
        "comment" => "Great!"
      }
      expected["next"]["next"] = expected

      assert_equal(expected, top_page_record.attributes)
    end

    def test_duplicate_records
      need_self_recursive_equal

      @bookmarks.define_column("next1", @bookmarks)
      @bookmarks.define_column("next2", @bookmarks)

      top_page_record = @bookmarks.add(top_page)
      doc_page_record = @bookmarks.add(doc_page)

      top_page_record["next1"] = doc_page_record
      top_page_record["next2"] = doc_page_record
      doc_page_record["next1"] = top_page_record

      doc_page_attributes = {
        "_id" => 2,
        "_value" => 0,
        "user" => nil,
        "uri" => "http://groonga.org/document.html",
        "rate" => 8,
        "content" => nil,
        "comment" => "Informative",
        "next2" => nil
      }
      top_page_attributes = {
        "_id" => 1,
        "_value" => 0,
        "user" => nil,
        "uri" => "http://groonga.org/",
        "rate" => 5,
        "next1" => doc_page_attributes,
        "next2" => doc_page_attributes,
        "content" => nil,
        "comment" => "Great!"
      }
      doc_page_attributes["next1"] = top_page_attributes

      actual_records = top_page_record.attributes
      assert_equal(top_page_attributes, actual_records)
      assert_equal(actual_records["next1"].object_id,
                   actual_records["next2"].object_id)
    end

    def test_select_result
      @bookmarks.add(top_page)
      select_result = @bookmarks.select
      select_result_result = select_result.first

      expected_attributes = {
        "_id" => 1,
        "_key" => {
          "comment" => "Great!",
          "content" => nil,
          "_id" => 1,
          "_value" => 0,
          "rate" => 5,
          "uri" => "http://groonga.org/",
          "user" => nil
        },
        "_score" => 1,
      }

      assert_equal(expected_attributes, select_result_result.attributes)
    end

    def test_sort_result
      @bookmarks.add(top_page)
      sort_result = @bookmarks.sort(["_id"])
      sort_result_result = sort_result.first

      expected_attributes = {
        "_id" => 1,
        "_value" => {
          "comment" => "Great!",
          "content" => nil,
          "_id" => 1,
          "_value" => 0,
          "rate" => 5,
          "uri" => "http://groonga.org/",
          "user" => nil
        },
      }

      assert_equal(expected_attributes, sort_result_result.attributes)
    end

    def test_self_referencing
      need_self_recursive_equal

      @bookmarks.define_column("next", @bookmarks)

      top_page_record = @bookmarks.add(top_page)
      top_page_record["next"] = top_page_record

      expected = {
        "_id" => 1,
        "_value" => 0,
        "user" => nil,
        "uri" => "http://groonga.org/",
        "rate" => 5,
        "content" => nil,
        "comment" => "Great!"
      }
      expected["next"] = expected

      assert_equal(expected, top_page_record.attributes)
    end

    def test_vector
      @bookmarks.define_column("related_bookmarks", @bookmarks, :type => :vector)

      top_page_record = @bookmarks.add(top_page)
      doc_page_record = @bookmarks.add(doc_page)
      top_page_record["related_bookmarks"] = [doc_page_record]

      expected = {
        "_id" => 1,
        "_value" => 0,
        "user" => nil,
        "uri" => "http://groonga.org/",
        "rate" => 5,
        "related_bookmarks" => [
          {
            "_id" => 2,
            "_value" => 0,
            "comment" => "Informative",
            "content" => nil,
            "rate" => 8,
            "related_bookmarks" => [],
            "uri" => "http://groonga.org/document.html",
            "user" => nil,
          }
        ],
        "content" => nil,
        "comment" => "Great!"
      }

      assert_equal(expected, top_page_record.attributes)
    end

    def test_self_referencing_vector
      need_self_recursive_equal

      @bookmarks.define_column("related_bookmarks", @bookmarks, :type => :vector)

      top_page_record = @bookmarks.add(top_page)
      doc_page_record = @bookmarks.add(doc_page)
      top_page_record["related_bookmarks"] = [
        top_page_record,
        doc_page_record
      ]

      top_page_attributes = {
        "_id" => 1,
        "_value" => 0,
        "user" => nil,
        "uri" => "http://groonga.org/",
        "rate" => 5,
        "content" => nil,
        "comment" => "Great!"
      }
      doc_page_attributes = {
        "_id" => 2,
        "_value" => 0,
        "comment" => "Informative",
        "content" => nil,
        "rate" => 8,
        "related_bookmarks" => [],
        "uri" => "http://groonga.org/document.html",
        "user" => nil,
      }
      top_page_attributes["related_bookmarks"] = [
        top_page_attributes,
        doc_page_attributes
      ]
      assert_equal(top_page_attributes, top_page_record.attributes)
    end
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
    grouped_users = @users.group("_key")
    assert_predicate(grouped_users.first, :support_sub_records?)
  end

  def test_set_record_like_object
    kou_at_clear_code = @addresses.add(:mail => "kou@clear-code.com")
    record_like_class = Class.new do
      attr_reader :record_raw_id
      def initialize(record_raw_id)
        @record_raw_id = record_raw_id
      end
    end
    record_like_object = record_like_class.new(kou_at_clear_code.record_raw_id)
    kou = @users.add("kou")
    kou.addresses = [record_like_object]
  end

  def test_set_array_like_records
    kou_at_clear_code = @addresses.add(:mail => "kou@clear-code.com")
    array_like_class = Class.new do
      def initialize(records)
        @records = records
      end

      def to_ary
        @records
      end
    end
    array_like_object = array_like_class.new([kou_at_clear_code])
    kou = @users.add("kou")
    kou.addresses = array_like_object
  end

  private
  def top_page
    {
      "uri" => "http://groonga.org/",
      "rate" => 5,
      "comment" => "Great!",
    }
  end

  def doc_page
    {
      "uri" => "http://groonga.org/document.html",
      "rate" => 8,
      "comment" => "Informative"
    }
  end

  class JSONTest < self
    def setup
      setup_database
      setup_schema
      setup_tz
    end

    def teardown
      teardown_tz
    end

    def setup_schema
      Groonga::Schema.define do |schema|
        schema.create_table("Bookmarks") do |table|
          table.short_text("uri")
          table.int32("rate")
          table.text("comment")
          table.time("created_at")
        end
      end
      @bookmarks = Groonga["Bookmarks"]
    end

    def setup_tz
      @tz_original = ENV["TZ"]
    end

    def teardown_tz
      ENV["TZ"] = @tz_original
    end

    def test_to_json
      only_not_windows
      ENV["TZ"] = "Japan"
      created_at = Time.parse("2013-05-16T16:57:34+09:00")
      values = {
        "uri"        => "http://groonga.org/",
        "rate"       => 5,
        "comment"    => "Great!",
        "created_at" => created_at,
      }
      groonga = @bookmarks.add(values)
      expected = {
        "_id"        => groonga.id,
        "comment"    => values["comment"],
        "created_at" => created_at.iso8601,
        "rate"       => values["rate"],
        "uri"        => values["uri"],
      }.to_json
      assert_equal(expected, groonga.to_json)
    end

    def test_to_json_with_leap_second
      only_linux
      ENV["TZ"] = "right/Japan"
      created_at = Time.parse("2012-07-01T08:59:60+09:00")
      values = {
        "uri"        => "http://groonga.org/",
        "rate"       => 5,
        "comment"    => "Great!",
        "created_at" => created_at,
      }
      groonga = @bookmarks.add(values)
      expected = {
        "_id"        => groonga.id,
        "comment"    => values["comment"],
        "created_at" => created_at.iso8601,
        "rate"       => values["rate"],
        "uri"        => values["uri"],
      }.to_json
      assert_equal(expected, groonga.to_json)
    end
  end
end
