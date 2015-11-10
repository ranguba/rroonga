# Copyright (C) 2015  Kouhei Sutou <kou@clear-code.com>
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

class ConfTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  sub_test_case "#[]" do
    test "existent" do
      context.conf["rroonga.key"] = "value"
      assert_equal("value", context.conf["rroonga.key"])
    end

    test "nonexistent" do
      assert_nil(context.conf["nonexistent"])
    end
  end
end
