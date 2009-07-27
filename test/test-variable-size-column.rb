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

class VariableSizeColumnTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database

    setup_users_table
  end

  def setup_users_table
    @users_path = @tables_dir + "users"
    @users = Groonga::Array.create(:name => "users",
                                   :path => @users_path.to_s)

    @users_name_column_path = @columns_dir + "name"
    @name = @users.define_column("name", "<shorttext>",
                                 :path => @users_name_column_path.to_s)
  end

  def test_inspect
    assert_equal("#<Groonga::VariableSizeColumn " +
                 "id: <#{@name.id}>, " +
                 "name: <users.name>, " +
                 "path: <#{@users_name_column_path}>, " +
                 "domain: <#{@users.inspect}>, " +
                 "range: <#{context['<shorttext>'].inspect}>, " +
                 "flags: <>" +
                 ">",
                 @name.inspect)
  end

  def test_domain
    assert_equal(@users, @name.domain)
  end

  def test_table
    assert_equal(@users, @name.table)
  end
end
