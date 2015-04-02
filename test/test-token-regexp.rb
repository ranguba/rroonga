# Copyright (C) 2015  Masafumi Yokoyama <clear-code.com>
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

class TokenRegexpTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
  end

  def test_default_tokenizer
    terms = Groonga::PatriciaTrie.create(:name => "Terms",
                                         :key_type => "ShortText",
                                         :default_tokenizer => "TokenRegexp")
    assert_equal(context["TokenRegexp"],
                 terms.default_tokenizer)
  end
end
