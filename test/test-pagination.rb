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

class PaginationTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  setup
  def setup_data
    Groonga::Schema.define do |schema|
      schema.create_table("Users",
                          :type => :hash,
                          :key_type => "ShortText") do |table|
        table.uint32(:number)
      end
    end
    @users = context["Users"]
    150.times do |i|
      @users.add("user#{i + 1}", :number => i + 1)
    end
  end

  def test_default
    assert_paginate({
                      :current_page => 1,
                      :page_size => 10,
                      :n_pages => 15,
                      :n_records => 150,
                      :record_range_in_page => 1..10
                    })
  end

  def test_page
    assert_paginate({
                      :current_page => 6,
                      :page_size => 10,
                      :n_pages => 15,
                      :n_records => 150,
                      :record_range_in_page => 51..60
                    },
                    :page => 6)
  end

  def test_max_page
    assert_paginate({
                      :current_page => 15,
                      :page_size => 10,
                      :n_pages => 15,
                      :n_records => 150,
                      :record_range_in_page => 141..150,
                    },
                    :page => 15)
  end

  def test_too_large_page
    assert_raise(Groonga::TooLargePage) do
      assert_paginate({},
                      :page => 16)
    end
  end

  def test_zero_page
    assert_raise(Groonga::TooSmallPage) do
      assert_paginate({},
                      :page => 0)
    end
  end

  def test_negative_page
    assert_raise(Groonga::TooSmallPage) do
      assert_paginate({},
                      :page => -1)
    end
  end

  private
  def assert_paginate(expected, options={})
    users = @users.paginate([["number"]], options)
    expected[:keys] ||= expected[:record_range_in_page].collect {|i| "user#{i}"}
    assert_equal(expected,
                 :current_page => users.current_page,
                 :page_size => users.page_size,
                 :n_pages => users.n_pages,
                 :n_records => users.n_records,
                 :record_range_in_page => users.record_range_in_page,
                 :keys => users.collect(&:key))
  end
end
