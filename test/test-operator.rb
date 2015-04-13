# -*- coding: utf-8 -*-
#
# Copyright (C) 2015  Masafumi Yokoyama <yokoyama@clear-code.com>
# Copyright (C) 2015  Kouhei Sutou  <kou@clear-code.com>
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

  sub_test_case "equal" do
    sub_test_case "#exec" do
      test "equal" do
        assert_true(Groonga::Operator::EQUAL.exec("hello",
                                                  "hello"))
      end

      test "not equal" do
        assert_false(Groonga::Operator::EQUAL.exec("hello",
                                                   "Hello"))
      end

      test ":context" do
        context = Groonga::Context.new
        assert_true(Groonga::Operator::EQUAL.exec("hello",
                                                  "hello",
                                                  :context => context))
      end
    end
  end

  sub_test_case "not-equal" do
    sub_test_case "#exec" do
      test "not equal" do
        assert_true(Groonga::Operator::NOT_EQUAL.exec("hello",
                                                      "Hello"))
      end

      test "equal" do
        assert_false(Groonga::Operator::NOT_EQUAL.exec("hello",
                                                       "hello"))
      end

      test ":context" do
        context = Groonga::Context.new
        assert_true(Groonga::Operator::NOT_EQUAL.exec("hello",
                                                      "Hello",
                                                      :context => context))
      end
    end
  end
end
