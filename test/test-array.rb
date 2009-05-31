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

class ArrayTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

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
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "size: <3>>",
                 contain_table.inspect)
  end

  def test_encoding
    array = Groonga::Array.create
    assert_false(array.respond_to?(:encoding))
  end

  def test_add
    users = Groonga::Array.create(:name => "<users>")
    users.define_column("name", "<text>")
    me = users.add(:name => "me")
    assert_equal("me", me[:name])
  end

  def test_define_index_column
    users = Groonga::Array.create(:name => "<users>")
    users.define_column("name", "<text>")
    bookmarks = Groonga::Array.create(:name => "<bookmarks>")
    bookmarks.define_column("title", "<text>")
    bookmarks.define_column("user", users)

    index = users.define_index_column("bookmarks", bookmarks,
                                      :source => "<bookmarks>.user")
    morita = users.add(:name => "morita")
    gunyara_kun = users.add(:name => "gunyara-kun")
    groonga = bookmarks.add(:title => "groonga", :user => morita)
    google = bookmarks.add(:title => "google", :user => morita)
    python = bookmarks.add(:title => "Python", :user => gunyara_kun)

    assert_equal(["groonga", "google"],
                 index.search(morita.id).collect {|record| record.key["title"]})
  end
end
