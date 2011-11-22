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

class PatriciaTrieTest < Test::Unit::TestCase
  include GroongaTestUtils
  include ERB::Util

  setup :setup_database

  def test_support_key?
    assert_predicate(Groonga::PatriciaTrie.create(:name => "Users",
                                                  :key_type => "ShortText"),
                     :support_key?)
  end

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
    users.define_column("name", "ShortText")

    bookmarks = Groonga::PatriciaTrie.create(:name => "Bookmarks",
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
                 records.records.collect {|record| record[".user_id.name"]})
  end

  def test_add
    users = Groonga::PatriciaTrie.create(:name => "Users")
    users.define_column("address", "Text")
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
    words.add('リンク')
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

  def test_tag_keys_other_text_handler
    Groonga::Context.default_options = {:encoding => "utf-8"}
    words = Groonga::PatriciaTrie.create(:key_type => "ShortText",
                                         :key_normalize => true)
    words.add('ｶﾞｯ')
    words.add('ＭＵＴＥＫＩ')

    text = 'muTEki マッチしない <> ガッ'
    other_text_handler = Proc.new do |string|
      h(string)
    end
    options = {:other_text_handler => other_text_handler}
    actual = words.tag_keys(text, options) do |record, word|
      "<span class=\"keyword\">#{h(word)}(#{h(record.key)})</span>\n"
    end
    assert_equal("<span class=\"keyword\">muTEki(muteki)</span>\n" +
                 " マッチしない &lt;&gt; " +
                 "<span class=\"keyword\">ガッ(ガッ)</span>\n",
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
    paths.add('/')
    paths.add('/tmp')
    paths.add('/usr/bin')
    paths.add('/usr/local/bin')

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


  def test_prefix_cursor
    paths = Groonga::PatriciaTrie.create(:name => "Paths",
                                         :key_type => 'ShortText')
    paths.add('/')
    paths.add('/tmp')
    paths.add('/usr/bin')
    paths.add('/usr/local/bin')

    assert_prefix_cursor(["/usr/local/bin", "/usr/bin", "/tmp", "/"],
                         paths, "/", {:order => :desc})
    assert_prefix_cursor(["/", "/tmp", "/usr/bin", "/usr/local/bin"],
                         paths, "/")
    assert_prefix_cursor(["/usr/local/bin", "/usr/bin"],
                         paths, "/usr/local",
                         {:key_bytes => "/usr".size, :order => :desc})
    assert_prefix_cursor(["/tmp", "/usr/bin"],
                         paths, "/",
                         {:offset => 1, :limit => 2})
  end

  def assert_prefix_cursor(expected, tables, prefix, options={})
    actual = []
    tables.open_prefix_cursor(prefix, options) do |cursor|
      cursor.each do |record|
        actual << record.key
      end
    end
    assert_equal(expected, actual)
  end

  def test_rk_cursor
    terms = Groonga::PatriciaTrie.create(:name => "Terms",
                                         :key_type => 'ShortText')
    ["インデックス",
     "エヌグラム",
     "エンジン",
     "カネソナエタ",
     "カノウ",
     "キノウ",
     "キョウカ",
     "クミコミ",
     "クミコム",
     "グルンガ",
     "ケンサク",
     "ケンサクヨウキュウ",
     "ゲンゴ",
     "コウセイド",
     "コウソク",
     "コンパクト",
     "サクセイ",
     "ショリ",
     "ショリケイ",
     "ジッソウ",
     "ジュンスイ",
     "スクリプト",
     "セッケイ",
     "ゼンブン",
     "タイプ",
     "タンゴ",
     "ダイキボ",
     "テンチ",
     "ディービーエムエス",
     "トウ",
     "トクチョウ",
     "ブンショリョウ",
     "ヨウキュウ"].each do |term|
      terms.add(term)
    end

    assert_rk_cursor(["カネソナエタ",
                      "カノウ",
                      "キノウ",
                      "キョウカ",
                      "クミコミ",
                      "クミコム",
                      "ケンサク",
                      "ケンサクヨウキュウ",
                      "コウセイド",
                      "コウソク",
                      "コンパクト"],
                     terms, "k")
  end

  def assert_rk_cursor(expected, tables, prefix, options={})
    actual = []
    tables.open_rk_cursor(prefix, options) do |cursor|
      cursor.each do |record|
        actual << record.key
      end
    end
    assert_equal(expected, actual.sort)
  end

  def test_near_cursor
    points = Groonga::PatriciaTrie.create(:name => "Points",
                                          :key_type => 'WGS84GeoPoint')
    ["130322053x504985073",
     "130285021x504715091",
     "130117012x504390088",
     "130335016x504662007",
     "130308044x504536008",
     "130306053x504530043",
     "130205016x505331054",
     "130222054x505270050",
     "130255017x504266011",
     "130239038x504251015",
     "129885039x503653023",
     "129809022x504597055",
     "130015001x504266057",
     "130089012x505045070",
     "130208017x504315098",
     "130347036x504325073",
     "130380061x505202034",
     "129903045x504648034",
     "130094061x505025099",
     "130133052x505120058",
     "130329069x505188046",
     "130226001x503769013",
     "129866001x504328017",
     "129786048x504792049",
     "129845056x504853081",
     "130055008x504968095",
     "130086003x504480071",
     "129680021x504441006",
     "129855010x504452003",
     "130280013x505208029",
     "129721099x504685024",
     "129690039x504418033",
     "130019020x505027021",
     "130046026x505082073",
     "130038025x505066028",
     "129917001x504675017"].each do |point|
      points.add(point)
    end

    assert_near_cursor(["129680021x504441006",
                        "129690039x504418033",
                        "129721099x504685024",
                        "129786048x504792049",
                        "129809022x504597055",
                        "129845056x504853081",
                        "129855010x504452003",
                        "129866001x504328017",
                        "129885039x503653023",
                        "129903045x504648034"],
                       points,
                       "129786048x504792049",
                       {:limit => 10})
  end

  def assert_near_cursor(expected, tables, prefix, options={})
    actual = []
    tables.open_near_cursor(prefix, options) do |cursor|
      cursor.each do |record|
        actual << record.key
      end
    end
    assert_equal(expected, actual)
  end

  def test_add_uint_key
    numbers = Groonga::PatriciaTrie.create(:name => "Numbers",
                                           :key_type => "UInt32")
    numbers.add(1)
    numbers.add(2)
    numbers.add(5)
    numbers.add(7)
    assert_equal([1, 2, 5, 7], numbers.collect {|number| number.key})
  end

  def test_added?
    users = Groonga::PatriciaTrie.create(:name => "Users",
                                         :key_type => "ShortText")
    bob = users.add("bob")
    assert_predicate(bob, :added?)
    bob_again = users.add("bob")
    assert_not_predicate(bob_again, :added?)
  end

  def test_defrag
    users = Groonga::PatriciaTrie.create(:name => "Users",
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
    users = Groonga::PatriciaTrie.create(:name => "Users",
                                         :key_type => "ShortText")
    name = users.define_column("name", "ShortText")
    address = users.define_column("address", "ShortText")

    users.rename("People")
    assert_equal(["People", "People.name", "People.address"],
                 [users.name, name.name, address.name])
  end
end
