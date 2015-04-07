# -*- coding: utf-8 -*-
#
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

class OperatorTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  class RegexpTest < self
    setup
    def setup_expression
      @expression = Groonga::Expression.new
      @expression.append_constant(39)
      @expression.append_constant(93)
    end

    def test_constant
      @expression.append_operation(Groonga::Operator::REGEXP, 2)
      assert_equal(<<-END_OF_INSPECT.chomp, @expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:2, value:39>,
    1:<push n_args:1, flags:0, modify:0, value:93>,
    2:<regexp n_args:2, flags:0, modify:0, value:(NULL)>
  }>
      END_OF_INSPECT
    end

    def test_name
      @expression.append_operation("regexp", 2)
      assert_equal(<<-END_OF_INSPECT.chomp, @expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:2, value:39>,
    1:<push n_args:1, flags:0, modify:0, value:93>,
    2:<regexp n_args:2, flags:0, modify:0, value:(NULL)>
  }>
      END_OF_INSPECT
    end
  end
end
