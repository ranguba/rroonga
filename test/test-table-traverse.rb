# Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>
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

class TableTraverseTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database

    @bookmarks_path = @tables_dir + "table"
    @bookmarks = Groonga::PatriciaTrie.create(:name => "Bookmarks",
                                              :path => @bookmarks_path.to_s)
    @groonga_bookmark = @bookmarks.add("groonga")
    @cutter_bookmark = @bookmarks.add("Cutter")
    @ruby_bookmark = @bookmarks.add("Ruby")
  end

  class CursorTest < self
    def test_default
      keys = []
      @bookmarks.open_cursor do |cursor|
        while cursor.next
          keys << cursor.key
        end
      end
      assert_equal(["Cutter", "Ruby", "groonga"],
                   keys)
    end

    def test_order_ascending
      record_and_key_list = []
      @bookmarks.open_cursor(:order => :ascending) do |cursor|
        record_and_key_list = cursor.collect {|record| [record, cursor.key]}
      end
      assert_equal([[@cutter_bookmark, "Cutter"],
                    [@ruby_bookmark, "Ruby"],
                    [@groonga_bookmark, "groonga"]],
                   record_and_key_list)
    end

    def test_without_limit_and_offset
      users = create_users
      add_users(users)
      results = []
      users.open_cursor do |cursor|
        cursor.each do |record|
          results << record["name"]
        end
      end

      assert_equal((100..199).collect {|i| "user#{i}"},
                   results)
    end

    def test_with_limit
      users = create_users
      add_users(users)
      results = []
      users.open_cursor(:limit => 20) do |cursor|
        cursor.each do |record|
          results << record["name"]
        end
      end

      assert_equal((100...120).collect {|i| "user#{i}"},
                   results)
    end

    def test_with_offset
      users = create_users
      add_users(users)
      results = []
      users.open_cursor(:offset => 20) do |cursor|
        cursor.each do |record|
          results << record["name"]
        end
      end

      assert_equal((120...200).collect {|i| "user#{i}"},
                   results)
    end

    def test_with_limit_and_offset
      users = create_users
      add_users(users)
      results = []
      users.open_cursor(:limit => 20, :offset => 20) do |cursor|
        cursor.each do |record|
          results << record["name"]
        end
      end

      assert_equal((120...140).collect {|i| "user#{i}"},
                   results)
    end

    def test_delete
      users = create_users
      add_users(users)

      users.open_cursor(:limit => 20) do |cursor|
        20.times do
          cursor.next
          cursor.delete
        end
      end

      results = []
      users.open_cursor do |cursor|
        cursor.each do |record|
          results << record["name"]
        end
      end

      assert_equal((120...200).collect {|i| "user#{i}"},
                   results)
    end

    def test_patricia_trie_cursor_key
      sites = Groonga::PatriciaTrie.create(:name => "Sites")
      sites.add("http://groonga.org/")
      sites.open_cursor do |cursor|
        cursor.next
        assert_equal("http://groonga.org/", cursor.key)
      end
    end

    def test_order_by_id
      sites = Groonga::PatriciaTrie.create(:name => "Sites")
      sites.add("http://qwik.jp/senna/")
      sites.add("http://www.ruby-lang.org/")
      sites.add("http://groonga.org/")
      keys = []
      sites.open_cursor(:order_by => :id) do |cursor|
        while cursor.next
          keys << cursor.key
        end
      end
      assert_equal(["http://qwik.jp/senna/",
                    "http://www.ruby-lang.org/",
                    "http://groonga.org/"],
                   keys)
    end

    def test_order_by_key
      sites = Groonga::PatriciaTrie.create(:name => "Sites")
      sites.add("http://www.ruby-lang.org/")
      sites.add("http://qwik.jp/senna/")
      sites.add("http://groonga.org/")
      keys = []
      sites.open_cursor(:order_by => :key) do |cursor|
        while cursor.next
          keys << cursor.key
        end
      end
      assert_equal(["http://groonga.org/",
                    "http://qwik.jp/senna/",
                    "http://www.ruby-lang.org/"],
                   keys)
    end

    def test_each_without_block
      @bookmarks.open_cursor do |cursor|
        keys = cursor.each.collect(&:key)
        assert_equal(["Cutter", "Ruby", "groonga"], keys)
      end
    end
  end

  class EachTest < self
    def test_default
      keys = []
      @bookmarks.each do |record|
        keys << record.key
      end
      assert_equal(["Cutter", "Ruby", "groonga"],
                   keys)
    end

    def test_order_ascending
      record_and_key_list = []
      @bookmarks.each(:order => :ascending) do |record|
        record_and_key_list << [record, record.key]
      end
      assert_equal([[@cutter_bookmark, "Cutter"],
                    [@ruby_bookmark, "Ruby"],
                    [@groonga_bookmark, "groonga"]],
                   record_and_key_list)
    end

    def test_without_limit_and_offset
      users = create_users
      add_users(users)
      results = []
      users.each do |record|
        results << record["name"]
      end

      assert_equal((100..199).collect {|i| "user#{i}"},
                   results)
    end

    def test_with_limit
      users = create_users
      add_users(users)
      results = []
      users.each(:limit => 20) do |record|
        results << record["name"]
      end

      assert_equal((100...120).collect {|i| "user#{i}"},
                   results)
    end

    def test_with_offset
      users = create_users
      add_users(users)
      results = []
      users.each(:offset => 20) do |record|
        results << record["name"]
      end

      assert_equal((120...200).collect {|i| "user#{i}"},
                   results)
    end

    def test_with_limit_and_offset
      users = create_users
      add_users(users)
      results = []
      users.each(:limit => 20, :offset => 20) do |record|
        results << record["name"]
      end

      assert_equal((120...140).collect {|i| "user#{i}"},
                   results)
    end

    def test_patricia_trie_cursor_key
      sites = Groonga::PatriciaTrie.create(:name => "Sites")
      sites.add("http://groonga.org/")
      sites.each do |record|
        assert_equal("http://groonga.org/", record.key)
      end
    end

    def test_order_by_id
      sites = Groonga::PatriciaTrie.create(:name => "Sites")
      sites.add("http://qwik.jp/senna/")
      sites.add("http://www.ruby-lang.org/")
      sites.add("http://groonga.org/")
      keys = []
      sites.each(:order_by => :id) do |record|
        keys << record.key
      end
      assert_equal(["http://qwik.jp/senna/",
                    "http://www.ruby-lang.org/",
                    "http://groonga.org/"],
                   keys)
    end

    def test_order_by_key
      sites = Groonga::PatriciaTrie.create(:name => "Sites")
      sites.add("http://www.ruby-lang.org/")
      sites.add("http://qwik.jp/senna/")
      sites.add("http://groonga.org/")
      keys = []
      sites.each(:order_by => :key) do |record|
        keys << record.key
      end
      assert_equal(["http://groonga.org/",
                    "http://qwik.jp/senna/",
                    "http://www.ruby-lang.org/"],
                   keys)
    end
  end

  private
  def create_users
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name", "ShortText")
    users
  end

  def add_users(users)
    (0...100).to_a.each do |i|
      user = users.add
      user["name"] = "user#{i + 100}"
    end
    users
  end
end
