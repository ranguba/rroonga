# Copyright (C) 2009-2018  Kouhei Sutou <kou@clear-code.com>
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

class DatabaseTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_create
    assert_nil(Groonga::Context.default.database)

    assert_not_predicate(@database_path, :exist?)
    Groonga::Database.create(:path => @database_path.to_s) do |database|
      assert_predicate(@database_path, :exist?)
      assert_not_predicate(database, :closed?)

      assert_equal(database, Groonga::Context.default.database)
    end
  end

  def test_temporary
    before_files = @tmp_dir.children
    database = Groonga::Database.create
    assert_nil(database.name)
    assert_equal(before_files, @tmp_dir.children)
  end

  def test_open
    create_context = Groonga::Context.new
    database = nil
    create_context.create_database(@database_path.to_s) do |_database|
      database = _database
    end
    assert_predicate(database, :closed?)
    create_context.close

    called = false
    Groonga::Database.open(@database_path.to_s) do |_database|
      database = _database
      assert_not_predicate(database, :closed?)
      called = true
    end
    assert_true(called)
    assert_predicate(database, :closed?)
  end

  def test_close
    create_context = Groonga::Context.new
    create_context.create_database(@database_path.to_s) do
    end
    create_context.close

    database = nil
    Groonga::Database.open(@database_path.to_s) do |_database|
      database = _database
      assert_not_predicate(database, :closed?)
    end
    assert_predicate(database, :closed?)
  end

  def test_new
    assert_raise(Groonga::NoSuchFileOrDirectory) do
      new_context = Groonga::Context.new
      Groonga::Database.new(@database_path.to_s, :context => new_context)
    end

    create_context = Groonga::Context.new
    create_context.create_database(@database_path.to_s) do
    end

    database = Groonga::Database.new(@database_path.to_s)
    begin
      assert_not_predicate(database, :closed?)
    ensure
      database.close
    end
  end

  def test_each
    setup_database
    default_object_names = @database.collect {|object| object.name}.sort
    assert do
      default_object_names.include?("Bool")
    end
  end

  def test_each_without_block
    setup_database
    default_object_names = @database.each.collect {|object| object.name}.sort
    assert do
      default_object_names.include?("Bool")
    end
  end

  def test_encoding
    assert_equal(Groonga::Encoding.default,
                 Groonga::Database.create.encoding)
  end

  def test_lock
    database = Groonga::Database.create

    assert_not_predicate(database, :locked?)
    database.lock
    assert_predicate(database, :locked?)
    database.unlock
    assert_not_predicate(database, :locked?)
  end

  def test_lock_failed
    database = Groonga::Database.create

    database.lock
    assert_raise(Groonga::ResourceDeadlockAvoided) do
      database.lock
    end
  end

  def test_lock_block
    database = Groonga::Database.create

    assert_not_predicate(database, :locked?)
    database.lock do
      assert_predicate(database, :locked?)
    end
    assert_not_predicate(database, :locked?)
  end

  def test_clear_lock
    database = Groonga::Database.create

    assert_not_predicate(database, :locked?)
    database.lock
    assert_predicate(database, :locked?)
    database.clear_lock
    assert_not_predicate(database, :locked?)
  end

  def test_touch
    database = Groonga::Database.create
    assert_nothing_raised do
      database.touch
    end
  end

  def test_defrag
    setup_database
    Groonga::Schema.define do |schema|
      schema.create_table("Users") do |table|
        table.short_text("name")
        table.short_text("address")
      end
    end
    users = context["Users"]
    large_data = "x" * (2 ** 16)
    100.times do |i|
      users.add(:name => "user #{i}" + large_data,
                :address => "address #{i}" + large_data)
    end
    assert_equal(2, @database.defrag)
  end

  def test_recover
    setup_database
    Groonga::Schema.define do |schema|
      schema.create_table("Users") do |table|
        table.short_text("name")
      end

      schema.create_table("Terms",
                          :type => :patricia_trie,
                          :key_type => :short_text,
                          :default_tokenizer => "TokenBigram",
                          :normalizer => "NormalizerAuto") do |table|
        table.index("Users.name")
      end
    end

    index = context["Terms.Users_name"]
    index.lock
    assert do
      index.locked?
    end
    @database.recover
    assert do
      not index.locked?
    end
  end

  def test_unmap
    setup_database
    Groonga::Schema.define do |schema|
      schema.create_table("Users") do |table|
        table.short_text("name")
      end
    end

    @database.unmap
  end

  def test_tables
    setup_database
    Groonga::Schema.define do |schema|
      schema.create_table("HashTable") do |table|
      end

      schema.create_table("PatriciaTrie",
                          :type => :patricia_trie) do |table|
      end

      schema.create_table("DoubleArrayTrie",
                          :type => :double_array_trie) do |table|
      end
    end

    assert_equal(["HashTable", "PatriciaTrie", "DoubleArrayTrie"].sort,
                 @database.tables.collect(&:name).sort)
  end

  def test_plugin_paths
    setup_database
    context.register_plugin("query_expanders/tsv")
    assert_equal(["query_expanders/tsv#{Groonga::Plugin.suffix}"],
                 @database.plugin_paths)
  end

  def test_reindex
    setup_database
    Groonga::Schema.define do |schema|
      schema.create_table("Memos",
                          :type => :array) do |table|
        table.column("content", "Text")
      end
      schema.create_table("Terms",
                          :type => :patricia_trie,
                          :key_type => "ShortText",
                          :default_tokenizer => "TokenBigram",
                          :normalizer => "NormalizerAuto") do |table|
        table.index("Memos.content")
      end
    end

    memos = context["Memos"]
    memos.add(:content => "This is a memo")

    terms = context["Terms"]
    terms.delete("this")

    assert_equal([
                   "a",
                   "is",
                   "memo",
                 ],
                 terms.collect(&:_key).sort)

    @database.reindex

    assert_equal([
                   "a",
                   "is",
                   "memo",
                   "this",
                 ],
                 terms.collect(&:_key).sort)
  end

  def test_remove_force
    setup_database
    table_name = "Bookmarks"
    table = Groonga::Array.create(:name => table_name)
    table_path = Pathname.new(table.path)
    @database.unmap
    table_path.open("w") do |file|
      file.print("BROKEN")
    end
    assert_raise(Groonga::IncompatibleFileFormat) do
      context[table_name]
    end
    assert do
      table_path.exist?
    end
    @database.remove_force(table_name)
    assert do
      not table_path.exist?
    end
  end

  class RemoveTest < self
    setup :setup_database

    def test_referenced_table
      Groonga::Schema.define do |schema|
        schema.create_table("Names",
                            :type => :hash,
                            :key_type => :short_text) do |table|
        end

        schema.create_table("Shops",
                            :type => :hash,
                            :key_type => "Names") do |table|
        end
      end

      path = @database.path
      @database.remove
      assert_false(File.exist?(path))
    end

    def test_referenced_column
      Groonga::Schema.define do |schema|
        schema.create_table("Categories",
                            :type => :hash,
                            :key_type => :short_text) do |table|
        end

        schema.create_table("Contents") do |table|
          table.reference("category", "Categories")
        end
      end

      path = @database.path
      @database.remove
      assert_false(File.exist?(path))
    end

    def test_indexed_table
      Groonga::Schema.define do |schema|
        schema.create_table("Categories",
                            :type => :hash,
                            :key_type => :short_text) do |table|
        end

        schema.create_table("Terms") do |table|
          table.index("Categories._key")
        end
      end

      path = @database.path
      @database.remove
      assert_false(File.exist?(path))
    end

    def test_indexed_column
      Groonga::Schema.define do |schema|
        schema.create_table("Categories",
                            :type => :hash,
                            :key_type => :short_text) do |table|
          table.text("content")
        end

        schema.create_table("Terms") do |table|
          table.index("Categories.content")
        end
      end

      path = @database.path
      @database.remove
      assert_false(File.exist?(path))
    end
  end

  class DiskUsageTest < self
    setup :setup_database

    def test_empty
      paths = [
        @database.path,
        "#{@database.path}.conf",
        "#{@database.path}.001",
        "#{@database.path}.0000000",
      ]
      expected_usage = paths.inject(0) do |previous, path|
        previous + File.size(path)
      end
      assert_equal(expected_usage,
                   @database.disk_usage)
    end
  end

  class LastModifiedTest < self
    setup :setup_database

    def test_touch
      assert_equal(Time.at(0), @database.last_modified)
      @database.touch
      assert_equal(Time.now.sec, @database.last_modified.sec)
    end
  end

  class DirtyTest < self
    setup :setup_database

    def test_dirty?
      assert do
        not @database.dirty?
      end
      @database.touch
      assert do
        @database.dirty?
      end
    end
  end

  class CorruptTest < self
    setup :setup_database

    def test_corrupt?
      assert do
        not @database.corrupt?
      end
    end
  end
end
