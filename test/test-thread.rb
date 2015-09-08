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

class ThreadTest < Test::Unit::TestCase
  include GroongaTestUtils

  sub_test_case "limit" do
    teardown do
      Groonga::Thread.limit_getter = nil
      Groonga::Thread.limit_setter = nil
    end

    test "default" do
      assert_equal(0, Groonga::Thread.limit)
    end

    test "custom" do
      limit = 0
      Groonga::Thread.limit_getter = lambda do
        limit
      end
      Groonga::Thread.limit_setter = lambda do |new_limit|
        limit = new_limit
      end

      Groonga::Thread.limit = 10
      assert_equal(10, Groonga::Thread.limit)
    end
  end
end
