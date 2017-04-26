# Copyright (C) 2017  Kouhei Sutou <kou@clear-code.com>
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

class DefaultCacheTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database
  def setup
    begin
      yield
    ensure
      Groonga::DefaultCache.base_path = nil
      Groonga::DefaultCache.reopen
    end
  end

  test "set base_path" do
    base_path = "#{@database_path}.cache"
    keys_path = "#{base_path}.keys"
    Groonga::DefaultCache.base_path = base_path
    assert do
      not File.exist?(keys_path)
    end

    Groonga::DefaultCache.reopen

    assert do
      File.exist?(keys_path)
    end
  end
end
