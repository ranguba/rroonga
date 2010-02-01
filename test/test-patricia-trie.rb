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
    trie.default_tokenizer = "TokenTrigram"
    assert_equal(Groonga::Context.default["TokenTrigram"],
                 trie.default_tokenizer)
  end

  def test_search
    users = Groonga::Array.create(:name => "Users")
    user_name = users.define_column("name", "ShortText")

    bookmarks = Groonga::PatriciaTrie.create(:name => "Bookmarks",
                                             :key_type => "ShortText")
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
    users = Groonga::PatriciaTrie.create(:name => "Users")
    users.define_column("address", "<text>")
    me = users.add("me", :address => "me@example.com")
    assert_equal("me@example.com", me[:address])
  end

  def test_default_tokenizer_on_create
    terms = Groonga::PatriciaTrie.create(:name => "Terms",
                                         :default_tokenizer => "TokenUnigram")
    assert_equal(context[Groonga::Type::UNIGRAM],
                 terms.default_tokenizer)
  end

  def test_duplicated_name
    Groonga::PatriciaTrie.create(:name => "Users")
    assert_raise(Groonga::InvalidArgument) do
      Groonga::PatriciaTrie.create(:name => "Users")
    end
  end

  def test_open_same_name
    users_created = Groonga::PatriciaTrie.create(:name => "Users")
    users_opened = Groonga::PatriciaTrie.open(:name => "Users")
    users_opened.add("morita")
    assert_equal(1, users_created.size)
  end

  def test_has_key?
    users = Groonga::PatriciaTrie.create(:name => "Users")
    assert_false(users.has_key?("morita"))
    users.add("morita")
    assert_true(users.has_key?("morita"))
  end

  def test_scan
    Groonga::Context.default_options = {:encoding => "utf-8"}
    words = Groonga::PatriciaTrie.create(:key_type => "ShortText",
                                         :key_normalize => true)
    words.add("リンク")
    arupaka = words.add("アルパカ")
    words.add("アルパカ(生物)")
    adventure_of_link = words.add('リンクの冒険')
    words.add('冒険')
    gaxtu = words.add('ｶﾞｯ')
    muteki = words.add('ＭＵＴＥＫＩ')
    assert_equal([[muteki, "muTEki", 0, 6],
                  [adventure_of_link, "リンクの冒険", 7, 18],
                  [arupaka, "アルパカ", 42, 12],
                  [gaxtu, "ガッ", 55, 6]],
                 words.scan('muTEki リンクの冒険 ミリバール アルパカ ガッ'))
  end

  def test_tag_keys
    Groonga::Context.default_options = {:encoding => "utf-8"}
    words = Groonga::PatriciaTrie.create(:key_type => "ShortText",
                                         :key_normalize => true)
    words.add('リンクの冒険')
    words.add('冒険')
    words.add('㍊')
    words.add('ｶﾞｯ')
    words.add('ＭＵＴＥＫＩ')

    text = 'muTEki リンク リンクの冒険 マッチしない ミリバール ガッ おわり'
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

    text = 'muTEki リンクの冒険'
    actual = words.tag_keys(text) do |record, word|
      "<#{word}(#{record.key})>"
    end
    assert_equal(text, actual)
  end

  def test_prefix_search
    paths = Groonga::PatriciaTrie.create(:name => "Paths",
                                         :key_type => 'ShortText')
    root_path = paths.add('/')
    tmp_path = paths.add('/tmp')
    usr_bin_path = paths.add('/usr/bin')
    usr_local_bin_path = paths.add('/usr/local/bin')

    records = paths.prefix_search('/')
    assert_equal(["/usr/local/bin", "/usr/bin", "/tmp", "/"],
                 records.records.collect {|record| record["._key"]})

    records = paths.prefix_search('/usr')
    assert_equal(["/usr/local/bin", "/usr/bin"],
                 records.records.collect {|record| record["._key"]})

    records = paths.prefix_search('/usr/local')
    assert_equal(["/usr/local/bin"],
                 records.records.collect {|record| record["._key"]})

    records = paths.prefix_search('nonexistent')
    assert_equal([],
                 records.records.collect {|record| record["._key"]})
  end
end
