# -*- coding: utf-8 -*-
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
    value_type = Groonga::Type.new("Text#{value.size}", :size => value.size)
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :path => bookmarks_path.to_s,
                                     :key_type => "<shorttext>",
                                     :value_type => value_type)
    bookmarks["http://google.com/"] = value
    assert_equal(value, bookmarks["http://google.com/"])
  end

  def test_find
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :path => bookmarks_path.to_s,
                                     :key_type => "<shorttext>")
    bookmark = bookmarks.add("http://google.com/")
    assert_equal(bookmark, bookmarks.find("http://google.com/"))
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
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "flags: <>, " +
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
                 "domain: <nil>, " +
                 "range: <nil>, " +
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
    users = Groonga::Array.create(:name => "<users>")
    user_name = users.define_column("name", "<shorttext>")

    bookmarks = Groonga::Hash.create(:name => "<bookmarks>",
                                     :key_type => "<shorttext>")
    bookmark_user_id = bookmarks.define_column("user_id", users)

    daijiro = users.add
    daijiro["name"] = "daijiro"
    gunyarakun = users.add
    gunyarakun["name"] = "gunyarakun"

    groonga = bookmarks.add("http://groonga.org/")
    groonga["user_id"] = daijiro

    records = bookmarks.search("http://groonga.org/")
    assert_equal(["daijiro"],
                 records.records.collect {|record| record[".user_id.name"]})
  end

  def test_add
    users = Groonga::Hash.create(:name => "<users>")
    users.define_column("address", "<text>")
    me = users.add("me", :address => "me@example.com")
    assert_equal("me@example.com", me[:address])
  end

  def test_default_tokenizer_on_create
    terms = Groonga::Hash.create(:name => "<terms>",
                                 :default_tokenizer => "<token:trigram>")
    assert_equal(context[Groonga::Type::TRIGRAM],
                 terms.default_tokenizer)
  end

  def test_duplicated_name
    Groonga::Hash.create(:name => "<users>")
    assert_raise(Groonga::InvalidArgument) do
      Groonga::Hash.create(:name => "<users>")
    end
  end

  def test_define_index_column_implicit_with_position
    bookmarks = Groonga::Hash.create(:name => "<bookmarks>")
    bookmarks.define_column("comment", "<text>")
    terms = Groonga::Hash.create(:name => "<terms>",
                                 :default_tokenizer => "<token:bigram>")
    index = terms.define_index_column("comment", bookmarks,
                                      :source => "<bookmarks>.comment")
    groonga = bookmarks.add("groonga", :comment => "search engine by Brazil")
    google = bookmarks.add("google", :comment => "search engine by Google")
    ruby = bookmarks.add("ruby", :comment => "programing language")

    assert_equal(["groonga", "google"],
                 index.search("engine").collect {|record| record.key.key})
  end

  def test_open_same_name
    users_created = Groonga::Hash.create(:name => "<users>")
    users_opened = Groonga::Hash.open(:name => "<users>")
    users_opened.add("morita")
    assert_equal(1, users_created.size)
  end

  def test_has_key?
    users = Groonga::Hash.create(:name => "<users>")
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
end
