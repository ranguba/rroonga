# Copyright (C) 2016  Kouhei Sutou <kou@clear-code.com>
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

class NameTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  sub_test_case(".column?") do
    test "true" do
      assert do
        Groonga::Name.column?("Users.age")
      end
    end

    test "false" do
      assert do
        not Groonga::Name.column?("Users")
      end
    end
  end
end
