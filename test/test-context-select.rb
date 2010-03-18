# Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>
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

class ContextSelectTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database
  setup
  def setup_data
    @users = Groonga::Hash.create(:name => "Users", :key_type => "ShortText")
    @users.add("morita")
    @users.add("gunyara-kun")
    @users.add("yu")
  end

  def test_no_option
    result = context.select(@users)
    assert_equal([3,
                  [{"_id" => 1, "_key" => "morita"},
                   {"_id" => 2, "_key" => "gunyara-kun"},
                   {"_id" => 3, "_key" => "yu"}]],
                 [result.n_hits, result.records])
  end

  def test_success
    result = context.select(@users)
    assert_equal([true, 0, nil],
                 [result.success?, result.return_code, result.error_message])
  end

  def test_output_columns
    result = context.select(@users, :output_columns => ["_key"])
    assert_equal([3,
                  [{"_key" => "morita"},
                   {"_key" => "gunyara-kun"},
                   {"_key" => "yu"}]],
                 [result.n_hits, result.records])
  end
end
