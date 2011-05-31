class IndexCursorTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
    setup_table
  end

  def test_open_cursor
    postings = []
    @terms.open_cursor do |table_cursor|
      index_cursor = nil
      @content_index.open_cursor(table_cursor) do |cursor|
        cursor.each do |posting|
          postings << posting.to_hash
        end
        index_cursor = cursor
      end
      assert_predicate(index_cursor, :closed?)
    end

    assert_equal(expected_postings, postings)
  end

  def test_enumerable
    postings = []
    @terms.open_cursor do |table_cursor|
      @content_index.open_cursor(table_cursor) do |cursor|
        postings = cursor.collect do |posting|
          posting.to_hash
        end
      end
    end

    assert_equal(expected_postings, postings)
  end

  private
  def create_hashes(keys, values)
    hashes = []
    values.each do |value|
      hash = {}
      keys.each_with_index do |key, i|
        hash[key] = value[i]
      end
      hashes << hash
    end
    hashes
  end

  def setup_table
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

    @articles = Groonga["Articles"]
    @terms = Groonga["Terms"]
    @content_index = Groonga["Terms.Articles_content"]

    @articles.add(:content => "l")
    @articles.add(:content => "ll")
    @articles.add(:content => "hello")
  end

  def expected_postings
    parameters = [:record_id, :section_id, :term_id, :position,
                  :term_frequency, :weight, :n_rest_postings]

    expected = [
                [1, 1, 1, 0, 1, 0, 1],
                [2, 1, 1, 0, 1, 0, 1],
                [2, 1, 2, 0, 1, 0, 1],
                [3, 1, 2, 0, 1, 0, 1],
                [3, 1, 3, 0, 1, 0, 0],
                [3, 1, 4, 1, 1, 0, 0],
                [3, 1, 5, 3, 1, 0, 0],
                [3, 1, 6, 4, 1, 0, 0]
               ]

    create_hashes(parameters, expected)
  end
end
