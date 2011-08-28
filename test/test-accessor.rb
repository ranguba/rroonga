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

class AccessorTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database
    @posts = Groonga::Hash.create(:name => "Posts", :key_type => "ShortText")
    @id = @posts.column("_id")
  end

  def teardown
    @id = nil
  end

  def test_name
    assert_nil(@id.name)
  end

  def test_local_name
    assert_equal("_id", @id.local_name)
  end
end
