# -*- coding: utf-8 -*-
#
# Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>
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

  def test_support_key?
    assert_predicate(Groonga::Hash.create(:name => "Users",
                                          :key_type => "ShortText"),
                     :support_key?)
  end

  def test_delete
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "Bookmarks",
                                     :path => bookmarks_path.to_s)

    bookmarks.add("groonga")
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

  def test_value
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "Bookmarks",
                                     :path => bookmarks_path.to_s,
                                     :key_type => "ShortText",
                                     :value_type => "UInt32")
    bookmarks.set_value("http://google.com/", 29)
    assert_equal(29, bookmarks.value("http://google.com/"))
  end

  def test_column_value
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "Bookmarks",
                                     :path => bookmarks_path.to_s,
                                     :key_type => "ShortText")
    bookmarks.define_column("uri", "ShortText")
    bookmarks.set_column_value("google", "uri", "http://google.com/")
    assert_equal("http://google.com/", bookmarks.column_value("google", "uri"))
  end

  def test_array_reference
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :path => bookmarks_path.to_s,
                                     :key_type => "ShortText")
    bookmark = bookmarks.add("http://google.com/")
    assert_equal(bookmark, bookmarks["http://google.com/"])
  end

  def test_inspect_anonymous
    path = @tables_dir + "anoymous.groonga"
    anonymous_table = Groonga::Hash.create(:path => path.to_s)
    assert_equal("#<Groonga::Hash " +
                 "id: <#{anonymous_table.id}>, " +
                 "name: (anonymous), " +
                 "path: <#{path}>, " +
                 "domain: <ShortText>, " +
                 "range: (nil), " +
                 "flags: <>, " +
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
                 "domain: <ShortText>, " +
                 "range: (nil), " +
                 "flags: <>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 anonymous_table.inspect)
  end

  def test_inspect_named
    path = @tables_dir + "named.groonga"
    named_table = Groonga::Hash.create(:name => "Users", :path => path.to_s)
    assert_equal("#<Groonga::Hash " +
                 "id: <#{named_table.id}>, " +
                 "name: <Users>, " +
                 "path: <#{path}>, " +
                 "domain: <ShortText>, " +
                 "range: (nil), " +
                 "flags: <>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 named_table.inspect)
  end

  def test_inspect_temporary
    named_table = Groonga::Hash.create
    assert_equal("#<Groonga::Hash " +
                 "id: <#{named_table.id}>, " +
                 "name: (anonymous), " +
                 "path: (temporary), " +
                 "domain: <ShortText>, " +
                 "range: (nil), " +
                 "flags: <>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 named_table.inspect)
  end

  def test_encoding
    assert_equal(Groonga::Encoding.default,
                 Groonga::Hash.create.encoding)
  end

  def test_tokenizer
    hash = Groonga::Hash.create
    assert_nil(hash.default_tokenizer)
    hash.default_tokenizer = "TokenBigram"
    assert_equal(Groonga::Context.default["TokenBigram"],
                 hash.default_tokenizer)
  end

  def test_search
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name", "ShortText")

    bookmarks = Groonga::Hash.create(:name => "Bookmarks",
                                     :key_type => "ShortText")
    bookmarks.define_column("user_id", users)

    daijiro = users.add
    daijiro["name"] = "daijiro"
    gunyarakun = users.add
    gunyarakun["name"] = "gunyarakun"

    groonga = bookmarks.add("http://groonga.org/")
    groonga["user_id"] = daijiro

    records = bookmarks.search("http://groonga.org/")
    assert_equal(["daijiro"],
                 records.records.collect {|record| record["user_id.name"]})
  end

  def test_add
    users = Groonga::Hash.create(:name => "Users")
    users.define_column("address", "Text")
    me = users.add("me", :address => "me@example.com")
    assert_equal("me@example.com", me[:address])
  end

  def test_default_tokenizer_on_create
    terms = Groonga::Hash.create(:name => "Terms",
                                 :default_tokenizer => "TokenTrigram")
    assert_equal(context[Groonga::Type::TRIGRAM],
                 terms.default_tokenizer)
  end

  def test_duplicated_name
    Groonga::Hash.create(:name => "Users")
    assert_raise(Groonga::InvalidArgument) do
      Groonga::Hash.create(:name => "Users")
    end
  end

  def test_define_index_column_implicit_with_position
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    bookmarks.define_column("comment", "Text")
    terms = Groonga::Hash.create(:name => "Terms",
                                 :default_tokenizer => "TokenBigram")
    index = terms.define_index_column("comment", bookmarks,
                                      :source => "Bookmarks.comment")
    bookmarks.add("groonga", :comment => "search engine by Brazil")
    bookmarks.add("google", :comment => "search engine by Google")
    bookmarks.add("ruby", :comment => "programing language")

    assert_equal(["groonga", "google"],
                 index.search("engine").collect {|record| record.key.key})
  end

  def test_has_key?
    users = Groonga::Hash.create(:name => "Users")
    assert_false(users.has_key?("morita"))
    users.add("morita")
    assert_true(users.has_key?("morita"))
  end

  def test_big_key
    hash = Groonga::Hash.create(:key_type => "UInt64")
    assert_nothing_raised do
      hash.add(1 << 63)
    end
  end

  def test_value_by_key
    users = Groonga::Hash.create(:key_type => "ShortText",
                                 :value_type => "Int32")
    key = "niku"
    users.add(key)
    users.set_value(key, 29)
    assert_equal(29, users.value(key))
  end

  def test_value_by_id
    users = Groonga::Hash.create(:key_type => "ShortText",
                                 :value_type => "Int32")
    user_id = users.add("niku").id
    users.set_value(user_id, 29, :id => true)
    assert_equal(29, users.value(user_id, :id => true))
  end

  def test_id
    users = Groonga::Hash.create(:key_type => "ShortText")

    key = "niku"
    assert_nil(users.id(key))
    user_id = users.add(key).id
    assert_equal(user_id, users.id(key))
  end

  def test_set_multi_values
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    users.define_column("self_introduction", "ShortText")
    users.define_column("age", "UInt32")

    key = "niku"
    niku = users.add(key)
    assert_equal({
                   "_id" => niku.id,
                   "_key" => key,
                   "self_introduction" => nil,
                   "age" => 0,
                 },
                 niku.attributes)
    users[key] = {
      "self_introduction" => "I'm a meet lover.",
      "age" => 29
    }
    assert_equal({
                   "_id" => niku.id,
                   "_key" => key,
                   "self_introduction" => "I'm a meet lover.",
                   "age" => 29,
                 },
                 niku.attributes)
  end

  def test_set_multi_values_for_nonexistent_record
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    users.define_column("self_introduction", "ShortText")
    users.define_column("age", "UInt32")

    key = "niku"
    users[key] = {
      "self_introduction" => "I'm a meet lover.",
      "age" => 29
    }

    assert_equal({
                   "_id" => users[key].id,
                   "_key" => key,
                   "self_introduction" => "I'm a meet lover.",
                   "age" => 29,
                 },
                 users[key].attributes)
  end

  def test_set_multi_values_with_nonexistent_column
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    users.define_column("self_introduction", "ShortText")
    users.define_column("age", "UInt32")

    key = "niku"
    inspected_users = users.inspect.sub(/size: <0>/, "size: <1>")
    message = "no such column: <\"nonexistent\">: <#{inspected_users}>"
    assert_raise(Groonga::NoSuchColumn.new(message)) do
      users[key] = {
        "nonexistent" => "No!",
      }
    end
  end

  def test_added?
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    bob = users.add("bob")
    assert_predicate(bob, :added?)
    bob_again = users.add("bob")
    assert_not_predicate(bob_again, :added?)
  end

  def test_defrag
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    users.define_column("name", "ShortText")
    users.define_column("address", "ShortText")
    1000.times do |i|
      users.add("user #{i}",
                :name => "user #{i}" * 1000,
                :address => "address #{i}" * 1000)
    end
    assert_equal(7, users.defrag)
  end

  def test_rename
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    name = users.define_column("name", "ShortText")
    address = users.define_column("address", "ShortText")

    users.rename("People")
    assert_equal(["People", "People.name", "People.address"],
                 [users.name, name.name, address.name])
  end
end
