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

  def normalize_query_log(log)
    log.lines.collect do |line|
      line.chomp.gsub(/\A.*?:\d+ /, "")
    end
  end

  def test_flush_table
    table = Groonga::Hash.create
    log = collect_query_log do
      table.flush
    end
    assert_equal([
                   "flush[(anonymous:table:hash_key)]",
                 ],
                 normalize_query_log(log))
  end

  def test_flush_column
    table = Groonga::Hash.create(:name => "Users")
    column = table.define_column("name", "ShortText")
    log = collect_query_log do
      column.flush
    end
    assert_equal([
                   "flush[Users.name]",
                 ],
                 normalize_query_log(log))
  end

  def test_flush_database
    table = Groonga::Hash.create(:name => "Users")
    table.define_column("name", "ShortText")
    log = collect_query_log do
      @database.flush
    end
    assert_equal([
                   "flush[Users.name]",
                   "flush[Users]",
                   "flush[(anonymous:table:dat_key)]",
                   "flush[(anonymous:column:var_size)]",
                   "flush[(anonymous:table:hash_key)]",
                   "flush[(anonymous:column:var_size)]",
                   "flush[(DB)]",
                 ],
                 normalize_query_log(log))
  end

  def test_flush_not_recursive
    table = Groonga::Hash.create(:name => "Users")
    table.define_column("name", "ShortText")
    log = collect_query_log do
      table.flush(:recursive => false)
    end
    assert_equal([
                   "flush[Users]",
                 ],
                 normalize_query_log(log))
  end

  def test_flush_dependent
    referenced_table = Groonga::Hash.create(name: "Names")
    table = Groonga::Hash.create(:name => "Users")
    table.define_column("name", referenced_table)
    log = collect_query_log do
      table.flush(:recursive => false, :dependent => true)
    end
    assert_equal([
                   "flush[Names]",
                   "flush[Users.name]",
                   "flush[Users]",
                 ],
                 normalize_query_log(log))
  end
end
