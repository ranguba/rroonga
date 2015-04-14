# Copyright (C) 2009-2014  Kouhei Sutou <kou@clear-code.com>
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
  end
end
