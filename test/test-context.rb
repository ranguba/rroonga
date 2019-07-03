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

class ContextTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_default
    context = Groonga::Context.default
    assert_equal(Groonga::Encoding.default, context.encoding)
  end

  def test_default_options
    Groonga::Context.default_options = {
      :encoding => :utf8,
    }
    context = Groonga::Context.default
    assert_equal(Groonga::Encoding::UTF8, context.encoding)
  end

  def test_create_database
    assert_not_predicate(@database_path, :exist?)
    context = Groonga::Context.new
    assert_equal(nil, context.database)
    database = context.create_database(@database_path.to_s)
    assert_predicate(@database_path, :exist?)
    assert_not_predicate(database, :closed?)
    assert_equal(database, context.database)
  end

  def test_create_temporary_database
    before_files = @tmp_dir.children
    context = Groonga::Context.new
    database = context.create_database
    assert_nil(database.name)
    assert_equal(before_files, @tmp_dir.children)
  end

  def test_open_database
    create_context = Groonga::Context.new
    database = nil
    create_context.create_database(@database_path.to_s) do |_database|
      database = _database
    end
    assert_predicate(database, :closed?)
    create_context.close

    called = false
    open_context = Groonga::Context.new
    open_context.open_database(@database_path.to_s) do |_database|
      database = _database
      assert_not_predicate(database, :closed?)
      called = true
    end
    assert_true(called)
    assert_predicate(database, :closed?)
  end

  def test_encoding
    context = Groonga::Context.new
    assert_equal(Groonga::Encoding.default, context.encoding)

    context = Groonga::Context.new(:encoding => :utf8)
    assert_equal(Groonga::Encoding::UTF8, context.encoding)
  end

  def test_inspect
    context = Groonga::Context.new(:encoding => Groonga::Encoding::UTF8)
    assert_equal("#<Groonga::Context " +
                 "encoding: <:utf8>, " +
                 "database: <nil>>",
                 context.inspect)
  end

  def test_inspect_with_database
    db = Groonga::Database.create
    context = Groonga::Context.default
    assert_equal("#<Groonga::Context " +
                 "encoding: <#{Groonga::Encoding.default.inspect}>, " +
                 "database: <#{db.inspect}>>",
                 context.inspect)
  end

  def test_array_reference_by_string
    Groonga::Database.create
    context = Groonga::Context.default
    assert_equal("Int32", context["<int>"].name)
  end

  def test_array_reference_by_symbol
    Groonga::Database.create
    context = Groonga::Context.default
    assert_equal("Bool", context[:Bool].name)
  end

  def test_shortcut_access
    Groonga::Database.create
    assert_equal("ShortText", Groonga["ShortText"].name)
  end

  def test_support_zlib?
    assert_boolean(Groonga::Context.default.support_zlib?)
  end

  def test_support_lzo?
    assert_false(Groonga::Context.default.support_lzo?)
  end

  def test_support_lz4?
    assert_boolean(Groonga::Context.default.support_lz4?)
  end

  def test_support_zstd?
    assert_boolean(Groonga::Context.default.support_zstd?)
  end

  def test_match_escalation_threshold
    assert_equal(0, context.match_escalation_threshold)
    context.match_escalation_threshold = -1
    assert_equal(-1, context.match_escalation_threshold)
  end

  sub_test_case("#force_match_escalation?") do
    def setup
      default_force_match_escalation = context.force_match_escalation?
      begin
        yield
      ensure
        context.force_match_escalation = default_force_match_escalation
      end
    end

    def test_true
      context.force_match_escalation = true
      assert do
        context.force_match_escalation?
      end
    end

    def test_false
      context.force_match_escalation = false
      assert do
        not context.force_match_escalation?
      end
    end
  end

  def test_close
    context = Groonga::Context.new
    assert_false(context.closed?)
    context.close
    assert_true(context.closed?)
  end

  def test_opened?
    Groonga::Database.create
    context = Groonga::Context.default
    assert do
      context.opened?(Groonga::Type::SHORT_TEXT)
    end
  end

  class RestoreTest < self
    def test_simple
      commands = <<EOD
table_create Items TABLE_HASH_KEY ShortText
column_create Items title COLUMN_SCALAR Text
EOD
      restore(commands)

      assert_equal(commands, dump)
    end

    def test_continuation_lines
      dumped_commands = <<-EOD
table_create Items TABLE_HASH_KEY\\
 ShortText
EOD
      restore(dumped_commands)

      assert_equal(<<-EOC, dump)
table_create Items TABLE_HASH_KEY ShortText
EOC
    end

    def test_empty_line
      restore(<<-EOC)
table_create Items TABLE_HASH_KEY ShortText

column_create Items title COLUMN_SCALAR Text

EOC

      assert_equal(<<-EOC, dump)
table_create Items TABLE_HASH_KEY ShortText
column_create Items title COLUMN_SCALAR Text
EOC
    end

    def test_comment
      restore(<<-EOC)
table_create Items TABLE_HASH_KEY ShortText
# column_create Items url COLUMN_SCALAR ShortText
column_create Items title COLUMN_SCALAR Text
EOC

      assert_equal(<<-EOC, dump)
table_create Items TABLE_HASH_KEY ShortText
column_create Items title COLUMN_SCALAR Text
EOC
    end

    def test_block
      table_create = "table_create Items TABLE_HASH_KEY ShortText"
      column_create = "column_create Items title COLUMN_SCALAR Text"
      commands = <<-COMMANDS
#{table_create}

#{column_create}
COMMANDS
      responses = []
      restore(commands) do |command, response|
        responses << [command.dup, response]
      end
      assert_equal([
                     [table_create, "true"],
                     ["", ""],
                     [column_create,"true"]
                   ],
                   responses)
    end

    private
    def restore(commands, &block)
      restore_context = Groonga::Context.new
      restore_context.create_database(@database_path.to_s) do
        restore_context.restore(commands, &block)
      end
    end

    def dump
      dump_context = Groonga::Context.new
      dump_context.open_database(@database_path.to_s) do
        Groonga::DatabaseDumper.dump(:context => dump_context)
      end
    end
  end
end
