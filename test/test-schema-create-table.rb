# Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>
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

class SchemaCreateTableArrayTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_normal
    assert_nil(context["Posts"])
    Groonga::Schema.create_table("Posts") do |table|
    end
    assert_not_nil(context["Posts"])
  end

  def test_force
    Groonga::Schema.create_table("Posts") do |table|
      table.string("name")
    end
    assert_not_nil(context["Posts.name"])

    Groonga::Schema.create_table("Posts",
                                 :type => :hash,
                                 :force => true) do |table|
    end
    assert_nil(context["Posts.name"])
  end

  def test_different_type
    Groonga::Schema.create_table("Posts") do |table|
    end

    assert_raise(Groonga::Schema::TableCreationWithDifferentOptions) do
      Groonga::Schema.create_table("Posts", :type => :hash) do |table|
      end
    end
  end

  def test_with_same_options
    Groonga::Schema.create_table("Posts") do |table|
    end

    Groonga::Schema.create_table("Posts") do |table|
    end
    assert_not_nil(context["Posts"])
  end

  def test_with_same_value_type
    Groonga::Schema.create_table("Posts", :value_type => "UInt32") do |table|
    end

    Groonga::Schema.create_table("Posts", :value_type => "UInt32") do |table|
    end
    assert_equal(context["UInt32"], context["Posts"].range)
  end

  def test_with_differnt_value_type
    Groonga::Schema.create_table("Posts") do |table|
    end

    assert_raise(Groonga::Schema::TableCreationWithDifferentOptions) do
      Groonga::Schema.create_table("Posts", :value_type => "Int32") do |table|
      end
    end
  end
end
