#!/usr/bin/env ruby
#
# Copyright (C) 2015  Kouhei Sutou <kou@clear-code.com>
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

require "groonga"
require "fileutils"

if ARGV.size != 3
  puts "Usage: #{$0} DB_DIR COLUMN_TYPE COLUMN_VALUE_TYPE"
  puts " e.g.: #{$0} db scalar ShortText"
  puts " e.g.: #{$0} db vector Time"
  puts " e.g.: #{$0} db vector reference"
  exit(false)
end

db_dir = ARGV.shift
column_type = ARGV.shift
column_value_type = ARGV.shift.to_sym

FileUtils.rm_rf(db_dir)
FileUtils.mkdir_p(db_dir)

Groonga::Database.create(:path => "#{db_dir}/db")

Groonga::Schema.define do |schema|
  schema.create_table("reference",
                      :type => :hash,
                      :key_type => "Int32") do |table|
  end

  schema.create_table("table", :type => :array) do |table|
    table.column("column", column_value_type, :type => column_type)
  end
end

def measure_column_disk_usage(column)
  puts "\# of records,total disk usage,increment"
  table = column.domain
  vector_p = column.vector?
  previous_disk_usage = 0
  loop do
    if previous_disk_usage != column.disk_usage
      diff = column.disk_usage - previous_disk_usage
      puts "#{table.size},#{column.disk_usage},#{diff}"
      $stdout.flush
      previous_disk_usage = column.disk_usage
      return if table.size > 200_000_000
    end
    if vector_p
      value = [yield, yield, yield]
    else
      value = yield
    end
    table.add(:column => value)
  end
end

def measure_reference_column_disk_usage(column)
  reference_key = 0
  reference = Groonga["reference"]
  measure_column_disk_usage(column) do
    reference_key += 1
    reference.add(reference_key)
  end
end

def measure_short_text_column_disk_usage(column)
  key1 = 1
  key2 = 1
  measure_column_disk_usage(column) do
    key = "#{key1}-#{key2}"
    key2 += 1
    if key2 > 100
      key1 += 1
      key2 = 1
    end
    key
  end
end

def measure_int_column_disk_usage(column)
  value = 1
  measure_column_disk_usage(column) do
    value += 1
    value
  end
end

def measure_time_column_disk_usage(column)
  value = Time.at(0)
  measure_column_disk_usage(column) do
    value += 1
    value
  end
end

column = Groonga["table.column"]
column_value_type_name = column.range.name
case column_value_type_name
when "reference"
  measure_reference_column_disk_usage(column)
when "ShortText"
  measure_short_text_column_disk_usage(column)
when /Int/
  measure_int_column_disk_usage(column)
when "Time"
  measure_time_column_disk_usage(column)
else
  puts "Unsupported column value type: #{column_value_type_name}"
  exit(false)
end
