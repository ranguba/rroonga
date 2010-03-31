# Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>
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

class ViewTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_inspect
    path = @tables_dir + "users.groonga"
    view = Groonga::View.create(:name => "Users", :path => path.to_s)
    assert_equal("#<Groonga::View " +
                 "id: <#{view.id}>, " +
                 "name: <Users>, " +
                 "path: <#{path}>, " +
                 "domain: (nil), " +
                 "range: (nil), " +
                 "flags: <>, " +
                 "encoding: <:default>, " +
                 "size: <0>>",
                 view.inspect)
  end

  def test_encoding
    view = Groonga::View.create
    assert_false(view.respond_to?(:encoding))
  end

  def test_add_table
    users = Groonga::Array.create(:name => "Users")
    dogs = Groonga::Array.create(:name => "Dogs")
    entries = Groonga::View.create(:name => "Entries")
    entries.add_table(users)
    entries.add_table(dogs)

    assert_equal(0, entries.size)
    users.add
    assert_equal(1, entries.size)
    dogs.add
    assert_equal(2, entries.size)
  end
end
