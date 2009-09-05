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

class DatabaseTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_create
    assert_nil(Groonga::Context.default.database)

    db_path = @tmp_dir + "db"
    assert_not_predicate(db_path, :exist?)
    database = Groonga::Database.create(:path => db_path.to_s)
    assert_predicate(db_path, :exist?)
    assert_not_predicate(database, :closed?)

    assert_equal(database, Groonga::Context.default.database)
  end

  def test_temporary
    before_files = @tmp_dir.children
    database = Groonga::Database.create
    assert_nil(database.name)
    assert_equal(before_files, @tmp_dir.children)
  end

  def test_open
    db_path = @tmp_dir + "db"
    database = Groonga::Database.create(:path => db_path.to_s)

    called = false
    Groonga::Database.open(db_path.to_s) do |_database|
      database = _database
      assert_not_predicate(database, :closed?)
      called = true
    end
    assert_true(called)
    assert_predicate(database, :closed?)
  end

  def test_new
    db_path = @tmp_dir + "db"
    assert_raise(Groonga::NoMemoryAvailable) do
      Groonga::Database.new(db_path.to_s)
    end

    database = Groonga::Database.create(:path => db_path.to_s)

    assert_not_predicate(Groonga::Database.new(db_path.to_s), :closed?)
  end

  def test_each
    db_path = @tmp_dir + "db"
    database = Groonga::Database.create(:path => db_path.to_s)
    assert_equal(["Bool",
                  "Float",
                  "Int16",
                  "Int32",
                  "Int64",
                  "Int8",
                  "LongText",
                  "Object",
                  "ShortText",
                  "Text",
                  "Time",
                  "TokenBigram",
                  "TokenDelimit",
                  "TokenMecab",
                  "TokenTrigram",
                  "TokenUnigram",
                  "TokyoGeoPoint",
                  "UInt16",
                  "UInt32",
                  "UInt64",
                  "UInt8",
                  "WGS84GeoPoint",
                  "column_create",
                  "column_list",
                  "define_selector",
                  "expr_missing",
                  "load",
                  "now",
                  "rand",
                  "select",
                  "status",
                  "table_create",
                  "table_list",
                  "view_add"],
                 database.collect {|object| object.name}.sort)
  end

  def test_encoding
    assert_equal(Groonga::Encoding.default,
                 Groonga::Database.create.encoding)
  end

  def test_lock
    database = Groonga::Database.create

    assert_not_predicate(database, :locked?)
    database.lock
    assert_predicate(database, :locked?)
    database.unlock
    assert_not_predicate(database, :locked?)
  end

  def test_lock_failed
    database = Groonga::Database.create

    database.lock
    assert_raise(Groonga::ResourceDeadlockAvoided) do
      database.lock
    end
  end

  def test_lock_block
    database = Groonga::Database.create

    assert_not_predicate(database, :locked?)
    database.lock do
      assert_predicate(database, :locked?)
    end
    assert_not_predicate(database, :locked?)
  end

  def test_clear_lock
    database = Groonga::Database.create

    assert_not_predicate(database, :locked?)
    database.lock
    assert_predicate(database, :locked?)
    database.clear_lock
    assert_not_predicate(database, :locked?)
  end
end
