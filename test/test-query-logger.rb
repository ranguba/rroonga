# Copyright (C) 2015  Kouhei Sutou <kou@clear-code.com>
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

class QueryLoggerTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    @default_log_path = Groonga::QueryLogger.path
  end

  def teardown
    Groonga::QueryLogger.path = @default_log_path
  end

  def test_reopen
    Groonga::QueryLogger.unregister
    Groonga::QueryLogger.path = @query_log_path.to_s
    if @query_log_path.exist?
      FileUtils.mv(@query_log_path, "#{@query_log_path}.old")
    end
    assert do
      not @query_log_path.exist?
    end
    Groonga::QueryLogger.reopen
    assert do
      @query_log_path.exist?
    end
  end
end
