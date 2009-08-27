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
    terms = Groonga::PatriciaTrie.create(:name => "terms")
    terms.define_index_column("comment_content", @comments,
                              :with_section => true,
                              :source => "comments.content")
    @comment1 = @comments.add(:content => "Hello Good-bye!",
                              :created_at => Time.parse("2009-08-09"))
    @comment2 = @comments.add(:content => "Hello World",
                              :created_at => Time.parse("2009-07-09"))
    @comment3 = @comments.add(:content => "test",
                              :created_at => Time.parse("2009-06-09"))
  end

  def test_select_sub_expression
    result = @comments.select do |record|
      record.match("Hello", "content") &
        (record["created_at"] < Time.parse("2009-08-01"))
    end
    assert_equal([@comment2],
                 result.collect {|record| record.key})
  end

  def test_select_query
    result = @comments.select("content:%Hello")
    assert_equal([@comment1, @comment2], result.collect {|record| record.key})
  end
  
  priority :must
  def test_select_query_with_block
    result = @comments.select("content:%Hello") do |record|
      record["created_at"] < Time.parse("2009-08-01")
    end
    assert_equal([@comment2], result.collect {|record| record.key})
  end

  def test_select_without_block
    assert_equal([@comment1, @comment2, @comment3],
                 @comments.select.collect {|record| record.key})
  end
end
