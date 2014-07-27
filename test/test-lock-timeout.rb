# Copyright (C) 2014  Kouhei Sutou <kou@clear-code.com>
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

class LockTimeoutTest < Test::Unit::TestCase
  def setup
    @original_lock_timeout = Groonga.lock_timeout
  end

  def teardown
    Groonga.lock_timeout = @original_lock_timeout
  end

  def test_accessor
    Groonga.lock_timeout = 10
    assert_equal(10, Groonga.lock_timeout)
  end
end
