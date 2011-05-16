class IndexCursorTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
  end

  def test_initialize
    assert_nothing_raised do
      Groonga::IndexCursor.new
    end
  end
end
