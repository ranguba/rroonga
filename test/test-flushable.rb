# Copyright (C) 2015  Masafumi Yokoyama <yokoyama@clear-code.com>
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

class FlushableTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_flush_table
    table = Groonga::Hash.create
    assert_nothing_raised do
      table.flush
    end
  end

  def test_flush_column
    table = Groonga::Hash.create(:name => "Users")
    column = table.define_column("name", "ShortText")
    assert_nothing_raised do
      column.flush
    end
  end

  def test_flush_database
    assert_nothing_raised do
      @database.flush
    end
  end

  def test_flush_not_recursive
    table = Groonga::Hash.create
    table.extend(Groonga::Flushable)
    assert_nothing_raised do
      table.flush(:recursive => false)
    end
  end

  def test_flush_dependent
    table = Groonga::Hash.create
    table.extend(Groonga::Flushable)
    assert_nothing_raised do
      table.flush(:recursive => false, :dependent => true)
    end
  end
end
