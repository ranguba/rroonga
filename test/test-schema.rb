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

  def test_define_table
    assert_nil(context["<posts>"])
    Groonga::Schema.define_table("<posts>") do |table|
    end
    assert_not_nil(context["<posts>"])
  end

  def test_define_hash
    Groonga::Schema.define_table("<posts>", :type => :hash) do |table|
    end
    assert_kind_of(Groonga::Hash, context["<posts>"])
  end

  def test_define_hash_with_full_option
    path = @tmp_dir + "hash.groonga"
    tokenizer = context["<token:trigram>"]
    Groonga::Schema.define_table("<posts>",
                                 :type => :hash,
                                 :key_type => "integer",
                                 :path => path.to_s,
                                 :value_size => 29,
                                 :default_tokenizer => tokenizer) do |table|
    end
    table = context["<posts>"]
    assert_equal("#<Groonga::Hash " +
                 "id: <#{table.id}>, " +
                 "name: <<posts>>, " +
                 "path: <#{path}>, " +
                 "domain: <#{context['<int>'].inspect}>, " +
                 "range: <nil>, " +
                 "encoding: <:utf8>, " +
                 "size: <0>>",
                 table.inspect)
    assert_equal(tokenizer, table.default_tokenizer)
  end

  def test_define_patricia_trie
    Groonga::Schema.define_table("<posts>", :type => :patricia_trie) do |table|
    end
    assert_kind_of(Groonga::PatriciaTrie, context["<posts>"])
  end

  def test_define_patricia_trie_with_full_option
    path = @tmp_dir + "patricia-trie.groonga"
    Groonga::Schema.define_table("<posts>",
                                 :type => :patricia_trie,
                                 :key_type => "integer",
                                 :path => path.to_s,
                                 :value_size => 29,
                                 :default_tokenizer => "<token:bigram>",
                                 :key_normalize => true,
                                 :key_with_sis => true) do |table|
    end
    table = context["<posts>"]
    assert_equal("#<Groonga::PatriciaTrie " +
                 "id: <#{table.id}>, " +
                 "name: <<posts>>, " +
                 "path: <#{path}>, " +
                 "domain: <#{context['<int>'].inspect}>, " +
                 "range: <nil>, " +
                 "encoding: <:utf8>, " +
                 "size: <0>>",
                 table.inspect)
    assert_equal(context["<token:bigram>"], table.default_tokenizer)
  end

  def test_define_array
    Groonga::Schema.define_table("<posts>", :type => :array) do |table|
    end
    assert_kind_of(Groonga::Array, context["<posts>"])
  end

  def test_define_array_with_full_option
    path = @tmp_dir + "array.groonga"
    Groonga::Schema.define_table("<posts>",
                                 :type => :array,
                                 :path => path.to_s,
                                 :value_size => 29) do |table|
    end
    table = context["<posts>"]
    assert_equal("#<Groonga::Array " +
                 "id: <#{table.id}>, " +
                 "name: <<posts>>, " +
                 "path: <#{path}>, " +
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "size: <0>>",
                 table.inspect)
  end

  def test_integer32_column
    assert_nil(context["<posts>.rate"])
    Groonga::Schema.define_table("<posts>") do |table|
      table.integer32 :rate
    end
    assert_equal(context["<int>"], context["<posts>.rate"].range)
  end

  def test_integer64_column
    assert_nil(context["<posts>.rate"])
    Groonga::Schema.define_table("<posts>") do |table|
      table.integer64 :rate
    end
    assert_equal(context["<int64>"], context["<posts>.rate"].range)
  end

  def test_unsigned_integer32_column
    assert_nil(context["<posts>.n_viewed"])
    Groonga::Schema.define_table("<posts>") do |table|
      table.unsigned_integer32 :n_viewed
    end
    assert_equal(context["<uint>"], context["<posts>.n_viewed"].range)
  end

  def test_unsigned_integer64_column
    assert_nil(context["<posts>.n_viewed"])
    Groonga::Schema.define_table("<posts>") do |table|
      table.unsigned_integer64 :n_viewed
    end
    assert_equal(context["<uint64>"], context["<posts>.n_viewed"].range)
  end

  def test_float_column
    assert_nil(context["<posts>.rate"])
    Groonga::Schema.define_table("<posts>") do |table|
      table.float :rate
    end
    assert_equal(context["<float>"], context["<posts>.rate"].range)
  end

  def test_time_column
    assert_nil(context["<posts>.last_modified"])
    Groonga::Schema.define_table("<posts>") do |table|
      table.time :last_modified
    end
    assert_equal(context["<time>"], context["<posts>.last_modified"].range)
  end

  def test_short_text_column
    assert_nil(context["<posts>.title"])
    Groonga::Schema.define_table("<posts>") do |table|
      table.short_text :title
    end
    assert_equal(context["<shorttext>"], context["<posts>.title"].range)
  end

  def test_text_column
    assert_nil(context["<posts>.comment"])
    Groonga::Schema.define_table("<posts>") do |table|
      table.text :comment
    end
    assert_equal(context["<text>"], context["<posts>.comment"].range)
  end

  def test_long_text_column
    assert_nil(context["<posts>.content"])
    Groonga::Schema.define_table("<posts>") do |table|
      table.long_text :content
    end
    assert_equal(context["<longtext>"], context["<posts>.content"].range)
  end

  def test_index
    assert_nil(context["<terms>.content"])
    Groonga::Schema.define_table("<posts>") do |table|
      table.long_text :content
    end
    Groonga::Schema.define_table("<terms>") do |table|
      table.index :posts_content, "<posts>.content"
    end
    assert_equal([context["<posts>.content"]],
                 context["<terms>.posts_content"].sources)
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
