#!/bin/sh

cat <<EOF > column-benchmark.rb
# -*- coding: utf-8 -*-
require 'benchmark'
require 'groonga'

def measure(try)
  n = 50000

  puts try
  table = Groonga[:Test]
  Benchmark.bm(12) do |x|
    x.report('empty loop') do
      n.times { }
    end
    x.report('column:') do
      n.times { table.column('col') }
    end
    x.report('columns:') do
      n.times { table.columns('col') }
    end
    x.report('have_column?') do
      n.times { table.have_column?('col') }
    end
  end
end

path = './test-table-column.db'
database = Groonga::Database.create(:path => path)
table = Groonga::Array.create(:name => 'Test')
table.define_column('col', 'ShortText')

puts 'immediate after column created'
measure '1st'
measure '2nd'

puts '----'

database.close
database = Groonga::Database.open(path)

puts 'after close/open'
measure '1st'
measure '2nd'

table.remove
database.remove
EOF

ruby -I lib -I ext/groogna column-benchmark.rb
