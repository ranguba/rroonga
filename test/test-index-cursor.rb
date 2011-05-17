class IndexCursorTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
  end

  def test_open_cursor
    Groonga::Schema.define do |schema|
      schema.create_table("Articles") do |table|
        table.text("content")
      end

      schema.create_table("Terms",
                          :type => :hash,
                          :default_tokenizer => :bigram_split_symbol_alpha) do |table|
        table.index("Articles.content")
      end
    end

    articles = Groonga["Articles"]
    terms = Groonga["Terms"]
    content_index = Groonga["Terms.Articles_content"]

    articles.add(:content => "l")
    articles.add(:content => "ll")
    articles.add(:content => "hello")

    terms.open_cursor do |table_cursor|
      cursor = content_index.open_cursor(table_cursor)
      postings = []
      while (posting = cursor.next)
        postings << posting
      end
      assert_equal(["xxx"],
                   postings)
    end
  end
end
