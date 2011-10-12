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
    database.close

    assert_predicate(database, :closed?)
    called = false
    Groonga::Database.open(db_path.to_s) do |_database|
      database = _database
      assert_not_predicate(database, :closed?)
      called = true
    end
    assert_true(called)
    assert_predicate(database, :closed?)
  end

  def test_close
    db_path = @tmp_dir + "db"
    database = Groonga::Database.create(:path => db_path.to_s)
    database.close

    database = Groonga::Database.open(db_path.to_s)
    assert_not_predicate(database, :closed?)
    database.close
    assert_predicate(database, :closed?)
  end

  def test_new
    db_path = @tmp_dir + "db"
    assert_raise(Groonga::NoSuchFileOrDirectory) do
      Groonga::Database.new(db_path.to_s)
    end

    Groonga::Database.create(:path => db_path.to_s)
    assert_not_predicate(Groonga::Database.new(db_path.to_s), :closed?)
  end

  def test_each
    db_path = @tmp_dir + "db"
    database = Groonga::Database.create(:path => db_path.to_s)
    default_object_names = database.collect {|object| object.name}.sort
    assert_send([default_object_names, :include?, "Bool"])
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

  def test_touch
    database = Groonga::Database.create
    assert_nothing_raised do
      database.touch
    end
  end

  def test_defrag
    setup_database
    Groonga::Schema.define do |schema|
      schema.create_table("Users") do |table|
        table.short_text("name")
        table.short_text("address")
      end
    end
    users = context["Users"]
    1000.times do |i|
      users.add(:name => "user #{i}" * 1000,
                :address => "address #{i}" * 1000)
    end
    assert_equal(7, @database.defrag)
  end
end
