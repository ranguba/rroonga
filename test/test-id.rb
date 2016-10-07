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

class IDTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  test "NIL" do
    assert_equal(0, Groonga::ID::NIL)
  end

  test "MAX" do
    assert_equal(2 ** 30 - 1, Groonga::ID::MAX)
  end

  sub_test_case(".builtin?") do
    test "true" do
      assert do
        Groonga::ID.builtin?(Groonga::Type::INT32)
      end
    end

    test "false" do
      Groonga::Schema.define do |schema|
        schema.create_table("Users", :type => :hash)
      end
      users_id = Groonga["Users"].id

      assert do
        not Groonga::ID.builtin?(users_id)
      end
    end
  end

  sub_test_case(".builtin_type?") do
    test "true" do
      assert do
        Groonga::ID.builtin_type?(Groonga::Type::INT32)
      end
    end

    test "false" do
      token_bigram_id = Groonga["TokenBigram"].id

      assert do
        not Groonga::ID.builtin_type?(token_bigram_id)
      end
    end
  end
end
