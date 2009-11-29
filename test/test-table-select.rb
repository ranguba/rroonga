# -*- coding: utf-8 -*-
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

class TableTestSelect < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  setup
  def setup_comments
    @comments = Groonga::Array.create(:name => "comments")
    @comments.define_column("content", "Text")
    @comments.define_column("created_at", "Time")
    terms = Groonga::PatriciaTrie.create(:name => "terms",
                                         :default_tokenizer => "TokenBigram")
    terms.define_index_column("comment_content", @comments,
                              :with_section => true,
                              :source => "comments.content")
    @comment1 = @comments.add(:content => "Hello Good-bye!",
                              :created_at => Time.parse("2009-08-09"))
    @comment2 = @comments.add(:content => "Hello World",
                              :created_at => Time.parse("2009-07-09"))
    @comment3 = @comments.add(:content => "test",
                              :created_at => Time.parse("2009-06-09"))
    @japanese_comment =
      @comments.add(:content => "うちのボロTVはまだ現役です",
                    :created_at => Time.parse("2009-06-09"))
  end

  def test_select_sub_expression
    result = @comments.select do |record|
      record.match("Hello", "content") &
        (record["created_at"] < Time.parse("2009-08-01"))
    end
    assert_equal_select_result([@comment2], result)
  end

  def test_select_query
    result = @comments.select("content:%Hello")
    assert_equal_select_result([@comment1, @comment2], result)
  end

  def test_select_query_with_parser
    result = @comments.select("content @ \"Hello\"", :syntax => :script)
    assert_equal_select_result([@comment1, @comment2], result)
  end

  def test_select_expression
    expression = Groonga::Expression.new
    variable = expression.define_variable(:domain => @comments)
    expression.append_object(variable)
    expression.parse("content:%Hello", :syntax => :query)
    expression.compile
    result = @comments.select(expression)
    assert_equal(expression, result.expression)
    assert_equal_select_result([@comment1, @comment2], result)
  end

  def test_select_query_with_block
    result = @comments.select("content:%Hello") do |record|
      record["created_at"] < Time.parse("2009-08-01")
    end
    assert_equal_select_result([@comment2], result)
  end

  def test_select_query_with_block_match
    result = @comments.select("content:%Hello") do |record|
      record.match("World", "content")
    end
    assert_equal_select_result([@comment2], result)
  end

  def test_select_without_block
    assert_equal_select_result([@comment1, @comment2,
                                @comment3, @japanese_comment],
                               @comments.select)
  end

  def test_select_query_japanese
    result = @comments.select("content:%ボロTV")
    assert_equal_select_result([@japanese_comment], result)
  end

  def test_select_but_query
    result = @comments.select do |record|
      record["content"].match "Hello -World"
    end
    assert_equal_select_result([@comment1], result)
  end

  def test_select_query_with_three_terms
    result = @comments.select do |record|
      record["content"].match "Say Hello World"
    end
    assert_equal_select_result([], result)
  end

  def test_select_query_with_brackets
    result = @comments.select do |record|
      record["content"].match "Say (Hello World)"
    end
    assert_equal_select_result([], result)
  end
end
