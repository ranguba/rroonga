# Copyright (C) 2009-2019  Sutou Kouhei <kou@clear-code.com>
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

class VariableTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
    @expression = Groonga::Expression.new
    @variable = @expression.define_variable
  end

  def test_value
    assert_nil(@variable.value)
    @variable.value = "morita"
    assert_equal("morita", @variable.value)
  end

  sub_test_case("#bulk?") do
    def test_true
      assert do
        @variable.bulk?
      end
    end

    def test_false
      assert do
        not @expression.bulk?
      end
    end
  end
end
