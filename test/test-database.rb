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
    assert_equal(["<float>",
                  "<int64>",
                  "<int>",
                  "<longtext>",
                  "<proc:disp>",
                  "<proc:init>",
                  "<proc:scan>",
                  "<proc:search>",
                  "<shorttext>",
                  "<text>",
                  "<time>",
                  "<token:bigram>",
                  "<token:delimit>",
                  "<token:mecab>",
                  "<token:trigram>",
                  "<token:unigram>",
                  "<uint64>",
                  "<uint>"],
                 database.collect {|object| object.name}.sort)
  end
end
