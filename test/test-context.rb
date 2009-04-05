class ContextTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    Groonga::Context.default = nil
    Groonga::Context.default_options = nil
  end

  def test_default
    context = Groonga::Context.default
    assert_not_predicate(context, :use_ql?)
    assert_not_predicate(context, :batch_mode?)
    assert_equal(Groonga::Encoding::DEFAULT, context.encoding)
  end

  def test_default_options
    Groonga::Context.default_options = {
      :use_ql => true,
      :batch_mode => true,
      :encoding => Groonga::Encoding::UTF8,
    }
    context = Groonga::Context.default
    assert_predicate(context, :use_ql?)
    assert_predicate(context, :batch_mode?)
    assert_equal(Groonga::Encoding::UTF8, context.encoding)
  end

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
