class TableTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    Groonga::Context.default = nil

    @db_path = @tmp_dir + "db"
    @database = Groonga::Database.create(:path => @db_path.to_s)

    @tables_dir = @tmp_dir + "tables"
    FileUtils.mkdir_p(@tables_dir.to_s)
  end

  def test_create
    table_path = @tables_dir + "table"
    assert_not_predicate(table_path, :exist?)
    Groonga::Table.create(:name => "bookmarks",
                          :path => table_path.to_s)
    assert_predicate(table_path, :exist?)
  end

  def test_temporary
    Groonga::Table.create
    assert_equal([], @tables_dir.children)
  end

  def test_open
    table_path = @tables_dir + "table"
    table = Groonga::Table.create(:path => table_path.to_s)

    called = false
    Groonga::Table.open(table_path.to_s) do |_table|
      table = _table
      assert_not_predicate(table, :closed?)
      called = true
    end
    assert_predicate(table, :closed?)
  end

  def test_new
    table_path = @tables_dir + "table"
    assert_raise(Groonga::NoSuchFileOrDirectory) do
      Groonga::Table.new(table_path.to_s)
    end

    Groonga::Table.create(:path => table_path.to_s)
    assert_not_predicate(Groonga::Table.new(table_path.to_s), :closed?)
  end
end
