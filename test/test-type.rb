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

  def test_new
    type = Groonga::Type.new("user-id", :type => :integer)
    assert_equal("user-id", type.name)
  end

  def test_builtins
    assert_equal_type("<int>", Groonga::Type::INT)
    assert_equal_type("<uint>", Groonga::Type::UINT)
    assert_equal_type("<int64>", Groonga::Type::INT64)
    assert_equal_type("<float>", Groonga::Type::FLOAT)
    assert_equal_type("<time>", Groonga::Type::TIME)
    assert_equal_type("<shorttext>", Groonga::Type::SHORT_TEXT)
    assert_equal_type("<text>", Groonga::Type::TEXT)
    assert_equal_type("<longtext>", Groonga::Type::LONG_TEXT)
  end

  private
  def assert_equal_type(expected_name, id)
    type = Groonga::Context.default[id]
    assert_equal(expected_name,
                 type ? type.name : type)
  end
end
