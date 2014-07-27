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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

class VectorColumnTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
    setup_tables
  end

  def setup_tables
    Groonga::Schema.define do |schema|
      schema.create_table("Users",
                          :type => :hash,
                          :key_type => "UInt32") do |table|
      end

      schema.create_table("Communities",
                          :type => :hash,
                          :key_type => "ShortText") do |table|
        table.reference("users", "Users", :type => :vector)
      end

      schema.create_table("Areas",
                          :type => :hash,
                          :key_type => "ShortText") do |table|
        table.reference("communities", "Communities", :type => :vector)
      end
    end

    @users = Groonga::Context.default["Users"]
    @communities = Groonga::Context.default["Communities"]
    @areas = Groonga::Context.default["Areas"]
  end

  def test_set_records
    groonga = @communities.add("groonga")
    morita = @users.add(29)
    groonga["users"] = [morita]

    assert_equal([29], @users.collect {|record| record.key})
    assert_equal([29], groonga["users"].collect {|record| record.key})
  end

  def test_set_nonexistent_keys
    shinjuku = @areas.add("Shinjuku")
    shinjuku["communities"] = ["groonga", "Senna"]

    assert_equal(["groonga", "Senna"],
                 @communities.collect {|record| record.key})
    assert_equal(["groonga", "Senna"],
                 shinjuku["communities"].collect {|record| record.key})
  end

  def test_set_nil
    groonga = @communities.add("groonga")
    assert_equal([], groonga["users"])
    groonga["users"] = nil
    assert_equal([], groonga["users"]) # should return nil?
                                       # Can groonga tell us
                                       # that the value is empty?
  end
end
