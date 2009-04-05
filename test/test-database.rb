class DatabaseTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    Groonga::Context.default = nil
  end

  def test_create
    assert_nil(Groonga::Context.default.database)

    db_path = @tmp_dir + "db"
    assert_not_predicate(db_path, :exist?)
    database = Groonga::Database.create(:path => db_path.to_s)
    assert_predicate(db_path, :exist?)
    assert_not_predicate(database, :closed?)

    assert_equal(database, Groonga::Context.default.database)
  end

  def test_temporary
    Groonga::Database.create
    assert_equal([], @tmp_dir.children)
  end

  def test_open
    db_path = @tmp_dir + "db"
    database = Groonga::Database.create(:path => db_path.to_s)

    called = false
    Groonga::Database.open(db_path.to_s) do |_database|
      database = _database
      assert_not_predicate(database, :closed?)
      called = true
    end
    assert_predicate(database, :closed?)
  end

  def test_new
    db_path = @tmp_dir + "db"
    assert_raise(Groonga::NoMemoryAvailable) do
      Groonga::Database.new(db_path.to_s)
    end

    database = Groonga::Database.create(:path => db_path.to_s)

    assert_not_predicate(Groonga::Database.new(db_path.to_s), :closed?)
  end
end
