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

class SchemaTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_create_table
    assert_nil(context["<posts>"])
    Groonga::Schema.create_table("<posts>") do |table|
    end
    assert_not_nil(context["<posts>"])
  end

  def test_integer32_column
    assert_nil(context["<posts>.rate"])
    Groonga::Schema.create_table("<posts>") do |table|
      table.integer32 :rate
    end
    assert_equal(context["<int>"], context["<posts>.rate"].range)
  end

  def test_integer64_column
    assert_nil(context["<posts>.rate"])
    Groonga::Schema.create_table("<posts>") do |table|
      table.integer64 :rate
    end
    assert_equal(context["<int64>"], context["<posts>.rate"].range)
  end

  def test_unsigned_integer32_column
    assert_nil(context["<posts>.n_viewed"])
    Groonga::Schema.create_table("<posts>") do |table|
      table.unsigned_integer32 :n_viewed
    end
    assert_equal(context["<uint>"], context["<posts>.n_viewed"].range)
  end

  def test_unsigned_integer64_column
    assert_nil(context["<posts>.n_viewed"])
    Groonga::Schema.create_table("<posts>") do |table|
      table.unsigned_integer64 :n_viewed
    end
    assert_equal(context["<uint64>"], context["<posts>.n_viewed"].range)
  end

  def test_float_column
    assert_nil(context["<posts>.rate"])
    Groonga::Schema.create_table("<posts>") do |table|
      table.float :rate
    end
    assert_equal(context["<float>"], context["<posts>.rate"].range)
  end

  def test_time_column
    assert_nil(context["<posts>.last_modified"])
    Groonga::Schema.create_table("<posts>") do |table|
      table.time :last_modified
    end
    assert_equal(context["<time>"], context["<posts>.last_modified"].range)
  end

  def test_short_text_column
    assert_nil(context["<posts>.title"])
    Groonga::Schema.create_table("<posts>") do |table|
      table.short_text :title
    end
    assert_equal(context["<shorttext>"], context["<posts>.title"].range)
  end

  def test_text_column
    assert_nil(context["<posts>.comment"])
    Groonga::Schema.create_table("<posts>") do |table|
      table.text :comment
    end
    assert_equal(context["<text>"], context["<posts>.comment"].range)
  end

  def test_long_text_column
    assert_nil(context["<posts>.content"])
    Groonga::Schema.create_table("<posts>") do |table|
      table.long_text :content
    end
    assert_equal(context["<longtext>"], context["<posts>.content"].range)
  end

  def test_index
    assert_nil(context["<terms>.content"])
    Groonga::Schema.create_table("<posts>") do |table|
      table.long_text :content
    end
    Groonga::Schema.create_table("<terms>") do |table|
      table.index :content, "<posts>.content"
    end
    assert_equal([context["<posts>.content"]],
                 context["<terms>.content"].sources)
  end

  def test_dump
    Groonga::Schema.define do |schema|
      schema.define_table("<posts>") do |table|
        table.short_text :title
      end
    end
    assert_equal(<<-EOS, Groonga::Schema.dump)
define_table("<posts>") do |table|
  table.short_text("title")
end
EOS
  end
end
