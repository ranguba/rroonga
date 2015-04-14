# -*- coding: utf-8 -*-
#
# Copyright (C) 2009-2015  Kouhei Sutou <kou@clear-code.com>
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

class IndexColumnTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
  end

  class PredicateTest < self
    def setup
      super
      setup_index
    end

    def setup_index
      articles = Groonga::Array.create(:name => "Articles")
      articles.define_column("content", "Text")

      terms = Groonga::Hash.create(:name => "Terms",
                                   :key_type => "ShortText",
                                   :default_tokenizer => "TokenBigram")
      @index = terms.define_index_column("content", articles,
                                         :with_section => true)
    end

    def test_index?
      assert_predicate(@index, :index?)
    end

    def test_vector?
      assert_not_predicate(@index, :vector?)
    end

    def test_scalar?
      assert_not_predicate(@index, :scalar?)
    end
  end

  class CRUDTest < self
    setup
    def setup_schema
      Groonga::Schema.define do |schema|
        schema.create_table("Articles") do |table|
          table.text("content")
        end

        schema.create_table("Terms",
                            :type => :hash,
                            :key_type => "ShortText",
                            :default_tokenizer => "TokenBigram") do |table|
          table.index("Articles.content",
                      :name => "articles_content",
                      :with_position => true,
                      :with_section => true)
        end
      end

      @articles = Groonga["Articles"]
      @index = Groonga["Terms.articles_content"]
    end

    def test_add
      content = <<-CONTENT
      Groonga is a fast and accurate full text search engine based on
      inverted index. One of the characteristics of groonga is that a
      newly registered document instantly appears in search
      results. Also, groonga allows updates without read locks. These
      characteristics result in superior performance on real-time
      applications.

      Groonga is also a column-oriented database management system
      (DBMS). Compared with well-known row-oriented systems, such as
      MySQL and PostgreSQL, column-oriented systems are more suited for
      aggregate queries. Due to this advantage, groonga can cover
      weakness of row-oriented systems.

      The basic functions of groonga are provided in a C library. Also,
      libraries for using groonga in other languages, such as Ruby, are
      provided by related projects. In addition, groonga-based storage
      engines are provided for MySQL and PostgreSQL. These libraries
      and storage engines allow any application to use groonga. See
      usage examples.
      CONTENT

      groonga = @articles.add(:content => content)

      content.split(/\n{2,}/).each_with_index do |sentence, i|
        @index.add(groonga, sentence, :section => i + 1)
      end
      assert_equal([groonga], @index.search("engine").collect(&:key))
    end

    def test_delete
      content = <<-CONTENT
      Groonga is a fast and accurate full text search engine based on
      inverted index. One of the characteristics of groonga is that a
      newly registered document instantly appears in search
      results. Also, groonga allows updates without read locks. These
      characteristics result in superior performance on real-time
      applications.

      Groonga is also a column-oriented database management system
      (DBMS). Compared with well-known row-oriented systems, such as
      MySQL and PostgreSQL, column-oriented systems are more suited for
      aggregate queries. Due to this advantage, groonga can cover
      weakness of row-oriented systems.

      The basic functions of groonga are provided in a C library. Also,
      libraries for using groonga in other languages, such as Ruby, are
      provided by related projects. In addition, groonga-based storage
      engines are provided for MySQL and PostgreSQL. These libraries
      and storage engines allow any application to use groonga. See
      usage examples.
      CONTENT

      groonga = @articles.add(:content => content)

      content.split(/\n{2,}/).each_with_index do |sentence, i|
        @index.add(groonga, sentence, :section => i + 1)
      end
      content.split(/\n{2,}/).each_with_index do |sentence, i|
        @index.delete(groonga, sentence, :section => i + 1)
      end
      assert_equal([], @index.search("engine").collect(&:key))
    end

    def test_update
      old_sentence = <<-SENTENCE
      Groonga is a fast and accurate full text search engine based on
      inverted index. One of the characteristics of groonga is that a
      newly registered document instantly appears in search
      results. Also, groonga allows updates without read locks. These
      characteristics result in superior performance on real-time
      applications.
      SENTENCE

      new_sentence = <<-SENTENCE
      Groonga is also a column-oriented database management system
      (DBMS). Compared with well-known row-oriented systems, such as
      MySQL and PostgreSQL, column-oriented systems are more suited for
      aggregate queries. Due to this advantage, groonga can cover
      weakness of row-oriented systems.
      SENTENCE

      groonga = @articles.add(:content => old_sentence)

      @index.add(groonga, old_sentence, :section => 1)
      assert_equal([groonga],
                   @index.search("engine").collect(&:key))
      assert_equal([],
                   @index.search("MySQL").collect(&:key))

      groonga[:content] = new_sentence
      @index.update(groonga, old_sentence, new_sentence, :section => 1)
      assert_equal([],
                   @index.search("engine").collect(&:key))
      assert_equal([groonga],
                   @index.search("MySQL").collect(&:key))
    end
  end

  class NGramTest < self
    setup
    def setup_schema
      Groonga::Schema.define do |schema|
        schema.create_table("Articles") do |table|
          table.text("content")
        end

        schema.create_table("Terms",
                            :type => :patricia_trie,
                            :key_type => "ShortText",
                            :default_tokenizer => "TokenBigram") do |table|
          table.index("Articles.content", :name => "content")
        end
      end

      @articles = Groonga["Articles"]
      @index = Groonga["Terms.content"]
    end

    setup
    def setup_records
      @articles.add(:content => 'l')
      @articles.add(:content => 'll')
      @articles.add(:content => 'hello')
    end

    def test_N_length_query
      assert_equal(["ll", "hello"], search("ll"))
    end

    def test_shorter_query
      assert_equal(["l", "ll", "hello"], search("l"))
    end

    private
    def search(keyword)
      @index.search(keyword).collect do |entry|
        entry.key["content"]
      end
    end
  end

  class FlagTest < self
    def setup
      super
      define_table
    end

    def define_table
      Groonga::Schema.define do |schema|
        schema.create_table("Articles") do |table|
          table.text("tags", :type => :vector)
        end
      end
    end

    def test_with_section?
      Groonga::Schema.define do |schema|
        schema.create_table("Tags",
                            :type => :patricia_trie,
                            :key_type => "ShortText",
                            :default_tokenizer => "TokenDelimit") do |table|
          table.index("Articles.tags",
                      :name => "section",
                      :with_section => true)
          table.index("Articles.tags",
                      :name => "no_section")
        end
      end

      assert_equal([
                     true,
                     false,
                   ],
                   [
                     context["Tags.section"].with_section?,
                     context["Tags.no_section"].with_section?,
                   ])
    end

    def test_with_weight?
      Groonga::Schema.define do |schema|
        schema.create_table("Tags",
                            :type => :patricia_trie,
                            :key_type => "ShortText",
                            :default_tokenizer => "TokenDelimit") do |table|
          table.index("Articles.tags",
                      :name => "weight",
                      :with_weight => true)
          table.index("Articles.tags",
                      :name => "no_weight")
        end
      end

      assert_equal([
                     true,
                     false,
                   ],
                   [
                     context["Tags.weight"].with_weight?,
                     context["Tags.no_weight"].with_weight?,
                   ])
    end

    def test_with_position?
      Groonga::Schema.define do |schema|
        schema.create_table("Tags",
                            :type => :patricia_trie,
                            :key_type => "ShortText",
                            :default_tokenizer => "TokenDelimit") do |table|
          table.index("Articles.tags",
                      :name => "true",
                      :with_position => true)
          table.index("Articles.tags",
                      :name => "default")
          table.index("Articles.tags",
                      :name => "false",
                      :with_position => false)
        end
      end

      assert_equal([
                     true,
                     false,
                     true,
                   ],
                   [
                     context["Tags.true"].with_position?,
                     context["Tags.false"].with_position?,
                     context["Tags.default"].with_position?,
                   ])
    end
  end

  class SourceTest < self
    def test_nil
      Groonga::Schema.define do |schema|
        schema.create_table("Contents",
                            :type => :patricia_trie,
                            :key_type => "ShortText") do |table|
        end

        schema.create_table("Terms",
                            :type => :patricia_trie,
                            :key_type => "ShortText",
                            :default_tokenizer => "TokenBigram") do |table|
        end
      end

      index = Groonga["Terms"].define_index_column("index", "Contents")
      assert_raise(ArgumentError.new("couldn't find source: <nil>")) do
        index.source = nil
      end
    end
  end

  class DiskUsageTest < self
    def setup
      setup_database
      setup_index
    end

    def setup_index
      articles = Groonga::Array.create(:name => "Articles")
      articles.define_column("content", "Text")

      terms = Groonga::Hash.create(:name => "Terms",
                                   :key_type => "ShortText",
                                   :default_tokenizer => "TokenBigram")
      @index = terms.define_index_column("content", articles,
                                         :with_section => true)
    end

    def test_disk_usage
      assert_equal(File.size(@index.path) + File.size("#{@index.path}.c"),
                   @index.disk_usage)
    end
  end

  class SearchTest < self
    class WeightTest < self
      def setup
        super
        setup_schema
      end

      def setup_schema
        Groonga::Schema.define do |schema|
          schema.create_table("Memos",
                              :type => :hash,
                              :key_type => :short_text) do |table|
            table.short_text("tags",
                             :type => :vector,
                             :with_weight => true)
          end

          schema.create_table("Tags",
                              :type => :hash,
                              :key_type => :short_text) do |table|
            table.index("Memos.tags",
                        :name => "memos_index",
                        :with_weight => true)
          end
        end

        @memos = context["Memos"]
        @index = context["Tags.memos_index"]
      end

      def search(keyword, options={})
        @index.search(keyword, options).collect do |record|
          {
            :key => record._key,
            :score => record.score,
          }
        end
      end

      def test_index
        record = @memos.add("Rroonga is fun!")
        record.tags = [
          {
            :value => "rroonga",
            :weight => 9,
          }
        ]

        expected = [
          {
            :key => "Rroonga is fun!",
            :score => 10,
          }
        ]
        assert_equal(expected, search("rroonga"))
      end

      def test_search
        record = @memos.add("Rroonga is fun!")
        record.tags = [
          {
            :value => "rroonga",
          }
        ]

        expected = [
          {
            :key => "Rroonga is fun!",
            :score => 5,
          }
        ]
        assert_equal(expected, search("rroonga", :weight => 5))
      end

      def test_index_and_search
        record = @memos.add("Rroonga is fun!")
        record.tags = [
          {
            :value => "rroonga",
            :weight => 9,
          }
        ]

        expected = [
          {
            :key => "Rroonga is fun!",
            :score => 50
          }
        ]
        assert_equal(expected, search("rroonga", :weight => 5))
      end
    end

    class OperatorTest < self
      def setup
        super
        setup_schema
      end

      def setup_schema
        Groonga::Schema.define do |schema|
          schema.create_table("Memos",
                              :type => :hash,
                              :key_type => :short_text) do |table|
            table.short_text("tags", :type => :vector)
          end

          schema.create_table("Tags",
                              :type => :hash,
                              :key_type => :short_text) do |table|
            table.index("Memos.tags",
                        :name => "memos_index")
          end
        end

        @memos = context["Memos"]
        @index = context["Tags.memos_index"]
      end

      def test_adjust
        @memos.add("Rroonga is fun!", :tags => ["rroonga", "groonga"])
        @memos.add("Groonga is fast!", :tags => ["groonga"])

        result = @index.search("groonga")
        @index.search("rroonga", :result => result, :operator => :adjust)
        expected = [
          {
            :key => "Rroonga is fun!",
            :score => 2,
          },
          {
            :key => "Groonga is fast!",
            :score => 1,
          }
        ]
        actual = result.collect do |record|
          {
            :key => record._key,
            :score => record.score,
          }
        end
        assert_equal(expected, actual)
      end
    end
  end

  class EstimateSizeTest < self
    sub_test_case "token ID" do
      setup
      def setup_schema
        Groonga::Schema.define do |schema|
          schema.create_table("Articles") do |table|
            table.text("content")
          end

          schema.create_table("Terms",
                              :type => :hash,
                              :key_type => "ShortText",
                              :default_tokenizer => "TokenBigram",
                              :normalizer => "NormalizerAuto") do |table|
            table.index("Articles.content",
                        :name => "articles_content",
                        :with_position => true,
                        :with_section => true)
          end
        end

        @articles = Groonga["Articles"]
        @terms = Groonga["Terms"]
        @index = Groonga["Terms.articles_content"]
      end

      setup
      def setup_data
        @articles.add(:content => "Groonga is fast")
        @articles.add(:content => "Rroonga is fast")
        @articles.add(:content => "Mroonga is fast")
      end

      def test_id
        assert_equal(7, @index.estimate_size(@terms["fast"].id))
      end

      def test_record
        assert_equal(7, @index.estimate_size(@terms["fast"]))
      end
    end
  end
end
