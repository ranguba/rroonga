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

class ExpressionTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_get_value
    users = Groonga::Hash.create(:name => "<users>")
    name = users.define_column("name", "<shorttext>")

    morita = users.add("morita", :name => "mori daijiro")

    expression = Groonga::Expression.new
    expression.append_constant(morita)
    expression.append_constant("name")
    expression.append_operation(Groonga::Operation::GET_VALUE, 2)
    expression.compile
    expression.execute
    assert_equal("mori daijiro", context.pop)
  end

  def test_get_value_with_variable
    users = Groonga::Hash.create(:name => "<users>")
    name = users.define_column("name", "<shorttext>")

    morita = users.add("morita", :name => "mori daijiro")
    gunyara_kun = users.add("gunyara-kun", :name => "Tasuku SUENAGA")

    expression = Groonga::Expression.new
    variable = expression.define_variable
    variable.value = morita
    expression.append_object(variable)
    expression.append_constant("name")
    expression.append_operation(Groonga::Operation::GET_VALUE, 2)
    expression.compile
    expression.execute
    assert_equal("mori daijiro", context.pop)

    variable.value = gunyara_kun.id
    expression.execute
    assert_equal("Tasuku SUENAGA", context.pop)
  end
end
