# -*- coding: utf-8 -*-
# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
# Copyright (C) 2009  SHIDARA Yoji <dara@shidara.net>
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

class TableSelectNormalizeTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  setup
  def setup_comments
    @comments = Groonga::Array.create(:name => "Comments")
    @comments.define_column("content", "Text")
    @comments.define_column("created_at", "Time")
    options = {
      :name => "Terms",
      :default_tokenizer => "TokenBigramSplitSymbol",
      :key_normalize => true,
    }
    terms = Groonga::PatriciaTrie.create(options)
    terms.define_index_column("comment_content", @comments,
                              :with_section => true,
                              :with_position => true,
                              :source => "Comments.content")
    @japanese_comment =
      @comments.add(:content => "うちのボロTV（アナログ...）はまだ現役です",
                    :created_at => Time.parse("2009-06-09"))
  end

  def test_select_query_with_japanese
    result = @comments.select("content:@ボロTV")
    assert_equal_select_result([@japanese_comment], result)
  end

  def test_select_query_with_japanese_parenthesis
    result = @comments.select("content:@）は")
    assert_equal_select_result([@japanese_comment], result)
  end

  def test_select_query_only_in_japanese
    result = @comments.select("content:@うちの")
    assert_equal_select_result([@japanese_comment], result)
  end
end
