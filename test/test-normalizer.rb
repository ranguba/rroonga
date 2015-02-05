# Copyright (C) 2012  Kouhei Sutou <kou@clear-code.com>
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

class NormalizerTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_normalize
    assert_equal("abc", Groonga::Normalizer.normalize("AbC"))
  end

  def test_normalize_with_space
    assert_equal("abcdefgh", Groonga::Normalizer.normalize("AbC Def　gh"))
  end

  def test_normalize_with_space_explicitly
    assert_equal("abcdefgh",
                 Groonga::Normalizer.normalize("AbC Def　gh", Groonga::Normalizer::REMOVE_BLANK))
  end

  def test_normalize_group_text
    assert_equal("キロメートルキロメートルキロメートルキロメートル",
                 Groonga::Normalizer.normalize("㌖㌖㌖㌖"));
  end

  def test_normalize_keep_space
    # full width space => half width space
    assert_equal("abc def gh",
                 Groonga::Normalizer.normalize("AbC Def　gh", 0))
  end

  def test_normalize_tilda
    assert_equal("~~~", Groonga::Normalizer.normalize("~～〜"))
  end
end
