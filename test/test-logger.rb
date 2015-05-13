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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

class LoggerTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    @default_log_path = Groonga::Logger.path
  end

  def teardown
    Groonga::Logger.path = @default_log_path
  end

  def test_reopen
    Groonga::Logger.unregister
    Groonga::Logger.path = @log_path.to_s
    if @log_path.exist?
      FileUtils.mv(@log_path, "#{@log_path}.old")
    end
    assert_false(@log_path.exist?)
    Groonga::Logger.reopen
    assert_true(@log_path.exist?)
  end
end
