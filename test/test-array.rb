# Copyright (C) 2009-2018  Kouhei Sutou <kou@clear-code.com>
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

class ArrayTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database, :before => :append

  def test_support_key?
    assert_not_predicate(Groonga::Array.create(:name => "Users"), :support_key?)
  end

  class SupportValueTest < self
    def test_have_value_type
      assert_predicate(Groonga::Array.create(:name => "Users",
                                             :value_type => "Int32"),
                       :support_value?)
    end

    def test_no_value_type
      assert_not_predicate(Groonga::Array.create(:name => "Users"),
                           :support_value?)
    end
  end

  def test_inspect_size
    path = @tables_dir + "named.groonga"
    contain_table = Groonga::Array.create(:name => "name", :path => path.to_s)
    3.times do
      contain_table.add
    end
    assert_equal("#<Groonga::Array " +
                 "id: <#{contain_table.id}>, " +
                 "name: <name>, " +
                 "path: <#{path}>, " +
                 "domain: (nil), " +
                 "range: (nil), " +
                 "flags: <>, " +
                 "size: <3>>",
                 contain_table.inspect)
  end

  def test_encoding
    array = Groonga::Array.create
    assert_false(array.respond_to?(:encoding))
  end

  def test_add
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name", "Text")
    me = users.add(:name => "me")
    assert_equal("me", me[:name])
  end

  def test_define_index_column
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name", "Text")
    bookmarks = Groonga::Array.create(:name => "Bookmarks")
    bookmarks.define_column("title", "Text")
    bookmarks.define_column("user", users)

    index = users.define_index_column("bookmarks", bookmarks,
                                      :source => "Bookmarks.user")
    morita = users.add(:name => "morita")
    gunyara_kun = users.add(:name => "gunyara-kun")
    bookmarks.add(:title => "groonga", :user => morita)
    bookmarks.add(:title => "google", :user => morita)
    bookmarks.add(:title => "Python", :user => gunyara_kun)

    assert_equal(["groonga", "google"],
                 index.search(morita.id).collect {|record| record.key["title"]})
  end

  def test_create_duplicated_name
    Groonga::Array.create(:name => "Users")
    assert_raise(Groonga::InvalidArgument) do
      Groonga::Array.create(:name => "Users")
    end
  end

  def test_value
    users = Groonga::Array.create(:value_type => "Int32")
    user_id = users.add.id
    users.set_value(user_id, 29)
    assert_equal(29, users.value(user_id))
  end

  def test_column_value
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name", "ShortText")
    morita_id = users.add.id
    users.set_column_value(morita_id, "name", "morita")
    assert_equal("morita", users.column_value(morita_id, "name"))
  end

  def test_added?
    users = Groonga::Array.create(:name => "Users")
    first_user = users.add
    assert_predicate(first_user, :added?)
    second_user = users.add
    assert_predicate(second_user, :added?)
  end

  def test_defrag
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name", "ShortText")
    users.define_column("address", "ShortText")
    large_data = "x" * (2 ** 16)
    100.times do |i|
      users.add(:name => "user #{i}" + large_data,
                :address => "address #{i}" + large_data)
    end
    assert_equal(2, users.defrag)
  end

  def test_rename
    users = Groonga::Array.create(:name => "Users")
    name = users.define_column("name", "ShortText")
    address = users.define_column("address", "ShortText")

    users.rename("People")
    assert_equal(["People", "People.name", "People.address"],
                 [users.name, name.name, address.name])
  end

  def test_each
    users = Groonga::Array.create(:name => "Users")
    users.add
    users.add
    users.add

    user_ids = []
    users.each do |user|
      user_ids << user.id
    end
    assert_equal([1, 2, 3], user_ids)
  end

  def test_each_without_block
    users = Groonga::Array.create(:name => "Users")
    users.add
    users.add
    users.add

    user_ids = users.each.collect(&:id)
    assert_equal([1, 2, 3], user_ids)
  end
end
