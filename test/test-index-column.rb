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

class IndexColumnTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
  end

  def test_index?
    articles = Groonga::Array.create(:name => "Articles")
    articles.define_column("content", "Text")

    terms = Groonga::Hash.create(:name => "Terms",
                                 :default_tokenizer => "TokenBigram")
    content_index = terms.define_index_column("content", articles,
                                              :with_section => true)
    assert_predicate(content_index, :index?)
  end

  def test_vector?
    articles = Groonga::Array.create(:name => "Articles")
    articles.define_column("content", "Text")

    terms = Groonga::Hash.create(:name => "Terms",
                                 :default_tokenizer => "TokenBigram")
    content_index = terms.define_index_column("content", articles,
                                              :with_section => true)
    assert_not_predicate(content_index, :vector?)
  end

  def test_scalar?
    articles = Groonga::Array.create(:name => "Articles")
    articles.define_column("content", "Text")

    terms = Groonga::Hash.create(:name => "Terms",
                                 :default_tokenizer => "TokenBigram")
    content_index = terms.define_index_column("content", articles,
                                              :with_section => true)
    assert_not_predicate(content_index, :scalar?)
  end

  def test_array_set_with_record
    articles = Groonga::Array.create(:name => "Articles")
    articles.define_column("content", "Text")

    terms = Groonga::Hash.create(:name => "Terms",
                                 :default_tokenizer => "TokenBigram")
    content_index = terms.define_index_column("content", articles,
                                              :with_section => true)

    content = <<-EOC
    groonga は組み込み型の全文検索エンジンライブラリです。
    DBMSやスクリプト言語処理系等に組み込むことによって、その
    全文検索機能を強化することができます。また、リレーショナ
    ルモデルに基づくデータストア機能を内包しており、groonga
    単体でも高速なデータストアサーバとして使用することができ
    ます。

    ■全文検索方式
    転置索引型の全文検索エンジンです。転置索引は圧縮されてファ
    イルに格納され、検索時のディスク読み出し量を小さく、かつ
    局所的に抑えるように設計されています。用途に応じて以下の
    索引タイプを選択できます。
    EOC

    groonga = articles.add(:content => content)

    content.split(/\n{2,}/).each_with_index do |sentence, i|
      content_index[groonga] = {:value => sentence, :section => i + 1}
    end
    assert_equal([groonga],
                 content_index.search("エンジン").collect {|record| record.key})
  end

  def test_shorter_query_than_ngram
    articles = Groonga::Array.create(:name => "Articles")
    articles.define_column("content", "Text")

    terms = Groonga::PatriciaTrie.create(:name => "Terms",
                                         :default_tokenizer => "TokenBigram")
    content_index = terms.define_index_column("content", articles,
                                              :source => "Articles.content")
    articles.add(:content => 'l')
    articles.add(:content => 'll')
    articles.add(:content => 'hello')

    assert_search(["hello"], content_index, "he")
    assert_search(["ll", "hello"], content_index, "ll")
    assert_search(["l", "ll", "hello"], content_index, "l")
  end

  def test_with_section?
    define_index_column_with_flags
    assert_equal({
                   :section => true,
                   :weight => false,
                   :position => false,
                 },
                 {
                   :section => context["Tags.section"].with_section?,
                   :weight => context["Tags.weight"].with_section?,
                   :position => context["Tags.position"].with_section?,
                 })
  end

  def test_with_weight?
    define_index_column_with_flags
    assert_equal({
                   :section => false,
                   :weight => true,
                   :position => false,
                 },
                 {
                   :section => context["Tags.section"].with_weight?,
                   :weight => context["Tags.weight"].with_weight?,
                   :position => context["Tags.position"].with_weight?,
                 })
  end

  def test_with_position?
    define_index_column_with_flags
    assert_equal({
                   :section => false,
                   :weight => false,
                   :position => true,
                 },
                 {
                   :section => context["Tags.section"].with_position?,
                   :weight => context["Tags.weight"].with_position?,
                   :position => context["Tags.position"].with_position?,
                 })
  end

  def test_open_cursor
    articles = Groonga::Array.create(:name => "Articles")
    articles.define_column("content", "Text")

    terms = Groonga::Hash.create(:name => "Terms",
                                 :default_tokenizer => "TokenBigramSplitSymbolAlpha")
    content_index = terms.define_index_column("content", articles,
                                              :with_section => true)

    articles.add(:content => "l")
    articles.add(:content => "ll")
    articles.add(:content => "hello")

    terms.open_cursor do |table_cursor|
      cursor = content_index.open_cursor(table_cursor)
      assert_instance_of(Groonga::IndexCursor, cursor)
    end
  end

  private
  def assert_search(expected, content_index, keyword)
    result = content_index.search(keyword).collect do |entry|
      entry.key["content"]
    end
    assert_equal(expected, result)
  end

  def define_index_column_with_flags
    Groonga::Schema.define do |schema|
      schema.create_table("Articles") do |table|
        table.text("tags", :type => :vector)
      end

      schema.create_table("Tags",
                          :type => :patricia_trie,
                          :key_type => "ShortText",
                          :default_tokenizer => "TokenDelimit") do |table|
        table.index("Articles.tags",
                    :name => "section",
                    :with_section => true)
        table.index("Articles.tags",
                    :name => "weight",
                    :with_weight => true)
        table.index("Articles.tags",
                    :name => "position",
                    :with_position => true)
      end
    end
  end
end
