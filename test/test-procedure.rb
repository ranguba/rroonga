# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

class TypeTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    Groonga::Context.default = nil

    @db_path = @tmp_dir + "db"
    @database = Groonga::Database.create(:path => @db_path.to_s)
  end

  def test_builtins
    assert_equal_procedure("<token:delimit>", Groonga::Procedure::DELIMIT)
    assert_equal_procedure("<token:unigram>", Groonga::Procedure::UNIGRAM)
    assert_equal_procedure("<token:bigram>", Groonga::Procedure::BIGRAM)
    assert_equal_procedure("<token:trigram>", Groonga::Procedure::TRIGRAM)
    assert_equal_procedure("<token:mecab>", Groonga::Procedure::MECAB)
  end

  private
  def assert_equal_procedure(expected_name, id)
    procedure = Groonga::Context.default[id]
    assert_equal(expected_name,
                 procedure ? procedure.name : procedure)
  end
end
