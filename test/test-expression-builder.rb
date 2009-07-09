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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

class ExpressionBuilderTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database
  setup :setup_tables
  setup :setup_data

  def setup_tables
    @users = Groonga::Hash.create(:name => "<users>")
    @name = @users.define_column("name", "<shorttext>")
  end

  def setup_data
    @morita = @users.add("morita", :name => "mori daijiro")
    @gunyara_kun = @users.add("gunyara-kun", :name => "Tasuku SUENAGA")
  end

  def test_equal
    result = @users.scan do |builder|
      builder.record["name"] == "mori daijiro"
    end
    assert_equal(["mori daijiro"],
                 result.collect {|record| record.key["name"]})
  end
end
