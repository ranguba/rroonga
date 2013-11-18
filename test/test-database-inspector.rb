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
    "%.3fMiB" % (@database.disk_usage / (2 ** 20).to_f)
  end

  class TestDatabase < self
    def test_empty
      assert_equal(<<-INSPECTED, report)
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  0
  N tables:   0
  N columns:  0
      INSPECTED
    end

    class TestNRecords < self
      setup
      def setup_tables
        Groonga::Schema.define(:context => context) do |schema|
          schema.create_table("Users") do |table|
            table.short_text("name")
            table.int8("age")
          end

          schema.create_table("Bookmarks") do |table|
            table.text("description")
          end
        end

        @users = context["Users"]
        @bookmarks = context["Bookmarks"]
      end

      def test_no_records
        assert_equal(<<-INSPECTED, report)
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  0
  N tables:   2
  N columns:  3
        INSPECTED
      end

      def test_has_records
        @users.add
        @users.add
        @bookmarks.add

        assert_equal(<<-INSPECTED, report)
Database
  Path:       <#{@database_path}>
  Disk usage: #{inspect_disk_usage(@database.disk_usage)}
  N records:  3
  N tables:   2
  N columns:  3
        INSPECTED
      end
    end
  end
end
