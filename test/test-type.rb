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

class TypeTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_new
    type = Groonga::Type.new("user-id", :type => :integer)
    assert_equal("user-id", type.name)
  end

  def test_builtins
    assert_equal_type("Int32", Groonga::Type::INT32)
    assert_equal_type("UInt32", Groonga::Type::UINT32)
    assert_equal_type("Int64", Groonga::Type::INT64)
    assert_equal_type("UInt64", Groonga::Type::UINT64)
    assert_equal_type("Float", Groonga::Type::FLOAT)
    assert_equal_type("Time", Groonga::Type::TIME)
    assert_equal_type("ShortText", Groonga::Type::SHORT_TEXT)
    assert_equal_type("Text", Groonga::Type::TEXT)
    assert_equal_type("LongText", Groonga::Type::LONG_TEXT)
  end

  def test_inspect
    assert_equal("#<Groonga::Type id: <#{Groonga::Type::LONG_TEXT}>, " +
                 "name: <LongText>, " +
                 "path: (temporary), " +
                 "domain: <nil>, " +
                 "range: <2147483648>>",
                 context["<longtext>"].inspect)
  end

  private
  def assert_equal_type(expected_name, id)
    type = Groonga::Context.default[id]
    assert_equal(expected_name,
                 type ? type.name : type)
  end
end
