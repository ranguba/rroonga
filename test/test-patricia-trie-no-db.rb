# -*- coding: utf-8 -*-
#
# Copyright (C) 2022  Horimoto Yasuhiro <horimoto@clear-code.com>
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

class PatriciaTrieTestNoDB < Test::Unit::TestCase
  include GroongaTestUtils
  include ERB::Util


  def test_support_key?
    assert_predicate(Groonga::PatriciaTrie.create(key_size: 4096,
                                                  key_var_size: true),
                     :support_key?)
  end

  def test_add
    users = Groonga::PatriciaTrie.create(key_size: 4096, key_var_size: true)
    users.define_column("address", "ShortText")
    me = users.add("me", :address => "me@example.com")
    assert_equal("me@example.com", me[:address])
  end

  def test_key?
    users = Groonga::PatriciaTrie.create(key_size: 4096, key_var_size: true)
    assert_false(users.key?("morita"))
    users.add("morita")
    assert_true(users.key?("morita"))
  end

  def test_has_key?
    users = Groonga::PatriciaTrie.create(key_size: 4096, key_var_size: true)
    assert_false(users.has_key?("morita"))
    users.add("morita")
    assert_true(users.has_key?("morita"))
  end

  def test_scan
    Groonga::Context.default_options = {:encoding => "utf-8"}
    words = Groonga::PatriciaTrie.create(key_size: 4096, key_var_size: true)
    words.add("リンク")
    arupaka = words.add("アルパカ")
    words.add("アルパカ(生物)")
    adventure_of_link = words.add('リンクの冒険')
    words.add('冒険')
    gaxtu = words.add('ガッ')
    muteki = words.add('ＭＵＴＥＫＩ')
    assert_equal([[muteki, "ＭＵＴＥＫＩ", 0, 6],
                  [adventure_of_link, "リンクの冒険", 7, 18],
                  [arupaka, "アルパカ", 42, 12],
                  [gaxtu, "ガッ", 55, 6]],
                 words.scan('ＭＵＴＥＫＩ リンクの冒険 ミリバール アルパカ ガッ'))
  end

  def test_added?
    users = Groonga::PatriciaTrie.create(key_size: 4096, key_var_size: true)
    bob = users.add("bob")
    assert_predicate(bob, :added?)
    bob_again = users.add("bob")
    assert_not_predicate(bob_again, :added?)
  end

  def test_rename
    users = Groonga::PatriciaTrie.create(key_size: 4096, key_var_size: true)
    name = users.define_column("name", "ShortText")
    address = users.define_column("address", "ShortText")

    users.rename("People")
    assert_equal(["People", "People.name", "People.address"],
                 [users.name, name.name, address.name])
  end

  def test_each
    users = Groonga::PatriciaTrie.create(key_size: 4096, key_var_size: true)
    users.add("Alice")
    users.add("Bob")
    users.add("Carl")

    user_names = []
    users.each do |user|
      user_names << user.key
    end
    assert_equal(["Alice", "Bob", "Carl"], user_names)
  end

  def test_truncate
    users = Groonga::PatriciaTrie.create(key_size: 4096, key_var_size: true)
    users.add("Alice")
    users.add("Bob")
    users.add("Carl")
    assert_equal(3, users.size)
    assert_nothing_raised do
      users.truncate
    end
    assert_equal(0, users.size)
  end
end
