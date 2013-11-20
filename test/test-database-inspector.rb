# Copyright (C) 2013  Kouhei Sutou <kou@clear-code.com>
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

class DatabaseInspectorTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database, :before => :append

  private
  def report
    output = StringIO.new
    inspector = Groonga::DatabaseInspector.new(@database)
    inspector.report(output)
    output.string
  end

  def inspect_disk_usage(disk_usage)
    if disk_usage < (2 ** 20)
      "%.3fKiB" % (disk_usage / (2 ** 10).to_f)
    else
      "%.3fMiB" % (disk_usage / (2 ** 20).to_f)
    end
  end

  def inspect_table(table)
    <<-INSPECTED
    #{table.name}:
      ID:         #{table.id}
      Type:       #{inspect_table_type(table)}
      Key type:   #{inspect_key_type(table)}
      Tokenizer:  #{inspect_tokenizer(table)}
      Normalizer: #{inspect_normalizer(table)}
      Path:       <#{table.path}>
      Disk usage: #{inspect_disk_usage(table.disk_usage)}
      N records:  #{table.size}
      N columns:  #{table.columns.size}
#{inspect_columns(table.columns).chomp}
    INSPECTED
  end

  def inspect_table_type(table)
    case table
    when Groonga::Array
      "array"
    when Groonga::Hash
      "hash"
    when Groonga::PatriciaTrie
      "patricia trie"
    when Groonga::DoubleArrayTrie
      "double array trie"
    end
  end

  def inspect_key_type(table)
    if table.support_key?
      table.domain.name
    else
      "(no key)"
    end
  end

  def inspect_tokenizer(table)
    if table.support_key?
      tokenizer = table.default_tokenizer
      if tokenizer
        tokenizer.name
      else
        "(no tokenizer)"
      end
    else
      "(no key)"
    end
  end

  def inspect_normalizer(table)
    if table.support_key?
      normalizer = table.normalizer
      if normalizer
        normalizer.name
      else
        "(no normalizer)"
      end
    else
      "(no key)"
    end
  end

  def inspect_columns(columns)
    if columns.empty?
      <<-INSPECTED
      Columns:
        None
      INSPECTED
    else
      inspected = "      Columns:\n"
      columns.each do |column|
        inspected << inspect_column(column)
      end
      inspected
    end
  end

  def inspect_column(column)
    <<-INSPECTED
        #{column.local_name}:
          ID:         #{column.id}
          Type:       #{inspect_column_type(column)}
          Value type: #{column.domain.name}
          Path:       <#{column.path}>
          Disk usage: #{inspect_disk_usage(column.disk_usage)}
    INSPECTED
  end

  def inspect_column_type(column)
    case column
    when Groonga::FixSizeColumn
      "scalar"
    when Groonga::VariableSizeColumn
      if column.vector?
        "vector"
      else
        "scalar"
      end
    else
      "index"
    end
  end

  class DatabaseTest < self
    def test_empty
      assert_equal(<<-INSPECTED, report)
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  0
  N tables:   0
  N columns:  0
  Plugins:
    None
  Tables:
    None
      INSPECTED
    end

    class NRecordsTest < self
      setup
      def setup_tables
        Groonga::Schema.define do |schema|
          schema.create_table("Users") do |table|
          end

          schema.create_table("Bookmarks") do |table|
          end
        end

        @users = context["Users"]
        @bookmarks = context["Bookmarks"]
      end

      def test_no_records
        assert_equal(inspected(0), report)
      end

      def test_has_records
        @users.add
        @users.add
        @bookmarks.add

        assert_equal(inspected(3), report)
      end

      private
      def inspected(n_records)
        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  #{n_records}
  N tables:   2
  N columns:  0
  Plugins:
    None
  Tables:
#{inspect_table(@bookmarks).chomp}
#{inspect_table(@users).chomp}
        INSPECTED
      end
    end

    class NTablesTest < self
      def test_no_tables
        assert_equal(inspected(0), report)
      end

      def test_has_tables
        Groonga::Schema.define do |schema|
          schema.create_table("Users") do |table|
          end

          schema.create_table("Bookmarks") do |table|
          end
        end

        assert_equal(inspected(2), report)
      end

      private
      def inspected(n_tables)
        inspected_tables = "  Tables:\n"
        if @database.tables.empty?
          inspected_tables << "    None\n"
        else
          @database.tables.each do |table|
            inspected_tables << inspect_table(table)
          end
        end

        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  0
  N tables:   #{n_tables}
  N columns:  0
  Plugins:
    None
#{inspected_tables.chomp}
        INSPECTED
      end
    end

    class NColumnsTest < self
      setup
      def setup_tables
        Groonga::Schema.define do |schema|
          schema.create_table("Users") do |table|
          end

          schema.create_table("Bookmarks") do |table|
          end
        end

        @users = context["Users"]
        @bookmarks = context["Bookmarks"]
      end

      def test_no_columns
        assert_equal(inspected(0), report)
      end

      def test_has_columns
        Groonga::Schema.define do |schema|
          schema.create_table("Users") do |table|
            table.short_text("name")
            table.int8("age")
          end

          schema.create_table("Bookmarks") do |table|
            table.text("description")
          end
        end

        assert_equal(inspected(3), report)
      end

      private
      def inspected(n_columns)
        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  0
  N tables:   2
  N columns:  #{n_columns}
  Plugins:
    None
  Tables:
#{inspect_table(@bookmarks).chomp}
#{inspect_table(@users).chomp}
        INSPECTED
      end
    end

    class PluginsTest < self
      def test_no_plugins
        assert_equal(inspected(<<-INSPECTED), report)
  Plugins:
    None
        INSPECTED
      end

      def test_has_plugin
        context.register_plugin("query_expanders/tsv")
        assert_equal(inspected(<<-INSPECTED), report)
  Plugins:
    * query_expanders/tsv#{Groonga::Plugin.suffix}
        INSPECTED
      end

      private
      def inspected(inspected_plugins)
        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  0
  N tables:   0
  N columns:  0
#{inspected_plugins.chomp}
  Tables:
    None
        INSPECTED
      end
    end
  end

  class TableTest < self
    class NoColumnTest < self
      def test_nothing
        assert_equal(inspected(<<-INSPECTED), report)
  Tables:
    None
        INSPECTED
      end

      def test_empty
        Groonga::Schema.define do |schema|
          schema.create_table("Users") do |table|
          end
        end
        users = context["Users"]

        assert_equal(inspected(<<-INSPECTED), report)
  Tables:
    Users:
      ID:         #{users.id}
      Type:       #{inspect_table_type(users)}
      Key type:   #{inspect_key_type(users)}
      Tokenizer:  #{inspect_tokenizer(users)}
      Normalizer: #{inspect_normalizer(users)}
      Path:       <#{users.path}>
      Disk usage: #{inspect_disk_usage(users.disk_usage)}
      N records:  #{users.size}
      N columns:  #{users.columns.size}
      Columns:
        None
        INSPECTED
      end

      private
      def inspected(inspected_tables)
        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  0
  N tables:   #{@database.tables.size}
  N columns:  0
  Plugins:
    None
#{inspected_tables.chomp}
        INSPECTED
      end
    end

    class NRecordsTest < self
      setup
      def setup_tables
        Groonga::Schema.define do |schema|
          schema.create_table("Users") do |table|
          end
        end
        @users = context["Users"]
      end

      def test_no_record
        assert_equal(inspected(0), report)
      end

      def test_empty
        @users.add
        @users.add
        @users.add

        assert_equal(inspected(3), report)
      end

      private
      def inspected(n_records)
        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  #{@users.size}
  N tables:   #{@database.tables.size}
  N columns:  0
  Plugins:
    None
  Tables:
    #{@users.name}:
      ID:         #{@users.id}
      Type:       #{inspect_table_type(@users)}
      Key type:   #{inspect_key_type(@users)}
      Tokenizer:  #{inspect_tokenizer(@users)}
      Normalizer: #{inspect_normalizer(@users)}
      Path:       <#{@users.path}>
      Disk usage: #{inspect_disk_usage(@users.disk_usage)}
      N records:  #{n_records}
      N columns:  #{@users.columns.size}
#{inspect_columns(@users.columns).chomp}
        INSPECTED
      end
    end

    class TypeTest < self
      def test_array
        Groonga::Schema.create_table("Users")
        @table = Groonga["Users"]
        assert_equal(inspected("array"), report)
      end

      def test_hash
        Groonga::Schema.create_table("Users",
                                     :type => :hash,
                                     :key_type => :short_text)
        @table = Groonga["Users"]
        assert_equal(inspected("hash"), report)
      end

      def test_patricia_trie
        Groonga::Schema.create_table("Users",
                                     :type => :patricia_trie,
                                     :key_type => :short_text)
        @table = Groonga["Users"]
        assert_equal(inspected("patricia trie"), report)
      end

      def test_double_array_trie
        Groonga::Schema.create_table("Users",
                                     :type => :double_array_trie,
                                     :key_type => :short_text)
        @table = Groonga["Users"]
        assert_equal(inspected("double array trie"), report)
      end

      private
      def inspected(type)
        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  #{@table.size}
  N tables:   #{@database.tables.size}
  N columns:  0
  Plugins:
    None
  Tables:
    #{@table.name}:
      ID:         #{@table.id}
      Type:       #{type}
      Key type:   #{inspect_key_type(@table)}
      Tokenizer:  #{inspect_tokenizer(@table)}
      Normalizer: #{inspect_normalizer(@table)}
      Path:       <#{@table.path}>
      Disk usage: #{inspect_disk_usage(@table.disk_usage)}
      N records:  #{@table.size}
      N columns:  #{@table.columns.size}
#{inspect_columns(@table.columns).chomp}
        INSPECTED
      end
    end

    class KeyTypeTest < self
      def test_array
        Groonga::Schema.create_table("Users")
        @table = Groonga["Users"]
        assert_equal(inspected("(no key)"), report)
      end

      def test_key_support_table
        Groonga::Schema.create_table("Users",
                                     :type => :hash,
                                     :key_type => "Int32")
        @table = Groonga["Users"]
        assert_equal(inspected("Int32"), report)
      end

      private
      def inspected(key_type)
        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  #{@table.size}
  N tables:   #{@database.tables.size}
  N columns:  0
  Plugins:
    None
  Tables:
    #{@table.name}:
      ID:         #{@table.id}
      Type:       #{inspect_table_type(@table)}
      Key type:   #{key_type}
      Tokenizer:  #{inspect_tokenizer(@table)}
      Normalizer: #{inspect_normalizer(@table)}
      Path:       <#{@table.path}>
      Disk usage: #{inspect_disk_usage(@table.disk_usage)}
      N records:  #{@table.size}
      N columns:  #{@table.columns.size}
#{inspect_columns(@table.columns).chomp}
        INSPECTED
      end
    end

    class TokenizerTest < self
      def test_array
        Groonga::Schema.create_table("Users")
        @table = Groonga["Users"]
        assert_equal(inspected("(no key)"), report)
      end

      def test_no_tokenizer
        Groonga::Schema.create_table("Users",
                                     :type => :hash,
                                     :key_type => :short_text)
        @table = Groonga["Users"]
        assert_equal(inspected("(no tokenizer)"), report)
      end

      def test_have_tokenizer
        Groonga::Schema.create_table("Users",
                                     :type => :patricia_trie,
                                     :key_type => :short_text,
                                     :default_tokenizer => "TokenBigram")
        @table = Groonga["Users"]
        assert_equal(inspected("TokenBigram"), report)
      end

      private
      def inspected(inspected_tokenizer)
        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  #{@table.size}
  N tables:   #{@database.tables.size}
  N columns:  0
  Plugins:
    None
  Tables:
    #{@table.name}:
      ID:         #{@table.id}
      Type:       #{inspect_table_type(@table)}
      Key type:   #{inspect_key_type(@table)}
      Tokenizer:  #{inspected_tokenizer}
      Normalizer: #{inspect_normalizer(@table)}
      Path:       <#{@table.path}>
      Disk usage: #{inspect_disk_usage(@table.disk_usage)}
      N records:  #{@table.size}
      N columns:  #{@table.columns.size}
#{inspect_columns(@table.columns).chomp}
        INSPECTED
      end
    end

    class NormalizerTest < self
      def test_array
        Groonga::Schema.create_table("Users")
        @table = Groonga["Users"]
        assert_equal(inspected("(no key)"), report)
      end

      def test_no_normalizer
        Groonga::Schema.create_table("Users",
                                     :type => :hash,
                                     :key_type => :short_text)
        @table = Groonga["Users"]
        assert_equal(inspected("(no normalizer)"), report)
      end

      def test_have_normalizer
        Groonga::Schema.create_table("Users",
                                     :type => :patricia_trie,
                                     :key_type => :short_text,
                                     :normalizer => "NormalizerAuto")
        @table = Groonga["Users"]
        assert_equal(inspected("NormalizerAuto"), report)
      end

      private
      def inspected(inspected_normalizer)
        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  #{@table.size}
  N tables:   #{@database.tables.size}
  N columns:  0
  Plugins:
    None
  Tables:
    #{@table.name}:
      ID:         #{@table.id}
      Type:       #{inspect_table_type(@table)}
      Key type:   #{inspect_key_type(@table)}
      Tokenizer:  #{inspect_tokenizer(@table)}
      Normalizer: #{inspected_normalizer}
      Path:       <#{@table.path}>
      Disk usage: #{inspect_disk_usage(@table.disk_usage)}
      N records:  #{@table.size}
      N columns:  #{@table.columns.size}
#{inspect_columns(@table.columns).chomp}
        INSPECTED
      end
    end

    class NColumnsTest < self
      def test_no_columns
        Groonga::Schema.create_table("Users")
        @table = Groonga["Users"]
        assert_equal(inspected(0), report)
      end

      def test_have_columns
        Groonga::Schema.create_table("Users") do |table|
          table.short_text("name")
          table.int8("age")
        end
        @table = Groonga["Users"]
        assert_equal(inspected(2), report)
      end

      private
      def inspected(n_columns)
        <<-INSPECTED
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  #{@table.size}
  N tables:   #{@database.tables.size}
  N columns:  #{@table.columns.size}
  Plugins:
    None
  Tables:
    #{@table.name}:
      ID:         #{@table.id}
      Type:       #{inspect_table_type(@table)}
      Key type:   #{inspect_key_type(@table)}
      Tokenizer:  #{inspect_tokenizer(@table)}
      Normalizer: #{inspect_normalizer(@table)}
      Path:       <#{@table.path}>
      Disk usage: #{inspect_disk_usage(@table.disk_usage)}
      N records:  #{@table.size}
      N columns:  #{n_columns}
#{inspect_columns(@table.columns).chomp}
        INSPECTED
      end
    end
  end

  class ColumnTest < self
    private
    def report
      output = StringIO.new
      reporter = Groonga::DatabaseInspector::Reporter.new(@database, output)
      reporter.send(:report_column, @column)
      output.string
    end

    class TypeTest < self
      def test_scalar_fix_size
        create_table do |table|
          table.int32("age")
        end
        assert_equal(inspected("scalar"), report)
      end

      def test_scalar_variable_size
        create_table do |table|
          table.short_text("name")
        end
        assert_equal(inspected("scalar"), report)
      end

      def test_vector
        create_table do |table|
          table.short_text("tags", :type => :vector)
        end
        assert_equal(inspected("vector"), report)
      end

      def test_index
        create_table do |table|
        end
        Groonga::Schema.create_table("Bookmarks") do |table|
          table.reference("user", "Users")
        end
        create_table do |table|
          table.index("Bookmarks.user")
        end
        assert_equal(inspected("index"), report)
      end

      private
      def create_table
        Groonga::Schema.create_table("Users") do |table|
          yield(table)
        end
        @table = Groonga["Users"]
        @column = @table.columns.first
      end

      def inspected(type)
        <<-INSPECTED
#{@column.local_name}:
  ID:         #{@column.id}
  Type:       #{type}
  Value type: #{@column.domain.name}
  Path:       <#{@column.path}>
  Disk usage: #{inspect_disk_usage(@column.disk_usage)}
        INSPECTED
      end
    end
  end
end
