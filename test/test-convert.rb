# -*- coding: utf-8 -*-
#
# Copyright (C) 2013  Kouhei Sutou <kou@clear-code.com>
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

class ConvertTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  setup
  def setup_schema
  end

  setup
  def setup_data
  end

  class Int64Test < self
    def setup_schema
      Groonga::Schema.define do |schema|
        schema.create_table("Values") do |table|
          table.int64("content")
        end
      end
      @values = Groonga["Values"]
    end

    data("Fixnum"     => -1,
         "Max Fixnum" => 2 ** 62 - 1,
         "Bignum"     => 2 ** 62)
    def test_select(value)
      @values.add(:content => value)
      result = @values.select do |record|
        record.content == value
      end
      assert_equal([value], result.collect(&:content))
    end
  end
end
