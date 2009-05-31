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
end
