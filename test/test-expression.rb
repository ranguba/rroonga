# -*- coding: utf-8 -*-
#
# Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>
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

  def test_array_reference
    expression = Groonga::Expression.new
    ryoqun = expression.define_variable({:name => "user"})
    ryoqun.value = "ryoqun"
    mori = expression.define_variable
    mori.value = "mori"

    expression.append_object(ryoqun)
    expression.append_object(mori)

    assert_equal("ryoqun", expression["user"])
    assert_equal("ryoqun", expression[0])
    assert_equal("mori", expression[1])
  end

  def test_get_value
    users = Groonga::Hash.create(:name => "Users")
    name = users.define_column("name", "ShortText")

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
    name = users.define_column("name", "ShortText")

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

    assert_equal("#<Groonga::Expression noname(){21,01,0PLUS}>",
                 expression.inspect)
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
end
