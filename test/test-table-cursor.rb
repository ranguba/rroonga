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

class TableCursorTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    Groonga::Context.default = nil
    @context = Groonga::Context.default

    @db_path = @tmp_dir + "db"
    @database = Groonga::Database.create(:path => @db_path.to_s)

    @tables_dir = @tmp_dir + "tables"
    FileUtils.mkdir_p(@tables_dir.to_s)

    @bookmarks_path = @tables_dir + "table"
    @bookmarks = Groonga::PatriciaTrie.create(:name => "bookmarks",
                                              :path => @bookmarks_path.to_s)
    @groonga_bookmark_id = @bookmarks.add("groonga")
    @cutter_bookmark_id = @bookmarks.add("Cutter")
    @ruby_bookmark_id = @bookmarks.add("Ruby")
  end

  def test_open
    keys = []
    @bookmarks.open_cursor do |cursor|
      while cursor.next
        keys << cursor.key
      end
    end
    assert_equal(["groonga", "Ruby", "Cutter"],
                 keys)
  end

  def test_open_ascendent
    record_and_key_list = []
    @bookmarks.open_cursor(:order => :ascending) do |cursor|
      record_and_key_list = cursor.collect {|record| [record, cursor.key]}
    end
    assert_equal([[record(@cutter_bookmark_id), "Cutter"],
                  [record(@ruby_bookmark_id), "Ruby"],
                  [record(@groonga_bookmark_id), "groonga"]],
                 record_and_key_list)
  end

  private
  def record(id)
    Groonga::Record.new(@bookmarks, id)
  end
end
