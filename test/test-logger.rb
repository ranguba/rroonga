# Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>
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

class LoggerTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    @default_log_path = Groonga::Logger.path
    @default_query_log_path = Groonga::QueryLogger.path
  end

  def teardown
    Groonga::Logger.path = @default_log_path
    Groonga::QueryLogger.path = @default_query_log_path
  end

  def test_reopen
    Groonga::Logger.unregister
    p [@default_log_path, File.exist?(@default_log_path)]
    system("ls -lah #{@default_log_path}")
    if File.exist?(@default_log_path)
      FileUtils.mv(@default_log_path, "#{@default_log_path}.old")
    end
    assert_false(File.exist?(@default_log_path))
    Groonga::Logger.reopen
    p [@default_log_path, File.exist?(@default_log_path)]
    system("ls -lah #{@default_log_path}")
    assert_true(File.exist?(@default_log_path))
  end
end
