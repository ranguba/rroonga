# Copyright (C) 2016  Kouhei Sutou <kou@clear-code.com>
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

class RequestTimerTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    @request_id = "request-29"
    @timeout = 2.9
  end

  def teardown
  end

  def test_register
    timer_id = Groonga::RequestTimer.register(@request_id, @timeout)
    assert do
      timer_id.is_a?(Groonga::RequestTimerID)
    end
    Groonga::RequestTimer.unregister(timer_id)
  end

  def test_deafult_timeout
    assert_in_delta(0.0, Groonga::RequestTimer.default_timeout, 0.01)
    Groonga::RequestTimer.default_timeout = @timeout
    assert_in_delta(@timeout, Groonga::RequestTimer.default_timeout, 0.01)
  end
end
