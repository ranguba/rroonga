# Copyright (C) 2009-2018  Kouhei Sutou <kou@clear-code.com>
# Copyright (C) 2015  Masafumi Yokoyama <yokoyama@clear-code.com>
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

class ProcedureTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_builtins
    assert_equal_procedure("TokenDelimit", Groonga::Procedure::DELIMIT)
    assert_equal_procedure("TokenUnigram", Groonga::Procedure::UNIGRAM)
    assert_equal_procedure("TokenBigram", Groonga::Procedure::BIGRAM)
    assert_equal_procedure("TokenTrigram", Groonga::Procedure::TRIGRAM)
    assert_equal_procedure("TokenMecab", Groonga::Procedure::MECAB,
                           :accept_nil => true)
  end

  def test_procedure?
    assert do
      Groonga["TokenBigram"].procedure?
    end
  end

  def test_function_procedure?
    assert do
      Groonga["rand"].function_procedure?
    end
  end

  def test_selector_procedure?
    assert do
      Groonga["between"].selector_procedure?
    end
  end

  def test_scorer_procedure?
    assert do
      Groonga["scorer_tf_idf"].scorer_procedure?
    end
  end

  def test_window_function_procedure?
    assert do
      Groonga["record_number"].window_function_procedure?
    end
  end

  def test_stable?
    assert do
      not Groonga["rand"].stable?
    end
  end

  private
  def assert_equal_procedure(expected_name, id, options={})
    procedure = Groonga::Context.default[id]
    expected_name = nil if procedure.nil? and options[:accept_nil]
    assert_equal(expected_name,
                 procedure ? procedure.name : procedure)
  end

  class TypeTest < self
    def test_tokenizer
      tokenizer = Groonga["TokenBigram"]
      assert_equal(Groonga::ProcedureType::TOKENIZER, tokenizer.type)
    end

    def test_scorer
      scorer = Groonga["scorer_tf_idf"]
      assert_equal(Groonga::ProcedureType::SCORER, scorer.type)
    end

    def test_window_function
      record_number = Groonga["record_number"]
      assert_equal(Groonga::ProcedureType::WINDOW_FUNCTION,
                   record_number.type)
    end
  end

  class SelectorOnlyProcedureTest < self
    def test_true
      assert do
        Groonga["sub_filter"].selector_only_procedure?
      end
    end

    def test_false
      assert do
        not Groonga["between"].selector_only_procedure?
      end
    end
  end
end
