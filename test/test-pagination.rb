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
                      :start_offset => 1,
                      :end_offset => 10,
                      :have_previous_page? => false,
                      :previous_page => nil,
                      :have_next_page? => true,
                      :next_page => 2,
                      :first_page? => true,
                      :last_page? => false,
                      :have_pages? => true,
                    })
  end

  def test_no_entries
    @users.each do |user|
      user.delete
    end
    assert_paginate({
                      :current_page => 1,
                      :page_size => 10,
                      :n_pages => 1,
                      :n_records => 0,
                      :start_offset => nil,
                      :end_offset => nil,
                      :have_previous_page? => false,
                      :previous_page => nil,
                      :have_next_page? => false,
                      :next_page => nil,
                      :first_page? => true,
                      :last_page? => true,
                      :have_pages? => false,
                      :keys => [],
                    })
  end

  def test_page
    assert_paginate({
                      :current_page => 6,
                      :page_size => 10,
                      :n_pages => 15,
                      :n_records => 150,
                      :start_offset => 51,
                      :end_offset => 60,
                      :have_previous_page? => true,
                      :previous_page => 5,
                      :have_next_page? => true,
                      :next_page => 7,
                      :first_page? => false,
                      :last_page? => false,
                      :have_pages? => true,
                    },
                    :page => 6)
  end

  def test_max_page
    assert_paginate({
                      :current_page => 15,
                      :page_size => 10,
                      :n_pages => 15,
                      :n_records => 150,
                      :start_offset => 141,
                      :end_offset => 150,
                      :have_previous_page? => true,
                      :previous_page => 14,
                      :have_next_page? => false,
                      :next_page => nil,
                      :first_page? => false,
                      :last_page? => true,
                      :have_pages? => true,
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

  def test_size
    assert_paginate({
                      :current_page => 1,
                      :page_size => 7,
                      :n_pages => 22,
                      :n_records => 150,
                      :start_offset => 1,
                      :end_offset => 7,
                      :have_previous_page? => false,
                      :previous_page => nil,
                      :have_next_page? => true,
                      :next_page => 2,
                      :first_page? => true,
                      :last_page? => false,
                      :have_pages? => true,
                    },
                    :size => 7)
  end

  def test_max_size
    assert_paginate({
                      :current_page => 1,
                      :page_size => 150,
                      :n_pages => 1,
                      :n_records => 150,
                      :start_offset => 1,
                      :end_offset => 150,
                      :have_previous_page? => false,
                      :previous_page => nil,
                      :have_next_page? => false,
                      :next_page => nil,
                      :first_page? => true,
                      :last_page? => true,
                      :have_pages? => false,
                    },
                    :size => 150)
  end

  def test_too_large_size
    assert_paginate({
                      :current_page => 1,
                      :page_size => 151,
                      :n_pages => 1,
                      :n_records => 150,
                      :start_offset => 1,
                      :end_offset => 150,
                      :have_previous_page? => false,
                      :previous_page => nil,
                      :have_next_page? => false,
                      :next_page => nil,
                      :first_page? => true,
                      :last_page? => true,
                      :have_pages? => false,
                    },
                    :size => 151)
  end

  def test_zero_size
    assert_raise(Groonga::TooSmallPageSize) do
      assert_paginate({},
                      :size => 0)
    end
  end

  def test_negative_size
    assert_raise(Groonga::TooSmallPageSize) do
      assert_paginate({},
                      :size => -1)
    end
  end

  def test_full
    assert_paginate({
                      :current_page => 2,
                      :page_size => 50,
                      :n_pages => 3,
                      :n_records => 150,
                      :start_offset => 51,
                      :end_offset => 100,
                      :have_previous_page? => true,
                      :previous_page => 1,
                      :have_next_page? => true,
                      :next_page => 3,
                      :first_page? => false,
                      :last_page? => false,
                      :have_pages? => true,
                    },
                    :page => 2,
                    :size => 50)
  end

  private
  def assert_paginate(expected, options={})
    users = @users.paginate([["number"]], options)
    if expected[:keys].nil?
      range = (expected[:start_offset]..expected[:end_offset])
      expected[:keys] = range.collect {|i| "user#{i}"}
    end
    assert_equal(expected,
                 :current_page => users.current_page,
                 :page_size => users.page_size,
                 :n_pages => users.n_pages,
                 :n_records => users.n_records,
                 :start_offset => users.start_offset,
                 :end_offset => users.end_offset,
                 :previous_page => users.previous_page,
                 :have_previous_page? => users.have_previous_page?,
                 :next_page => users.next_page,
                 :have_next_page? => users.have_next_page?,
                 :first_page? => users.first_page?,
                 :last_page? => users.last_page?,
                 :have_pages? => users.have_pages?,
                 :keys => users.collect {|record| record.value.key})
  end
end
