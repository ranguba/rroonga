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

class PatriciaTrieTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_encoding
    assert_equal(Groonga::Encoding.default,
                 Groonga::PatriciaTrie.create.encoding)
  end

  def test_tokenizer
    trie = Groonga::PatriciaTrie.create
    assert_nil(trie.default_tokenizer)
    trie.default_tokenizer = "<token:trigram>"
    assert_equal(Groonga::Context.default["<token:trigram>"],
                 trie.default_tokenizer)
  end

  def test_search
    omit("creating entry is broken.")

    users = Groonga::Array.create(:name => "<users>")
    user_name = users.define_column("name", "<shorttext>")

    bookmarks = Groonga::PatriciaTrie.create(:name => "<bookmarks>",
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
    users = Groonga::PatriciaTrie.create(:name => "<users>")
    users.define_column("address", "<text>")
    me = users.add("me", :address => "me@example.com")
    assert_equal("me@example.com", me[:address])
  end

  def test_default_tokenizer_on_create
    terms = Groonga::PatriciaTrie.create(:name => "<terms>",
                                         :default_tokenizer => "<token:unigram>")
    assert_equal(context[Groonga::Type::UNIGRAM],
                 terms.default_tokenizer)
  end

  def test_duplicated_name
    Groonga::PatriciaTrie.create(:name => "<users>")
    assert_raise(Groonga::InvalidArgument) do
      Groonga::PatriciaTrie.create(:name => "<users>")
    end
  end

  def test_open_same_name
    users_created = Groonga::PatriciaTrie.create(:name => "<users>")
    users_opened = Groonga::PatriciaTrie.open(:name => "<users>")
    users_opened.add("morita")
    assert_equal(1, users_created.size)
  end

  def test_has_key?
    users = Groonga::PatriciaTrie.create(:name => "<users>")
    assert_false(users.has_key?("morita"))
    users.add("morita")
    assert_true(users.has_key?("morita"))
  end

  def test_scan
    Groonga::Context.default_options = {:encoding => "utf-8"}
    words = Groonga::PatriciaTrie.create(:key_type => "ShortText",
                                         :key_normalize => true)
    words.add("リンク")
    adventure_of_link = words.add('リンクの冒険')
    words.add('冒険')
    gaxtu = words.add('ｶﾞｯ')
    muteki = words.add('ＭＵＴＥＫＩ')
    assert_equal([[muteki, "muTEki", 0, 6],
                  [adventure_of_link, "リンクの冒険", 7, 18],
                  [gaxtu, "ガッ", 42, 6]],
                 words.scan('muTEki リンクの冒険 ミリバール ガッ'))
  end

  def test_tag_keys
    Groonga::Context.default_options = {:encoding => "utf-8"}
    words = Groonga::PatriciaTrie.create(:key_type => "ShortText",
                                         :key_normalize => true)
    words.add("リンク")
    words.add('リンクの冒険')
    words.add('冒険')
    words.add('㍊')
    words.add('ｶﾞｯ')
    words.add('ＭＵＴＥＫＩ')

    text = 'muTEki リンクの冒険 マッチしない ミリバール ガッ おわり'
    actual = words.tag_keys(text) do |record, word|
      "<#{word}(#{record.key})>"
    end
    assert_equal("<muTEki(muteki)> " +
                 "<リンクの冒険(リンクの冒険)> " +
                 "マッチしない " +
                 "<ミリバール(ミリバール)> " +
                 "<ガッ(ガッ)> " +
                 "おわり",
                 actual)
  end

  def test_tag_keys_with_last_match
    Groonga::Context.default_options = {:encoding => "utf-8"}
    words = Groonga::PatriciaTrie.create(:key_type => "ShortText",
                                         :key_normalize => true)
    words.add('ｶﾞｯ')
    words.add('ＭＵＴＥＫＩ')

    text = 'muTEki マッチしない ガッ'
    actual = words.tag_keys(text) do |record, word|
      "<#{word}(#{record.key})>"
    end
    assert_equal("<muTEki(muteki)> " +
                 "マッチしない " +
                 "<ガッ(ガッ)>",
                 actual)
  end

  def test_tag_keys_with_no_match
    Groonga::Context.default_options = {:encoding => "utf-8"}
    words = Groonga::PatriciaTrie.create(:key_type => "ShortText",
                                         :key_normalize => true)

    words.add('無敵')
    words.add('ＢＯＵＫＥＮ')

    text = 'muTEki リンクの冒険 マッチしない ミリバール ガッ おわり'
    actual = words.tag_keys(text) do |record, word|
      "<#{word}(#{record.key})>"
    end
    assert_equal(text, actual)
  end
end
