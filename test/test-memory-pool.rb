# Copyright (C) 2013  Kouhei Sutou <kou@clear-code.com>
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

class MemoryPoolTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_sandbox
    setup_database
    setup_schema
    setup_data
  end

  def setup_schema
    Groonga::Schema.define do |schema|
      schema.create_table("Users",
                          :type => :hash,
                          :key_type => :short_text) do |table|
        table.uint8(:age)
        table.short_text(:hobby)
      end
    end
    @users = context["Users"]
  end

  def setup_data
    @users.add("mori",   :age => 46, :hobby => "violin")
    @users.add("s-yata", :age => 28, :hobby => "programming")
    @users.add("kou",    :age => 31, :hobby => "programming")
  end

  def teardown
    teardown_sandbox
  end

  def test_block
    adults = nil
    context.push_memory_pool do
      adults = @users.select do |user|
        user.age >= 20
      end
      assert_true(adults.temporary?)
      assert_false(adults.closed?)
    end
    assert_true(adults.closed?)
  end

  def test_not_block
    context.push_memory_pool
    adults = @users.select do |user|
      user.age >= 20
    end
    assert_true(adults.temporary?)
    assert_false(adults.closed?)
    context.pop_memory_pool
    assert_true(adults.closed?)
  end

  def test_nested
    adults = nil
    context.push_memory_pool do
      adults = @users.select do |user|
        user.age >= 20
      end
      grouped_adults = nil
      context.push_memory_pool do
        grouped_adults = adults.group(["hobby"])
        assert_true(grouped_adults.temporary?)
        assert_false(grouped_adults.closed?)
      end
      assert_true(grouped_adults.closed?)
      assert_false(adults.closed?)
    end
    assert_true(adults.closed?)
  end
end
