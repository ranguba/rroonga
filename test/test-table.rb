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
end
