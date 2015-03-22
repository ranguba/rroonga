# -*- coding: utf-8 -*-
#
# Copyright (C) 2012-2015  Kouhei Sutou <kou@clear-code.com>
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

  sub_test_case(".normalize") do
    def test_normal
      assert_equal("abc", Groonga::Normalizer.normalize("AbC"))
    end

    def test_space
      assert_equal("abcdefgh", Groonga::Normalizer.normalize("AbC Def　gh"))
    end

    def test_remove_blank
      assert_equal("abcdefgh",
                   Groonga::Normalizer.normalize("AbC Def　gh",
                                                 :remove_blank => true))
    end

    def test_group_text
      assert_equal("キロメートルキロメートルキロメートルキロメートル",
                   Groonga::Normalizer.normalize("㌖㌖㌖㌖"));
    end

    def test_keep_space
      # full width space => half width space
      assert_equal("abc def gh",
                   Groonga::Normalizer.normalize("AbC Def　gh",
                                                 :remove_blank => false))
    end

    def test_tilda
      assert_equal("~~~", Groonga::Normalizer.normalize("~～〜"))
    end

    def test_empty
      assert_equal("", Groonga::Normalizer.normalize(""))
    end
  end
end
