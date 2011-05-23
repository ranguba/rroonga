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

        # term = Groonga::Record.new(terms, posting.term_id)
        # p [posting, term.key]
        # record = Groonga::Record.new(articles, posting.record_id)
        # p [posting.record_id, record["content"]]
      end

      assert_equal(1, postings[0].record_id)
      assert_equal(1, postings[0].section_id)
      assert_equal(1, postings[0].term_id)
      assert_equal(0, postings[0].position)
      assert_equal(1, postings[0].term_frequency)
      assert_equal(0, postings[0].weight)
      assert_equal(1, postings[0].n_rest_postings)

      assert_equal(2, postings[1].record_id)
      assert_equal(1, postings[1].section_id)
      assert_equal(1, postings[1].term_id)
      assert_equal(0, postings[1].position)
      assert_equal(1, postings[1].term_frequency)
      assert_equal(0, postings[1].weight)
      assert_equal(1, postings[1].n_rest_postings)

      assert_equal(2, postings[2].record_id)
      assert_equal(1, postings[2].section_id)
      assert_equal(2, postings[2].term_id)
      assert_equal(0, postings[2].position)
      assert_equal(1, postings[2].term_frequency)
      assert_equal(0, postings[2].weight)
      assert_equal(1, postings[2].n_rest_postings)

      assert_equal(3, postings[3].record_id)
      assert_equal(1, postings[3].section_id)
      assert_equal(2, postings[3].term_id)
      assert_equal(0, postings[3].position)
      assert_equal(1, postings[3].term_frequency)
      assert_equal(0, postings[3].weight)
      assert_equal(1, postings[3].n_rest_postings)

      assert_equal(3, postings[4].record_id)
      assert_equal(1, postings[4].section_id)
      assert_equal(3, postings[4].term_id)
      assert_equal(0, postings[4].position)
      assert_equal(1, postings[4].term_frequency)
      assert_equal(0, postings[4].weight)
      assert_equal(0, postings[4].n_rest_postings)

      assert_equal({:record_id => 3,
                     :section_id => 1,
                     :term_id => 4,
                     :postion => 1,
                     :term_frequency => 1,
                     :weight => 0,
                     :n_rest_postings => 0},
                   {:record_id => postings[5].record_id,
                     :section_id => postings[5].section_id,
                     :term_id => postings[5].term_id,
                     :postion => postings[5].position,
                     :term_frequency => postings[5].term_frequency,
                     :weight => postings[5].weight,
                     :n_rest_postings => postings[5].n_rest_postings})

      assert_equal({:record_id => 3,
                     :section_id => 1,
                     :term_id => 5,
                     :postion => 3,
                     :term_frequency => 1,
                     :weight => 0,
                     :n_rest_postings => 0},
                   {:record_id => postings[6].record_id,
                     :section_id => postings[6].section_id,
                     :term_id => postings[6].term_id,
                     :postion => postings[6].position,
                     :term_frequency => postings[6].term_frequency,
                     :weight => postings[6].weight,
                     :n_rest_postings => postings[6].n_rest_postings})

      assert_equal({:record_id => 3,
                     :section_id => 1,
                     :term_id => 6,
                     :postion => 4,
                     :term_frequency => 1,
                     :weight => 0,
                     :n_rest_postings => 0},
                   {:record_id => postings[7].record_id,
                     :section_id => postings[7].section_id,
                     :term_id => postings[7].term_id,
                     :postion => postings[7].position,
                     :term_frequency => postings[7].term_frequency,
                     :weight => postings[7].weight,
                     :n_rest_postings => postings[7].n_rest_postings})

    end
  end
end
