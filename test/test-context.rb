class ContextTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_use_ql?
    context = Groonga::Context.new
    assert_not_predicate(context, :use_ql?)

    context = Groonga::Context.new(:use_ql => true)
    assert_predicate(context, :use_ql?)
  end

  def test_batch_mode?
    context = Groonga::Context.new
    assert_not_predicate(context, :batch_mode?)

    context = Groonga::Context.new(:batch_mode => true)
    assert_predicate(context, :batch_mode?)
  end

  def test_encoding
    context = Groonga::Context.new
    assert_equal(Groonga::Encoding::DEFAULT, context.encoding)

    context = Groonga::Context.new(:encoding => :utf8)
    assert_equal(Groonga::Encoding::UTF8, context.encoding)
  end
end
