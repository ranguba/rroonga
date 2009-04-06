# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

class TableTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    Groonga::Context.default = nil

    @db_path = @tmp_dir + "db"
    @database = Groonga::Database.create(:path => @db_path.to_s)

    @tables_dir = @tmp_dir + "tables"
    FileUtils.mkdir_p(@tables_dir.to_s)
  end

  def test_create
    table_path = @tables_dir + "table"
    assert_not_predicate(table_path, :exist?)
    table = Groonga::Table.create(:name => "bookmarks",
                                  :path => table_path.to_s)
    assert_equal("bookmarks", table.name)
    assert_predicate(table_path, :exist?)
  end

  def test_temporary
    table = Groonga::Table.create
    assert_nil(table.name)
    assert_equal([], @tables_dir.children)
  end

  def test_open
    table_path = @tables_dir + "table"
    table = Groonga::Table.create(:name => "bookmarks",
                                  :path => table_path.to_s)
    assert_equal("bookmarks", table.name)
    table.close

    called = false
    Groonga::Table.open(:name => "bookmarks") do |_table|
      table = _table
      assert_not_predicate(table, :closed?)
      assert_equal("bookmarks", _table.name)
      called = true
    end
    assert_true(called)
    assert_predicate(table, :closed?)
  end

  def test_open_by_path
    table_path = @tables_dir + "table"
    table = Groonga::Table.create(:name => "bookmarks",
                                  :path => table_path.to_s)
    assert_equal("bookmarks", table.name)
    table.close

    called = false
    Groonga::Table.open(:path => table_path.to_s) do |_table|
      table = _table
      assert_not_predicate(table, :closed?)
      assert_nil(_table.name)
      called = true
    end
    assert_true(called)
    assert_predicate(table, :closed?)
  end

  def test_open_override_name
    table_path = @tables_dir + "table"
    table = Groonga::Table.create(:name => "bookmarks",
                                  :path => table_path.to_s)
    assert_equal("bookmarks", table.name)
    table.close

    called = false
    Groonga::Table.open(:name => "no-name", :path => table_path.to_s) do |_table|
      table = _table
      assert_not_predicate(table, :closed?)
      assert_equal("no-name", _table.name)
      called = true
    end
    assert_true(called)
    assert_predicate(table, :closed?)
  end

  def test_new
    table_path = @tables_dir + "table"
    assert_raise(Groonga::NoSuchFileOrDirectory) do
      Groonga::Table.new(:path => table_path.to_s)
    end

    Groonga::Table.create(:path => table_path.to_s)
    assert_not_predicate(Groonga::Table.new(:path => table_path.to_s), :closed?)
  end

  def test_define_column
    table_path = @tables_dir + "table"
    table = Groonga::Table.create(:name => "bookmarks",
                                  :path => table_path.to_s)
    column = table.define_column("name", "<text>",
                                 :type => "index",
                                 :compress => "zlib",
                                 :with_section => true,
                                 :with_weight => true,
                                 :with_position => true)
    assert_equal("bookmarks.name", column.name)
    assert_equal(column, table.column("name"))
  end

  def test_column
    table_path = @tables_dir + "table"
    table = Groonga::Table.create(:name => "bookmarks",
                                  :path => table_path.to_s)
    assert_nil(table.column("nonexistent"))
  end
end
