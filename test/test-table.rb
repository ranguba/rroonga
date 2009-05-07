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

class TableTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_create
    table_path = @tables_dir + "table"
    assert_not_predicate(table_path, :exist?)
    table = Groonga::PatriciaTrie.create(:name => "bookmarks",
                                         :path => table_path.to_s)
    assert_equal("bookmarks", table.name)
    assert_predicate(table_path, :exist?)
  end

  def test_temporary
    table = Groonga::PatriciaTrie.create
    assert_nil(table.name)
    assert_equal([], @tables_dir.children)
  end

  def test_open
    table_path = @tables_dir + "table"
    table = Groonga::Hash.create(:name => "bookmarks",
                                  :path => table_path.to_s)
    assert_equal("bookmarks", table.name)
    table.close

    called = false
    Groonga::Table.open(:name => "bookmarks") do |_table|
      table = _table
      assert_not_predicate(table, :closed?)
      assert_equal("bookmarks", _table.name)
      called = true
    end
    assert_true(called)
    assert_predicate(table, :closed?)
  end

  def test_open_by_path
    table_path = @tables_dir + "table"
    table = Groonga::PatriciaTrie.create(:name => "bookmarks",
                                         :path => table_path.to_s)
    assert_equal("bookmarks", table.name)
    table.close

    called = false
    Groonga::Table.open(:path => table_path.to_s) do |_table|
      table = _table
      assert_not_predicate(table, :closed?)
      assert_nil(_table.name)
      called = true
    end
    assert_true(called)
    assert_predicate(table, :closed?)
  end

  def test_open_override_name
    table_path = @tables_dir + "table"
    table = Groonga::PatriciaTrie.create(:name => "bookmarks",
                                         :path => table_path.to_s)
    assert_equal("bookmarks", table.name)
    table.close

    called = false
    Groonga::Table.open(:name => "no-name", :path => table_path.to_s) do |_table|
      table = _table
      assert_not_predicate(table, :closed?)
      assert_equal("no-name", _table.name)
      called = true
    end
    assert_true(called)
    assert_predicate(table, :closed?)
  end

  def test_open_wrong_table
    table_path = @tables_dir + "table"
    Groonga::Hash.create(:name => "bookmarks",
                         :path => table_path.to_s) do
    end

    assert_raise(TypeError) do
      Groonga::PatriciaTrie.open(:name => "bookmarks",
                                 :path => table_path.to_s)
    end
  end

  def test_new
    table_path = @tables_dir + "table"
    assert_raise(Groonga::NoSuchFileOrDirectory) do
      Groonga::Hash.new(:path => table_path.to_s)
    end

    Groonga::Hash.create(:path => table_path.to_s)
    assert_not_predicate(Groonga::Hash.new(:path => table_path.to_s), :closed?)
  end

  def test_define_column
    table_path = @tables_dir + "table"
    table = Groonga::Hash.create(:name => "bookmarks",
                                 :path => table_path.to_s)
    column = table.define_column("name", "<text>",
                                 :type => "index",
                                 :compress => "zlib",
                                 :with_section => true,
                                 :with_weight => true,
                                 :with_position => true)
    assert_equal("bookmarks.name", column.name)
    assert_equal(column, table.column("name"))
  end

  def test_add_column
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :path => (@tables_dir + "bookmarks").to_s)

    description_column_path = @columns_dir + "description"
    bookmarks_description =
      bookmarks.define_column("description", "<text>",
                              :type => "index",
                              :path => description_column_path.to_s)

    books = Groonga::Hash.create(:name => "books",
                                 :path => (@tables_dir + "books").to_s)
    books_description = books.add_column("description",
                                         "<longtext>",
                                         description_column_path.to_s)
    assert_equal("books.description", books_description.name)
    assert_equal(books_description, books.column("description"))

    assert_equal(bookmarks_description, bookmarks.column("description"))
  end

  def test_column_nonexistent
    table_path = @tables_dir + "bookmarks"
    table = Groonga::Hash.create(:name => "bookmarks",
                                 :path => table_path.to_s)
    assert_nil(table.column("nonexistent"))
  end

  def test_set_value
    table_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :value_size => 512,
                                     :path => table_path.to_s)
    comment_column_path = @columns_dir + "comment"
    bookmarks_comment =
      bookmarks.define_column("comment", "<shorttext>",
                              :type => "scalar",
                              :path => comment_column_path.to_s)
    groonga = bookmarks.add("groonga")
    url = "http://groonga.org/"
    groonga.value = url
    bookmarks_comment[groonga.id] = "fulltext search engine"

    assert_equal([url, "fulltext search engine"],
                 [groonga.value[0, url.length],
                  bookmarks_comment[groonga.id]])
  end

  def test_array_set
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :value_size => 512)
    url = "http://groonga.org/"
    bookmarks["groonga"] = "#{url}\0"

    values = bookmarks.records.collect do |record|
      record.value.split(/\0/, 2)[0]
    end
    assert_equal([url], values)
  end

  def test_add_without_name
    users_path = @tables_dir + "users"
    users = Groonga::Array.create(:name => "users",
                                  :path => users_path.to_s)
    name_column_path = @columns_dir + "name"
    users_name = users.define_column("name", "<shorttext>",
                                     :path => name_column_path.to_s)
    morita = users.add
    users_name[morita.id] = "morita"
    assert_equal("morita", users_name[morita.id])
  end

  def test_add_by_id
    users_path = @tables_dir + "users"
    users = Groonga::Hash.create(:name => "users",
                                 :path => users_path.to_s)
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "bookmarks",
                                     :key_type => users,
                                     :value_size => 512,
                                     :path => bookmarks_path.to_s)
    morita = users.add("morita")
    groonga = bookmarks.add(morita.id)
    url = "http://groonga.org/"
    groonga.value = url

    assert_equal(url, groonga.value[0, url.length])
  end

  def test_columns
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "bookmarks",
                                      :path => bookmarks_path.to_s)

    uri_column = bookmarks.define_column("uri", "<shorttext>")
    comment_column = bookmarks.define_column("comment", "<text>")
    assert_equal([uri_column.name, comment_column.name].sort,
                 bookmarks.columns.collect {|column| column.name}.sort)
  end

  def test_size
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "bookmarks",
                                      :path => bookmarks_path.to_s)

    assert_equal(0, bookmarks.size)

    bookmarks.add
    bookmarks.add
    bookmarks.add

    assert_equal(3, bookmarks.size)
  end

  def test_time_column
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "bookmarks",
                                      :path => bookmarks_path.to_s)
    column = bookmarks.define_column("created_at", "<time>")

    bookmark = bookmarks.add
    now = Time.now
    bookmark["created_at"] = now
    assert_equal(now.to_a,
                 bookmark["created_at"].to_a)
  end

  def test_delete
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "bookmarks",
                                      :path => bookmarks_path.to_s)

    bookmark1 = bookmarks.add
    bookmark2 = bookmarks.add
    bookmark3 = bookmarks.add

    assert_equal(3, bookmarks.size)
    bookmarks.delete(bookmark2.id)
    assert_equal(2, bookmarks.size)
  end

  def test_remove
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "bookmarks",
                                      :path => bookmarks_path.to_s)
    assert_predicate(bookmarks_path, :exist?)
    bookmarks.remove
    assert_not_predicate(bookmarks_path, :exist?)
  end

  def test_tokenizer
    table = Groonga::Hash.create
    assert_equal(Groonga::Context.default["<token:bigram>"],
                 table.default_tokenizer)
    table.default_tokenizer = "<token:mecab>"
    assert_equal(Groonga::Context.default["<token:mecab>"],
                 table.default_tokenizer)
  end

  def test_inspect_anonymous
    path = @tables_dir + "anoymous.groonga"
    anonymous_table = Groonga::Hash.create(:path => path.to_s)
    assert_equal("#<Groonga::Hash " +
                 "id: <#{anonymous_table.id}>, " +
                 "name: (anonymous), " +
                 "path: <#{path}>, " +
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 anonymous_table.inspect)
  end

  def test_inspect_anonymous_temporary
    anonymous_table = Groonga::Hash.create
    assert_equal("#<Groonga::Hash " +
                 "id: <#{anonymous_table.id}>, " +
                 "name: (anonymous), " +
                 "path: (temporary), " +
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 anonymous_table.inspect)
  end

  def test_inspect_named
    path = @tables_dir + "named.groonga"
    named_table = Groonga::Hash.create(:name => "name", :path => path.to_s)
    assert_equal("#<Groonga::Hash " +
                 "id: <#{named_table.id}>, " +
                 "name: <name>, " +
                 "path: <#{path}>, " +
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 named_table.inspect)
  end

  def test_inspect_named_temporary
    named_table = Groonga::Hash.create(:name => "name")
    assert_equal("#<Groonga::Hash " +
                 "id: <#{named_table.id}>, " +
                 "name: <name>, " +
                 "path: (temporary), " +
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "encoding: <#{encoding.inspect}>, " +
                 "size: <0>>",
                 named_table.inspect)
  end

  def test_inspect_size
    path = @tables_dir + "named.groonga"
    contain_table = Groonga::Array.create(:name => "name", :path => path.to_s)
    3.times do
      contain_table.add
    end
    assert_equal("#<Groonga::Array " +
                 "id: <#{contain_table.id}>, " +
                 "name: <name>, " +
                 "path: <#{path}>, " +
                 "domain: <nil>, " +
                 "range: <nil>, " +
                 "size: <3>>",
                 contain_table.inspect)
  end

  def test_temporary_add
    table = Groonga::Hash.create(:key_type => "<shorttext>")
    assert_equal(0, table.size)
    table.add("key")
    assert_equal(1, table.size)
  end

  def test_search
    users = Groonga::Array.create(:name => "<users>")
    user_name = users.define_column("name", "<shorttext>")

    bookmarks = Groonga::Hash.create(:name => "<bookmarks>",
                                     :key_type => "<shorttext>")
    bookmark_user_id = bookmarks.define_column("user_id", users)

    daijiro = users.add
    daijiro["name"] = "daijiro"
    gunyarakun = users.add
    gunyarakun["name"] = "gunyarakun"

    groonga = bookmarks.add("http://groonga.org/")
    groonga["user_id"] = daijiro

    records = bookmarks.search("http://groonga.org/")
    assert_equal(["daijiro"],
                 records.records.collect {|record| record[".user_id.name"]})
  end

  def test_each
    users = Groonga::Array.create(:name => "<users>")
    user_name = users.define_column("name", "<shorttext>")

    names = ["daijiro", "gunyarakun", "yu"]
    names.each do |name|
      user = users.add
      user["name"] = name
    end

    assert_equal(names.sort, users.collect {|user| user["name"]}.sort)
  end

  def test_truncate
    users = Groonga::Array.create(:name => "<users>")
    users.add
    users.add
    users.add
    assert_equal(3, users.size)
    assert_nothing_raised do
      users.truncate
    end
    # assert_equal(0, users.size) # truncate isn't implemented in groonga.
  end

  def test_encoding
    assert_equal(Groonga::Encoding.default,
                 Groonga::Hash.create.encoding)
    assert_equal(Groonga::Encoding.default,
                 Groonga::PatriciaTrie.create.encoding)

    array = Groonga::Array.create
    assert_false(array.respond_to?(:encoding))
  end

  def test_sort
    bookmarks = Groonga::Array.create(:name => "<bookmarks>")
    id_column = bookmarks.define_column("id", "<int>")
    100.times do |i|
      bookmark = bookmarks.add
      bookmark["id"] = i + 100
    end

    results = bookmarks.sort([
                              {
                                :key => "id",
                                :order => :descending,
                              },
                             ],
                             :limit => 20)
    results = results.records(:order => :ascending).collect do |record|
      id_column[record.value.unpack("i")[0]]
    end
    assert_equal((180..199).to_a.reverse, results)
  end

  def test_sort_without_limit
    bookmarks = Groonga::Array.create(:name => "<bookmarks>")
    id_column = bookmarks.define_column("id", "<int>")
    100.times do |i|
      bookmark = bookmarks.add
      bookmark["id"] = i + 100
    end

    results = bookmarks.sort([
                              {
                                :key => "id",
                                :order => :descending,
                              },
                             ])
    results = results.records(:order => :ascending).collect do |record|
      id_column[record.value.unpack("i")[0]]
    end
    assert_equal((100..199).to_a.reverse, results)
  end
end
