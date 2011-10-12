# -*- coding: utf-8 -*-
#
# Copyright (C) 2010-2011  Kouhei Sutou <kou@clear-code.com>
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

class GQTPTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  setup
  def setup_comments
    @users = Groonga::Hash.create(:name => "Users", :key_type => "ShortText")

    @comments = Groonga::Array.create(:name => "Comments")
    @comments.define_column("content", "Text")
    @comments.define_column("created_at", "Time")
    @comments.define_column("user", "Users")

    terms = Groonga::PatriciaTrie.create(:name => "Terms",
                                         :default_tokenizer => "TokenBigram")
    terms.define_index_column("comment_content", @comments,
                              :with_section => true,
                              :source => "content")
    @comment1 = @comments.add(:content => "Hello Good-bye!",
                              :created_at => Time.parse("2009-08-09"),
                              :user => "morita")
    @comment2 = @comments.add(:content => "Hello World",
                              :created_at => Time.parse("2009-07-09"),
                              :user => "")
    @comment3 = @comments.add(:content => "test",
                              :created_at => Time.parse("2009-06-09"),
                              :user => "gunyara-kun")
    @japanese_comment =
      @comments.add(:content => "うちのボロTVはまだ現役です",
                    :created_at => Time.parse("2009-06-09"),
                    :user => "darashi")
  end

  def test_select_filter_by_existent_user
    assert_equal([[[1], [["user", "Users"]], ["darashi"]]],
                 process("select Comments --output_columns user " +
                         "--filter 'user == \"darashi\"'"))
  end

  def test_select_filter_by_nonexistent_user
    assert_equal([[[0], [["user", "Users"]]]],
                 process("select Comments --output_columns user " +
                         "--filter 'user == \"yu\"'"))
  end

  private
  def process(gqtp)
    context.send(gqtp)
    _, result = context.receive
    JSON.parse(result)
  end
end
