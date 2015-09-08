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

  sub_test_case "#to_s" do
    test "equal" do
      assert_equal("equal", Groonga::Operator::EQUAL.to_s)
    end
  end

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

  sub_test_case "less" do
    sub_test_case "#exec" do
      test "less" do
        assert_true(Groonga::Operator::LESS.exec(1, 2))
      end

      test "greater" do
        assert_false(Groonga::Operator::LESS.exec(2, 1))
      end

      test ":context" do
        context = Groonga::Context.new
        assert_true(Groonga::Operator::LESS.exec(1, 2,
                                                 :context => context))
      end
    end
  end

  sub_test_case "greater" do
    sub_test_case "#exec" do
      test "greater" do
        assert_true(Groonga::Operator::GREATER.exec(2, 1))
      end

      test "less" do
        assert_false(Groonga::Operator::GREATER.exec(1, 2))
      end

      test ":context" do
        context = Groonga::Context.new
        assert_true(Groonga::Operator::GREATER.exec(2, 1,
                                                    :context => context))
      end
    end
  end

  sub_test_case "less-equal" do
    sub_test_case "#exec" do
      test "equal" do
        assert_true(Groonga::Operator::LESS_EQUAL.exec(1, 1))
      end

      test "less" do
        assert_true(Groonga::Operator::LESS_EQUAL.exec(1, 2))
      end

      test "greater" do
        assert_false(Groonga::Operator::LESS_EQUAL.exec(2, 1))
      end

      test ":context" do
        context = Groonga::Context.new
        assert_true(Groonga::Operator::LESS_EQUAL.exec(1, 2,
                                                       :context => context))
      end
    end
  end

  sub_test_case "greater-equal" do
    sub_test_case "#exec" do
      test "equal" do
        assert_true(Groonga::Operator::GREATER_EQUAL.exec(1, 1))
      end

      test "greater" do
        assert_true(Groonga::Operator::GREATER_EQUAL.exec(2, 1))
      end

      test "less" do
        assert_false(Groonga::Operator::GREATER_EQUAL.exec(1, 2))
      end

      test ":context" do
        context = Groonga::Context.new
        assert_true(Groonga::Operator::GREATER_EQUAL.exec(2, 1,
                                                          :context => context))
      end
    end
  end

  sub_test_case "match" do
    sub_test_case "#exec" do
      test "match" do
        assert_true(Groonga::Operator::MATCH.exec("Hello Rroonga",
                                                  "Rroonga"))
      end

      test "not match" do
        assert_false(Groonga::Operator::MATCH.exec("Hello Rroonga",
                                                   "Groonga"))
      end

      test ":context" do
        context = Groonga::Context.new
        assert_true(Groonga::Operator::MATCH.exec("Hello Rroonga",
                                                  "Rroonga",
                                                  :context => context))
      end
    end
  end

  sub_test_case "prefix" do
    sub_test_case "#exec" do
      test "have prefix" do
        assert_true(Groonga::Operator::PREFIX.exec("Hello Rroonga",
                                                   "Hello"))
      end

      test "not have prefix" do
        assert_false(Groonga::Operator::PREFIX.exec("Hello Rroonga",
                                                    "Rroonga"))
      end

      test ":context" do
        context = Groonga::Context.new
        assert_true(Groonga::Operator::PREFIX.exec("Hello Rroonga",
                                                   "Hello",
                                                   :context => context))
      end
    end
  end

  sub_test_case "regexp" do
    sub_test_case "#exec" do
      test "match" do
        assert_true(Groonga::Operator::REGEXP.exec("hello rroonga",
                                                   /rro+nga/))
      end

      test "not match" do
        assert_false(Groonga::Operator::REGEXP.exec("hello rroonga",
                                                    /gro+nga/))
      end

      test ":context" do
        context = Groonga::Context.new
        assert_true(Groonga::Operator::REGEXP.exec("hello rroonga",
                                                   /rro+nga/,
                                                   :context => context))
      end
    end
  end
end
