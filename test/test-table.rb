# Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>
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
    table_path = @tables_dir + "bookmarks"
    assert_not_predicate(table_path, :exist?)
    table = Groonga::PatriciaTrie.create(:name => "Bookmarks",
                                         :path => table_path.to_s)
    assert_equal("Bookmarks", table.name)
    assert_predicate(table_path, :exist?)
  end

  def test_temporary
    table = Groonga::PatriciaTrie.create
    assert_nil(table.name)
    assert_predicate(table, :temporary?)
    assert_not_predicate(table, :persistent?)
    assert_equal([], @tables_dir.children)
  end

  def test_define_column
    table_path = @tables_dir + "bookmarks"
    table = Groonga::Hash.create(:name => "Bookmarks",
                                 :path => table_path.to_s)
    column = table.define_column("name", "Text")
    assert_equal("Bookmarks.name", column.name)
    assert_equal(column, table.column("name"))
  end

  def test_temporary_table_define_column_default_persistent
    table = Groonga::Hash.create
    assert_raise(Groonga::InvalidArgument) do
      table.define_column("name", "ShortText")
    end
  end

  def test_temporary_table_define_index_column_default_persistent
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    terms = Groonga::Hash.create
    assert_raise(Groonga::InvalidArgument) do
      terms.define_index_column("url", bookmarks)
    end
  end

  def test_define_column_default_persistent
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    real_name = bookmarks.define_column("real_name", "ShortText")
    assert_predicate(real_name, :persistent?)
  end

  def test_define_column_not_persistent
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    real_name = bookmarks.define_column("real_name", "ShortText",
                                        :persistent => false)
    assert_predicate(real_name, :temporary?)
  end

  def test_define_column_not_persistent_and_path
    column_path = @tables_dir + "bookmakrs.real_name.column"

    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    message = "should not pass :path if :persistent is false: <#{column_path}>"
    assert_raise(ArgumentError.new(message)) do
      bookmarks.define_column("real_name", "ShortText",
                              :path => column_path.to_s,
                              :persistent => false)
    end
  end

  def test_define_index_column_default_persistent
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    terms = Groonga::Hash.create(:name => "Terms")
    real_name = terms.define_index_column("real_name", bookmarks)
    assert_predicate(real_name, :persistent?)
  end

  def test_define_index_column_not_persistent
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    terms = Groonga::Hash.create(:name => "Terms")
    real_name = terms.define_index_column("real_name", bookmarks,
                                          :persistent => false)
    assert_predicate(real_name, :temporary?)
  end

  def test_define_index_column_not_persistent_and_path
    column_path = @tables_dir + "bookmakrs.real_name.column"

    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    terms = Groonga::Hash.create(:name => "Terms")
    message = "should not pass :path if :persistent is false: <#{column_path}>"
    assert_raise(ArgumentError.new(message)) do
      terms.define_index_column("real_name", bookmarks,
                                :path => column_path.to_s,
                                :persistent => false)
    end
  end

  def test_define_index_column
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    bookmarks.define_column("content", "Text")
    terms = Groonga::Hash.create(:name => "Terms")
    terms.default_tokenizer = "TokenBigram"
    index = terms.define_index_column("content_index", bookmarks,
                                      :with_section => true,
                                      :source => "Bookmarks.content")
    bookmarks.add("google", :content => "Search engine")
    assert_equal(["google"],
                 index.search("engine").collect {|record| record.key.key})
  end

  def test_column_nonexistent
    table_path = @tables_dir + "bookmarks"
    table = Groonga::Hash.create(:name => "Bookmarks",
                                 :path => table_path.to_s)
    assert_nil(table.column("nonexistent"))
  end

  def test_set_value
    table_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "Bookmarks",
                                     :value_type => "Int32",
                                     :path => table_path.to_s)
    comment_column_path = @columns_dir + "comment"
    bookmarks_comment =
      bookmarks.define_column("comment", "ShortText",
                              :type => "scalar",
                              :path => comment_column_path.to_s)
    groonga = bookmarks.add("groonga")
    groonga.value = 29
    bookmarks_comment[groonga.id] = "fulltext search engine"

    assert_equal([29, "fulltext search engine"],
                 [groonga.value, bookmarks_comment[groonga.id]])
  end

  def test_array_set
    bookmarks = Groonga::Hash.create(:name => "Bookmarks",
                                     :value_type => "Int32")
    bookmarks.set_value("groonga", 29)

    values = bookmarks.records.collect do |record|
      record.value
    end
    assert_equal([29], values)
  end

  def test_add_without_name
    users_path = @tables_dir + "users"
    users = Groonga::Array.create(:name => "Users",
                                  :path => users_path.to_s)
    name_column_path = @columns_dir + "name"
    users_name = users.define_column("name", "ShortText",
                                     :path => name_column_path.to_s)
    morita = users.add
    users_name[morita.id] = "morita"
    assert_equal("morita", users_name[morita.id])
  end

  def test_add_by_id
    users_path = @tables_dir + "users"
    users = Groonga::Hash.create(:name => "Users",
                                 :path => users_path.to_s)

    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Hash.create(:name => "Bookmarks",
                                     :key_type => users,
                                     :value_type => "Int32",
                                     :path => bookmarks_path.to_s)
    morita = users.add("morita")
    groonga = bookmarks.add(morita.id)
    groonga.value = 29
    assert_equal(29, groonga.value)
  end

  def test_columns
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                      :path => bookmarks_path.to_s)

    uri_column = bookmarks.define_column("uri", "ShortText")
    comment_column = bookmarks.define_column("comment", "Text")
    assert_equal([uri_column.name, comment_column.name].sort,
                 bookmarks.columns.collect {|column| column.name}.sort)
  end

  def test_column_by_symbol
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                      :path => bookmarks_path.to_s)

    uri_column = bookmarks.define_column("uri", "Text")
    assert_equal(uri_column, bookmarks.column(:uri))
  end

  def test_size
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                      :path => bookmarks_path.to_s)

    assert_equal(0, bookmarks.size)

    bookmarks.add
    bookmarks.add
    bookmarks.add

    assert_equal(3, bookmarks.size)
  end

  def test_empty?
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                      :path => bookmarks_path.to_s)

    assert_predicate(bookmarks, :empty?)
    bookmarks.add
    assert_not_predicate(bookmarks, :empty?)
  end

  def test_path
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                      :path => bookmarks_path.to_s)
    assert_equal(bookmarks_path.to_s, bookmarks.path)
  end

  def test_time_column
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                      :path => bookmarks_path.to_s)
    bookmarks.define_column("created_at", "Time")

    bookmark = bookmarks.add
    now = Time.now
    bookmark["created_at"] = now
    assert_equal(now.to_a,
                 bookmark["created_at"].to_a)
  end

  def test_delete
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                      :path => bookmarks_path.to_s)

    bookmark_records = []
    bookmark_records << bookmarks.add
    bookmark_records << bookmarks.add
    bookmark_records << bookmarks.add

    assert_equal(3, bookmarks.size)
    bookmarks.delete(bookmark_records[1].id)
    assert_equal(2, bookmarks.size)
  end

  def test_remove
    bookmarks_path = @tables_dir + "bookmarks"
    bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                      :path => bookmarks_path.to_s)
    assert_predicate(bookmarks_path, :exist?)
    bookmarks.remove
    assert_not_predicate(bookmarks_path, :exist?)
  end

  def test_temporary_add
    table = Groonga::Hash.create(:key_type => "ShortText")
    assert_equal(0, table.size)
    table.add("key")
    assert_equal(1, table.size)
  end

  def test_each
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name", "ShortText")

    names = ["daijiro", "gunyarakun", "yu"]
    names.each do |name|
      user = users.add
      user["name"] = name
    end

    assert_equal(names.sort, users.collect {|user| user["name"]}.sort)
  end

  def test_truncate
    users = Groonga::Array.create(:name => "Users")
    users.add
    users.add
    users.add
    assert_equal(3, users.size)
    assert_nothing_raised do
      users.truncate
    end
    assert_equal(0, users.size)
  end

  def test_sort
    bookmarks = create_bookmarks
    add_shuffled_ids(bookmarks)

    results = bookmarks.sort([
                              {
                                :key => "id",
                                :order => :descending,
                              },
                             ],
                             :limit => 20)
    assert_equal((180..199).to_a.reverse,
                 results.collect {|record| record["id"]})
  end

  def test_sort_simple
    bookmarks = create_bookmarks
    add_shuffled_ids(bookmarks)

    results = bookmarks.sort(["id"], :limit => 20)
    assert_equal((100..119).to_a,
                 results.collect {|record| record["id"]})
  end

  def test_sort_by_array
    bookmarks = create_bookmarks
    add_shuffled_ids(bookmarks)

    results = bookmarks.sort([["id", "descending"]], :limit => 20)
    assert_equal((180..199).to_a.reverse,
                 results.collect {|record| record["id"]})
  end

  def test_sort_without_limit_and_offset
    bookmarks = create_bookmarks
    add_shuffled_ids(bookmarks)

    results = bookmarks.sort([{:key => "id", :order => :descending}])
    assert_equal((100..199).to_a.reverse,
                 results.collect {|record| record["id"]})
  end

  def test_sort_with_limit
    bookmarks = create_bookmarks
    add_shuffled_ids(bookmarks)

    results = bookmarks.sort([{:key => "id", :order => :descending}],
                             :limit => 20)
    assert_equal((180..199).to_a.reverse,
                 results.collect {|record| record["id"]})
  end

  def test_sort_with_offset
    bookmarks = create_bookmarks
    add_shuffled_ids(bookmarks)

    results = bookmarks.sort([{:key => "id", :order => :descending}],
                             :offset => 20)
    assert_equal((100..179).to_a.reverse,
                 results.collect {|record| record["id"]})
  end

  def test_sort_with_limit_and_offset
    bookmarks = create_bookmarks
    add_shuffled_ids(bookmarks)

    results = bookmarks.sort([{:key => "id", :order => :descending}],
                             :limit => 20, :offset => 20)
    assert_equal((160..179).to_a.reverse,
                 results.collect {|record| record["id"]})
  end

  def test_sort_with_nonexistent_key
    bookmarks = create_bookmarks
    add_shuffled_ids(bookmarks)
    message = "no such column: <\"nonexistent\">: <#{bookmarks.inspect}>"
    assert_raise(Groonga::NoSuchColumn.new(message)) do
      bookmarks.sort([{:key => "nonexistent", :order => :descending}])
    end
  end

  def test_sort_with_nonexistent_value
    bookmarks = create_bookmarks
    bookmarks.define_column("uri", "ShortText")
    empty1 = bookmarks.add
    groonga = bookmarks.add(:uri => "http://groonga.org/")
    empty2 = bookmarks.add
    assert_equal([groonga, empty1, empty2],
                 bookmarks.sort([{:key => "uri", :order => :descending}]))
  end

  def test_group
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    bookmarks.define_column("title", "Text")
    comments = Groonga::Array.create(:name => "Comments")
    comments.define_column("bookmark", bookmarks)
    comments.define_column("content", "Text")
    comments.define_column("issued", "Int32")

    groonga = bookmarks.add("http://groonga.org/", :title => "groonga")
    ruby = bookmarks.add("http://ruby-lang.org/", :title => "Ruby")

    now = Time.now.to_i
    comments.add(:bookmark => groonga,
                 :content => "full-text search",
                 :issued => now)
    comments.add(:bookmark => groonga,
                 :content => "column store",
                 :issued => now)
    comments.add(:bookmark => ruby,
                 :content => "object oriented script language",
                 :issued => now)

    records = comments.select do |record|
      record["issued"] > 0
    end
    assert_equal([[2, "groonga", "http://groonga.org/"],
                  [1, "Ruby", "http://ruby-lang.org/"]],
                 records.group(".bookmark").collect do |record|
                   bookmark = record.key
                   [record.n_sub_records,
                    bookmark["title"],
                    bookmark.key]
                 end)
    assert_equal([[2, "groonga", "http://groonga.org/"],
                  [1, "Ruby", "http://ruby-lang.org/"]],
                 records.group(%w".bookmark").collect do |record|
                   bookmark = record.key
                   [record.n_sub_records,
                    bookmark["title"],
                    bookmark.key]
                 end)
  end

  def test_group_with_unknown_key
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    message = "unknown group key: <\"nonexistent\">: <#{bookmarks.inspect}>"
    assert_raise(ArgumentError.new(message)) do
      bookmarks.group("nonexistent")
    end
  end

  def test_union!
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    bookmarks.define_column("title", "ShortText")

    bookmarks.add("http://groonga.org/", :title => "groonga")
    bookmarks.add("http://ruby-lang.org/", :title => "Ruby")

    ruby_bookmarks = bookmarks.select {|record| record["title"] == "Ruby"}
    groonga_bookmarks = bookmarks.select {|record| record["title"] == "groonga"}
    assert_equal(["Ruby", "groonga"],
                 ruby_bookmarks.union!(groonga_bookmarks).collect do |record|
                   record[".title"]
                 end)
  end

  def test_intersection!
    bookmarks = Groonga::Hash.create(:name => "bookmarks")
    bookmarks.define_column("title", "ShortText")

    bookmarks.add("http://groonga.org/", :title => "groonga")
    bookmarks.add("http://ruby-lang.org/", :title => "Ruby")

    ruby_bookmarks = bookmarks.select {|record| record["title"] == "Ruby"}
    all_bookmarks = bookmarks.select
    assert_equal(["Ruby"],
                 ruby_bookmarks.intersection!(all_bookmarks).collect do |record|
                   record[".title"]
                 end)
  end

  def test_difference!
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    bookmarks.define_column("title", "ShortText")

    bookmarks.add("http://groonga.org/", :title => "groonga")
    bookmarks.add("http://ruby-lang.org/", :title => "Ruby")

    ruby_bookmarks = bookmarks.select {|record| record["title"] == "Ruby"}
    all_bookmarks = bookmarks.select
    assert_equal(["groonga"],
                 all_bookmarks.difference!(ruby_bookmarks).collect do |record|
                   record[".title"]
                 end)
  end

  def test_merge!
    bookmarks = Groonga::Hash.create(:name => "Bookmarks")
    bookmarks.define_column("title", "ShortText")

    bookmarks.add("http://groonga.org/", :title => "groonga")
    bookmarks.add("http://ruby-lang.org/", :title => "Ruby")

    ruby_bookmarks = bookmarks.select {|record| (record["title"] == "Ruby") &
                                                (record["title"] == "Ruby") }
    all_bookmarks = bookmarks.select
    assert_equal([["groonga", 1], ["Ruby", 2]],
                 all_bookmarks.merge!(ruby_bookmarks).collect do |record|
                   [record[".title"], record.score]
                 end)
  end

  def test_lock
    bookmarks = Groonga::Array.create(:name => "Bookmarks")
    assert_not_predicate(bookmarks, :locked?)
    bookmarks.lock
    assert_predicate(bookmarks, :locked?)
    bookmarks.unlock
    assert_not_predicate(bookmarks, :locked?)
  end

  def test_lock_failed
    bookmarks = Groonga::Array.create(:name => "Bookmarks")
    bookmarks.lock
    assert_raise(Groonga::ResourceDeadlockAvoided) do
      bookmarks.lock
    end
  end

  def test_lock_block
    bookmarks = Groonga::Array.create(:name => "Bookmarks")
    assert_not_predicate(bookmarks, :locked?)
    bookmarks.lock do
      assert_predicate(bookmarks, :locked?)
    end
    assert_not_predicate(bookmarks, :locked?)
  end

  def test_clear_lock
    bookmarks = Groonga::Array.create(:name => "Bookmarks")
    assert_not_predicate(bookmarks, :locked?)
    bookmarks.lock
    assert_predicate(bookmarks, :locked?)
    bookmarks.clear_lock
    assert_not_predicate(bookmarks, :locked?)
  end

  def test_auto_record_register
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    books = Groonga::Hash.create(:name => "Books",
                                 :key_type => "ShortText")
    users.define_column("book", "Books")

    assert_equal([], books.select.collect {|book| book.key})
    users.add("ryoqun", :book => "XP")
    assert_equal([books["XP"]],
                 books.select.collect {|book| book.key})
  end

  def test_get_common_prefix_column
    users = Groonga::Array.create(:name => "Users")
    users.define_column("name_kana", "ShortText")
    name = users.define_column("name", "ShortText")

    assert_equal(name, users.column("name"))
  end

  def test_empty_reference_column_value
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    books = Groonga::Hash.create(:name => "Books",
                                 :key_type => "ShortText")
    users.define_column("book", books)
    users.add("morita", :book => "")
    assert_equal({"_id" => 1, "_key" => "morita", "book" => nil},
                 users["morita"].attributes)
  end

  def test_have_column
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    users.define_column("name", "ShortText")
    assert_true(users.have_column?("name"), "name")
    assert_false(users.have_column?("description"), "description")
  end

  def test_have_column_id
    users = Groonga::Array.create(:name => "Users")
    assert_true(users.have_column?(:_id))
  end

  def test_have_column_key_hash
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    assert_true(users.have_column?(:_key))
  end

  def test_have_column_key_array
    users = Groonga::Array.create(:name => "Users")
    assert_false(users.have_column?(:_key))
  end

  def test_have_column_value_hash_with_value_type
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText",
                                 :value_type => "Int32")
    assert_true(users.have_column?(:_value))
  end

  def test_have_column_value_hash_without_value_type
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    assert_false(users.have_column?(:_value))
  end

  def test_have_column_value_array
    users = Groonga::Array.create(:name => "Users")
    assert_false(users.have_column?(:_value))
  end

  def test_have_column_nsubrecs_existent
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    assert_true(users.select.have_column?(:_nsubrecs))
  end

  def test_have_column_nsubrecs_nonexistent
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    assert_false(users.have_column?(:_nsubrecs))
  end

  def test_have_column_score_existent
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    assert_true(users.select.have_column?(:_score))
  end

  def test_have_column_score_nonexistent
    users = Groonga::Hash.create(:name => "Users",
                                 :key_type => "ShortText")
    assert_false(users.have_column?(:_score))
  end

  def test_exist?
    users = Groonga::Hash.create(:name => "Users")
    morita = users.add("morita")
    assert_true(users.exist?(morita.id))
    assert_false(users.exist?(morita.id + 1))
  end

  def test_builtin?
    bookmarks = Groonga::PatriciaTrie.create(:name => "Bookmarks")
    assert_not_predicate(bookmarks, :builtin?)
  end

  private
  def create_bookmarks
    bookmarks = Groonga::Array.create(:name => "Bookmarks")
    bookmarks.define_column("id", "Int32")
    bookmarks
  end

  def add_shuffled_ids(bookmarks)
    srand(Time.now.to_i)
    (0...100).to_a.shuffle.each do |i|
      bookmark = bookmarks.add
      bookmark["id"] = i + 100
    end
    bookmarks
  end
end
