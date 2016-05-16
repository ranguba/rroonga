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

class RequestCancelerTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    @request_id = "request-29"
  end

  def teardown
    Groonga::RequestCanceler.unregister(@request_id)
  end

  def test_cancel
    assert do
      not Groonga::RequestCanceler.cancel(@request_id)
    end
    Groonga::RequestCanceler.register(@request_id)
    assert do
      Groonga::RequestCanceler.cancel(@request_id)
    end
  end

  def test_cancel_all
    assert do
      not Groonga::RequestCanceler.cancel_all
    end
    Groonga::RequestCanceler.register(@request_id)
    assert do
      Groonga::RequestCanceler.cancel_all
    end
  end
end
