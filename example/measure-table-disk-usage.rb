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
  puts "Usage: #{$0} DB_DIR TABLE_TYPE TABLE_KEY_TYPE"
  puts " e.g.: #{$0} db hash ShortText"
  exit(false)
end

db_dir = ARGV.shift
table_type = ARGV.shift
table_key_type = ARGV.shift

FileUtils.rm_rf(db_dir)
FileUtils.mkdir_p(db_dir)

Groonga::Database.create(:path => "#{db_dir}/db")

Groonga::Schema.define do |schema|
  schema.create_table("table",
                      :type => table_type.to_sym,
                      :key_type => table_key_type) do
  end
end

def measure_table_disk_usage(table)
  puts "\# of records,total disk usage,increment"
  previous_disk_usage = 0
  loop do
    if previous_disk_usage != table.disk_usage
      diff = table.disk_usage - previous_disk_usage
      puts "#{table.size},#{table.disk_usage},#{diff}"
      $stdout.flush
      previous_disk_usage = table.disk_usage
      return if table.size > 200_000_000
    end
    table.add(yield)
  end
end

def measure_short_text_table_disk_usage(table)
  key1 = 1
  key2 = 1
  measure_table_disk_usage(table) do
    key = "#{key1}-#{key2}"
    key2 += 1
    if key2 > 100
      key1 += 1
      key2 = 1
    end
    key
  end
end

def measure_int_table_disk_usage(table)
  value = 1
  measure_table_disk_usage(table) do
    value += 1
    value
  end
end

def measure_time_table_disk_usage(table)
  value = Time.at(0)
  measure_table_disk_usage(table) do
    value += 1
    value
  end
end

table = Groonga["table"]
table_key_type_name = table.domain.name
case table_key_type_name
when "ShortText"
  measure_short_text_table_disk_usage(table)
when /Int/
  measure_int_table_disk_usage(table)
when "Time"
  measure_time_table_disk_usage(table)
else
  puts "Unsupported table key type: #{table_key_type_name}"
  exit(false)
end
