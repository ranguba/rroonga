# Copyright (C) 2013  Kouhei Sutou <kou@clear-code.com>
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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

class TableKeySupportTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  class TypeTest < self
    def test_int8
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "Int8")
      id = ids.add(-1)
      assert_equal(-1, id.key)
    end

    def test_uint8
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "UInt8")
      id = ids.add(1)
      assert_equal(1, id.key)
    end
  end
end
