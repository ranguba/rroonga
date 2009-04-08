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

class RecordTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database

    @bookmarks_path = @tables_dir + "table"
    @bookmarks = Groonga::PatriciaTrie.create(:name => "bookmarks",
                                              :path => @bookmarks_path.to_s)
    @uri_column_path = @columns_dir + "columns"
    @bookmark_uri = @bookmarks.define_column("uri", "<shorttext>",
                                             :path => @uri_column_path.to_s)
  end

  def test_column_accessor
    groonga = @bookmarks.add("groonga")
    groonga["uri"] = "http://groonga.org/"
    assert_equal("http://groonga.org/", groonga["uri"])
  end
end
