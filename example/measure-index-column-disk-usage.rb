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

if ARGV.size != 4
  puts "Usage: #{$0} DB_DIR WITH_POSITION WITH_SECTION N_POSTINGS_PER_TERM"
  puts " e.g.: #{$0} db true false 100"
  exit(false)
end

db_dir = ARGV.shift
with_position_p = (ARGV.shift == "true")
with_section_p = (ARGV.shift == "true")
n_postings_per_term = Integer(ARGV.shift)

FileUtils.rm_rf(db_dir)
FileUtils.mkdir_p(db_dir)

Groonga::Database.create(:path => "#{db_dir}/db")

Groonga::Schema.define do |schema|
  schema.create_table("table", :type => :array) do |table|
    table.column("column", "Int32")
  end

  schema.create_table("lexicon",
                      :type => :patricia_trie,
                      :key_type => "Int32") do |table|
    table.index("table.column",
                :name => "index",
                :with_position => with_position_p,
                :with_section => with_section_p)
  end
end

def measure_disk_usage(data_table, index_column, n_postings_per_term)
  puts "\# of data records,\# of terms,total disk usage,increment"

  data_table = Groonga["table"]
  lexicon = index_column.domain
  n_postings = 0
  value = 0

  previous_disk_usage = 0
  loop do
    if previous_disk_usage != index_column.disk_usage
      diff = index_column.disk_usage - previous_disk_usage
      puts "#{data_table.size},#{lexicon.size},#{index_column.disk_usage},#{diff}"
      $stdout.flush
      previous_disk_usage = index_column.disk_usage
      return if data_table.size > 200_000_000
    end
    if n_postings < n_postings_per_term
      n_postings += 1
    else
      n_postings = 0
      value += 1
    end
    data_table.add(:column => value)
  end
end

data_table = Groonga["table"]
index_column = Groonga["lexicon.index"]
measure_disk_usage(data_table, index_column, n_postings_per_term)
