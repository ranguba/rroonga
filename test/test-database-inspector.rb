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

  class TestDatabase < self
    def test_path
      assert_equal(<<-INSPECTED, report)
Database
  path: <#{@database_path}>
      INSPECTED
    end
  end
end
