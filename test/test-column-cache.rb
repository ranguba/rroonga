# Copyright (C) 2018  Kouhei Sutou <kou@clear-code.com>
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

class ColumnCacheTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database

    setup_users_table
  end

  def setup_users_table
    Groonga::Schema.define do |schema|
      schema.create_table("Users", :type => :hash) do |table|
        table.integer32("age")
      end
    end

    @users = context["Users"]
    @users.add("alice", :age => 9)
    @users.add("bob",   :age => 19)
    @users.add("chris", :age => 29)

    @age = @users.column("age")
  end

  def test_array_reference
    Groonga::ColumnCache.open(@age) do |column_cache|
      assert_equal(@users.collect(&:age),
                   @users.collect {|user| column_cache[user]})
    end
  end
end
