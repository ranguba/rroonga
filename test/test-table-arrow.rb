# Copyright (C) 2017-2020  Sutou Kouhei <kou@clear-code.com>
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

class TableArrowTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database

    omit("Apache Arrow support is required") unless context.support_arrow?
  end

  def open_temporary_file(extension)
    tempfile = Tempfile.new(["table-arrow", extension])
    begin
      yield(tempfile)
    ensure
      tempfile.close!
    end
  end

  def assert_dump_load(type, n_records)
    Groonga::Schema.define do |schema|
      schema.create_table("Source") do |table|
        table.__send__(type, "data")
      end

      schema.create_table("Destination") do |table|
      end
    end

    source = Groonga["Source"]
    destination = Groonga["Destination"]

    expected = []
    n_records.times do |i|
      data = yield(i)
      expected << {"_id" => i + 1, "data" => data}
      source.add(:data => data)
    end

    open_temporary_file(".arrow") do |tempfile|
      source.dump_arrow(tempfile.path)
      destination.load_arrow(tempfile.path)
    end

    assert_equal(expected,
                 destination.collect(&:attributes))
  end

  def test_uint8
    n_records = 128
    assert_dump_load(:uint8, n_records) do |i|
      i
    end
  end

  def test_int8
    n_records = 128
    assert_dump_load(:int8, n_records) do |i|
      data = i
      data = -data if (i % 2).zero?
      data
    end
  end

  def test_uint16
    n_records = 128
    assert_dump_load(:uint16, n_records) do |i|
      i * 10
    end
  end

  def test_int16
    n_records = 128
    assert_dump_load(:int16, n_records) do |i|
      data = i * 10
      data = -data if (i % 2).zero?
      data
    end
  end

  def test_uint32
    n_records = 128
    assert_dump_load(:uint32, n_records) do |i|
      i * 100
    end
  end

  def test_int32
    n_records = 128
    assert_dump_load(:int32, n_records) do |i|
      data = i * 100
      data = -data if (i % 2).zero?
      data
    end
  end

  def test_uint64
    n_records = 128
    assert_dump_load(:uint64, n_records) do |i|
      i * 100
    end
  end

  def test_int64
    n_records = 128
    assert_dump_load(:int64, n_records) do |i|
      data = i * 100
      data = -data if (i % 2).zero?
      data
    end
  end

  def test_float
    n_records = 128
    assert_dump_load(:float, n_records) do |i|
      data = i * 1.1
      data = -data if (i % 2).zero?
      data
    end
  end

  def test_time
    n_records = 128
    now = Time.at(Time.now.to_i)
    assert_dump_load(:time, n_records) do |i|
      now + i
    end
  end

  def test_dump_columns
    Groonga::Schema.define do |schema|
      schema.create_table("Source") do |table|
        table.int32("data1")
        table.short_text("data2")
      end

      schema.create_table("Destination") do |table|
      end
    end

    source = Groonga["Source"]
    destination = Groonga["Destination"]

    expected = []
    10.times do |i|
      data1 = i * 10
      data2 = i.to_s
      expected << {"_id" => i + 1, "data1" => data1}
      source.add(:data1 => data1, :data2 => data2)
    end

    open_temporary_file(".arrow") do |tempfile|
      source.dump_arrow(tempfile.path, columns: source.columns[0, 1])
      destination.load_arrow(tempfile.path)
    end

    assert_equal(expected,
                 destination.collect(&:attributes))
  end

  def test_dump_column_names
    Groonga::Schema.define do |schema|
      schema.create_table("Source") do |table|
        table.int32("data1")
        table.short_text("data2")
      end

      schema.create_table("Destination") do |table|
      end
    end

    source = Groonga["Source"]
    destination = Groonga["Destination"]

    expected = []
    10.times do |i|
      data1 = i * 10
      data2 = i.to_s
      expected << {"_id" => i + 1, "data1" => data1}
      source.add(:data1 => data1, :data2 => data2)
    end

    open_temporary_file(".arrow") do |tempfile|
      source.dump_arrow(tempfile.path, column_names: ["data1"])
      destination.load_arrow(tempfile.path)
    end

    assert_equal(expected,
                 destination.collect(&:attributes))
  end
end
