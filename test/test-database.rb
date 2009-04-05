class DatabaseTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    Groonga::Context.default = nil
  end

  def test_create
    assert_nil(Groonga::Context.default.database)

    db_path = @tmp_dir + "db"
    assert_not_predicate(db_path, :exist?)
    database = Groonga::Database.create(db_path.to_s)
    assert_predicate(db_path, :exist?)

    assert_equal(database, Groonga::Context.default.database)
  end
end
