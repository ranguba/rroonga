# Copyright (C) 2011  Kouhei Sutou <kou@clear-code.com>
# Copyright (C) 2016  Masafumi Yokoyama <yokoyama@clear-code.com>
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

class AccessorTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
    @posts = Groonga::Hash.create(:name => "Posts",
                                  :key_type => "ShortText",
                                  :value_type => "UInt32")
    @grouped_posts = @posts.group("_key")
    @id = @posts.column("_id")
    @key = @posts.column("_key")
    @value = @posts.column("_value")
  end

  def test_name
    assert_nil(@id.name)
  end

  def test_local_name
    assert_equal("_id", @id.local_name)
  end

  sub_test_case "#accessor?" do
    test "true" do
      assert do
        @id.accessor?
      end
    end

    test "false" do
      assert do
        not @posts.accessor?
      end
    end
  end

  sub_test_case "#key_accessor?" do
    test "true" do
      assert do
        @key.key_accessor?
      end
    end

    test "false" do
      assert do
        not @id.key_accessor?
      end
    end
  end

  sub_test_case "#id_accessor?" do
    test "true" do
      assert do
        @id.id_accessor?
      end
    end

    test "false" do
      assert do
        not @key.id_accessor?
      end
    end
  end

  sub_test_case "#value_accessor?" do
    test "true" do
      assert do
        @value.value_accessor?
      end
    end

    test "false" do
      assert do
        not @key.value_accessor?
      end
    end
  end

  sub_test_case "#score_accessor?" do
    test "true" do
      assert do
        @posts.select.column("_score").score_accessor?
      end
    end

    test "false" do
      assert do
        not @key.score_accessor?
      end
    end
  end

  sub_test_case "#n_sub_records_accessor?" do
    test "true" do
      assert do
        @grouped_posts.column("_nsubrecs").n_sub_records_accessor?
      end
    end

    test "false" do
      assert do
        not @key.n_sub_records_accessor?
      end
    end
  end
end
