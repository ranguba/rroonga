class ContextTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_new
    Groonga::Context.new
  end
end
