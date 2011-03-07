# Copyright (C) 2011  Kouhei Sutou <kou@clear-code.com>
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

class TableDumperTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def setup
    @output = StringIO.new
    @dumper = Groonga::TableDumper.new(@output)
  end

  def test_dump_empty
    Groonga::Schema.define do |schema|
      schema.create_table("Posts") do |table|
      end
    end
    @dumper.dump(context["Posts"])
    assert_equal(<<-EOS, @output.string)
load --table Posts
[
]
EOS
  end
end
