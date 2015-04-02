# -*- coding: utf-8 -*-
#
# Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>
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

class TableSelectMecabTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  setup :check_mecab_availability

  setup
  def setup_tables
    Groonga::Schema.define do |schema|
      schema.create_table("Users", :key_type => "ShortText") do |table|
      end

      schema.create_table("Comments", :type => :array) do |table|
        table.short_text("title")
        table.text("content")
        table.time("created_at")
        table.reference("user", "Users")
      end

      schema.create_table("Terms",
                          :type => :patricia_trie,
                          :default_tokenizer => "TokenMecab") do |table|
        table.index("Comments.title", :with_section => true)
        table.index("Comments.content", :with_section => true)
      end
    end
  end

  setup
  def setup_comments
    @comments = context["Comments"]
    @hello = @comments.add(:title => "Hello",
                           :content => "Hello Good-bye!",
                           :created_at => Time.parse("2009-08-09"),
                           :user => "morita")
    @hello_only_in_content =
      @comments.add(:title => "(no title)",
                    :content => "Hello! Hello! Hello!",
                    :created_at => Time.parse("2009-07-09"))
    @test = @comments.add(:title => "title",
                          :content => "test",
                          :created_at => Time.parse("2009-06-09"),
                          :user => "gunyara-kun")
    @japanese =
      @comments.add(:title => "日本語",
                    :content => "うちのボロTVはまだ現役です",
                    :created_at => Time.parse("2009-06-09"),
                    :user => "darashi")
  end

  def test_match
    result = @comments.select do |record|
      record.content =~ "ボロ"
    end
    assert_equal_select_result(["日本語"], result) do |record|
      record.title
    end
  end
end
