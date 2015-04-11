# -*- coding: utf-8 -*-
#
# Copyright (C) 2009-2013  Kouhei Sutou <kou@clear-code.com>
# Copyright (C) 2014  Masafumi Yokoyama <myokoym@gmail.com>
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

class ExpressionTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_array_reference
    expression = Groonga::Expression.new
    ryoqun = expression.define_variable({:name => "user"})
    ryoqun.value = "ryoqun"
    mori = expression.define_variable
    mori.value = "mori"

    expression.append_object(ryoqun)
    expression.append_object(mori)

    assert_equal("ryoqun", expression["user"].value)
    assert_equal("ryoqun", expression[0].value)
    assert_equal("mori", expression[1].value)
  end

  def test_get_value
    users = Groonga::Hash.create(:name => "Users")
    users.define_column("name", "ShortText")

    morita = users.add("morita", :name => "mori daijiro")

    expression = Groonga::Expression.new
    expression.append_constant(morita)
    expression.append_constant("name")
    expression.append_operation(Groonga::Operation::GET_VALUE, 2)
    expression.compile
    assert_equal("mori daijiro", expression.execute)
  end

  def test_get_value_with_variable
    users = Groonga::Hash.create(:name => "Users")
    users.define_column("name", "ShortText")

    morita = users.add("morita", :name => "mori daijiro")
    gunyara_kun = users.add("gunyara-kun", :name => "Tasuku SUENAGA")

    expression = Groonga::Expression.new
    variable = expression.define_variable
    variable.value = morita
    expression.append_object(variable)
    expression.append_constant("name")
    expression.append_operation(Groonga::Operation::GET_VALUE, 2)
    expression.compile
    assert_equal("mori daijiro", expression.execute)

    variable.value = gunyara_kun.id
    assert_equal("Tasuku SUENAGA", expression.execute)
  end

  def test_inspect
    expression = Groonga::Expression.new
    expression.append_constant(1)
    expression.append_constant(1)
    expression.append_operation(Groonga::Operation::PLUS, 2)
    expression.compile

    assert_equal(<<-INSPECTED.chomp, expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:2, value:1>,
    1:<push n_args:1, flags:0, modify:0, value:1>,
    2:<plus n_args:2, flags:0, modify:0, value:(NULL)>
  }>
    INSPECTED
  end

  def test_snippet
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name", "ShortText")
    Groonga::Hash.create(:name => "Terms",
                         :key_type => "ShortText",
                         :default_tokenizer => "TokenBigram")
    users.define_index_column("user_name", users,
                              :source => "Users.name",
                              :with_position => true)

    expression = Groonga::Expression.new
    variable = expression.define_variable(:domain => users)
    expression.append_object(variable)
    expression.parse("ラングバ OR Ruby OR groonga", :default_column => name)
    expression.compile

    snippet = expression.snippet([["[[", "]]"], ["<", ">"]],
                                 :width => 30)
    assert_equal(["[[ラングバ]]プロジェクト",
                  "ン[[groonga]]の機能を<Ruby>か",
                  "。[[groonga]]の機能を<Ruby>ら"],
                 snippet.execute("ラングバプロジェクトはカラムストア機能も" +
                                 "備える高速・高機能な全文検索エンジンgroonga" +
                                 "の機能をRubyから利用するためのライブラリを" +
                                 "提供するプロジェクトです。groongaの機能を" +
                                 "Rubyらしい読み書きしやすい構文で利用できる" +
                                 "ことが利点です。"))
    snippet.close
  end

  def test_snippet_without_tags
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name", "ShortText")
    Groonga::Hash.create(:name => "Terms",
                         :key_type => "ShortText",
                         :default_tokenizer => "TokenBigram")
    users.define_index_column("user_name", users,
                              :source => "Users.name",
                              :with_position => true)

    expression = Groonga::Expression.new
    variable = expression.define_variable(:domain => users)
    expression.append_object(variable)
    expression.parse("ラングバ", :default_column => name)
    expression.compile

    snippet = expression.snippet([], :width => 30)
    assert_equal(["ラングバプロジェクト"],
                 snippet.execute("ラングバプロジェクトはカラムストア機能も"))
    snippet.close
  end

  def test_keywords
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name", "ShortText")

    expression = Groonga::Expression.new
    variable = expression.define_variable(:domain => users)
    expression.append_object(variable)
    expression.parse("ラングバ OR Ruby OR groonga", :default_column => name)
    expression.compile

    assert_equal([
                   "Ruby",
                   "groonga",
                   "ラングバ",
                 ],
                 expression.keywords.sort)
  end

  class AppendObjectTest < self
    setup
    def setup_expression
      @expression = Groonga::Expression.new
    end

    class OperatorTest < self
      def test_constant
        @expression.append_object(Groonga["TokenBigram"],
                                  Groonga::Operator::PUSH,
                                  1)
        assert_equal(<<-INSPECTED.chomp, @expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:0, value:#<proc:tokenizer TokenBigram arguments:[$1, $2, $3]>>
  }>
        INSPECTED
      end

      def test_name
        @expression.append_object(Groonga["TokenBigram"], "push", 1)
        assert_equal(<<-INSPECTED.chomp, @expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:0, value:#<proc:tokenizer TokenBigram arguments:[$1, $2, $3]>>
  }>
        INSPECTED
      end
    end
  end

  class AppendConstantTest < self
    setup
    def setup_expression
      @expression = Groonga::Expression.new
    end

    class OperatorTest < self
      def test_constant
        @expression.append_constant(29, Groonga::Operator::PUSH, 1)
        assert_equal(<<-INSPECTED.chomp, @expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:0, value:29>
  }>
        INSPECTED
      end

      def test_name
        @expression.append_constant(29, "push", 1)
        assert_equal(<<-INSPECTED.chomp, @expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:0, value:29>
  }>
        INSPECTED
      end
    end
  end

  class AppendOperatorTest < self
    class PlusTest < self
      setup
      def setup_expression
        @expression = Groonga::Expression.new
        @expression.append_constant(29)
        @expression.append_constant(92)
      end

      def test_constant
        @expression.append_operation(Groonga::Operator::PLUS, 2)
        assert_equal(<<-INSPECTED.chomp, @expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:2, value:29>,
    1:<push n_args:1, flags:0, modify:0, value:92>,
    2:<plus n_args:2, flags:0, modify:0, value:(NULL)>
  }>
        INSPECTED
      end

      def test_name
        @expression.append_operation("plus", 2)
        assert_equal(<<-INSPECTED.chomp, @expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:2, value:29>,
    1:<push n_args:1, flags:0, modify:0, value:92>,
    2:<plus n_args:2, flags:0, modify:0, value:(NULL)>
  }>
        INSPECTED
      end
    end

    class RegexpTest < self
      setup
      def setup_expression
        @expression = Groonga::Expression.new
        @expression.append_constant("Alice")
        @expression.append_constant(/\A[aA].*l/.source)
      end

      def test_constant
        @expression.append_operation(Groonga::Operator::REGEXP, 2)
        assert_equal(<<-INSPECTED.chomp, @expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:2, value:"Alice">,
    1:<push n_args:1, flags:0, modify:0, value:"\\\\A[aA].*l">,
    2:<regexp n_args:2, flags:0, modify:0, value:(NULL)>
  }>
        INSPECTED
      end

      def test_name
        @expression.append_operation("regexp", 2)
        assert_equal(<<-INSPECTED.chomp, @expression.inspect)
#<Groonga::Expression
  vars:{
  },
  codes:{
    0:<push n_args:1, flags:0, modify:2, value:"Alice">,
    1:<push n_args:1, flags:0, modify:0, value:"\\\\A[aA].*l">,
    2:<regexp n_args:2, flags:0, modify:0, value:(NULL)>
  }>
        INSPECTED
      end
    end
  end

  class VariableTest < self
    def test_reference
      expression = Groonga::Expression.new
      variable = expression.define_variable(:name => "$condition",
                                            :reference => true)
      variable.value = "TODO: Change me to expression"
      assert_equal("TODO: Change me to expression", variable.value)
    end
  end

  class ParseTest < self
    setup
    def setup_schema
      Groonga::Schema.define do |schema|
        schema.create_table("Users") do
        end
      end
    end

    setup
    def setup_expression
      @expression = Groonga::Expression.new
      @variable = @expression.define_variable
      @variable.value = Groonga["Users"].add
    end

    class DefaultOperatorTest < self
      def test_nil
        assert_equal("and", parse(nil))
      end

      def test_name
        assert_equal("or", parse("or"))
      end

      def test_name_symbol
        assert_equal("or", parse(:or))
      end

      def test_symbol
        assert_equal("or", parse("||"))
      end

      def test_integer
        assert_equal("adjust", parse(Groonga::Operator::ADJUST))
      end

      private
      def parse(default_operator)
        @expression.parse("_id:1 _id:2", :default_operator => default_operator)
        operator = @expression.inspect[/6:<([a-zA-Z_-]+)/, 1]
        operator
      end
    end

    class DefaultModeTest < self
      def test_nil
        assert_equal("match", parse(nil))
      end

      def test_name
        assert_equal("equal", parse("equal"))
      end

      def test_symbol
        assert_equal("equal", parse(:equal))
      end

      def test_integer
        assert_equal("equal", parse(Groonga::Operator::EQUAL))
      end

      private
      def parse(default_mode)
        @expression.parse("query",
                          :default_column => "_id",
                          :default_mode => default_mode)
        operator = @expression.inspect[/2:<([a-zA-Z_-]+)/, 1]
        operator
      end
    end
  end
end
