# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
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

class EncodingTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_constants
    assert_equal(:default, Groonga::Encoding::DEFAULT)
    assert_equal(:none, Groonga::Encoding::NONE)
    assert_equal(:euc_jp, Groonga::Encoding::EUC_JP)
    assert_equal(:sjis, Groonga::Encoding::SJIS)
    assert_equal(:utf8, Groonga::Encoding::UTF8)
    assert_equal(:latin1, Groonga::Encoding::LATIN1)
    assert_equal(:koi8r, Groonga::Encoding::KOI8R)
  end

  def test_default
    Groonga::Encoding.default = :utf8
    assert_equal(:utf8, Groonga::Encoding.default)
  end
end
