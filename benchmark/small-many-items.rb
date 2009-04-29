#!/usr/bin/ruby

# This benchmark is based on Tokyo Cabinet's benchmark at
# http://alpha.mixi.co.jp/blog/?p=791
#
# On my environment at 2009/04/29:
# % for x in {0..8}; do ruby benchmark/small-many-items.rb $x; done
#                           user     system      total        real        memory
# Hash                  1.070000   0.140000   1.210000 (  1.462675)     46.234MB
# groonga: Hash: memory 1.380000   0.140000   1.520000 (  1.804040)     23.531MB
# groonga: Trie: memory 1.780000   0.110000   1.890000 (  2.136395)     15.520MB
# groonga: Hash: file   1.380000   0.180000   1.560000 (  1.879252)     23.535MB
# groonga: Trie: file   1.650000   0.160000   1.810000 (  2.257756)     15.523MB
# TC: Hash: memory      0.680000   0.170000   0.850000 (  1.038155)     38.246MB
# TC: Tree: memory      0.640000   0.130000   0.770000 (  1.029011)     30.609MB
# TC: Hash: file        1.150000   2.900000   4.050000 (  4.908274)      0.164MB
# TC: Tree: file        0.970000   0.210000   1.180000 (  1.416418)      5.367MB
#
# Ruby: Debian GNU/Linux sid at 2009/04/29:
#       ruby 1.8.7 (2008-08-11 patchlevel 72) [x86_64-linux]
# groonga: HEAD at 2009/04/29: c97c3cf78b8f0761ca48ef211caa155135f89487
# Ruby/Groonga: trunk at 2009/04/29: r221
# Tokyo Cabinet: 1.4.17
# Tokyo Cabinet Ruby: 1.23

require 'benchmark'
require 'tempfile'

def memory_usage()
  status = `cat /proc/#{$$}/status`
  lines = status.split("\n")
  lines.each do |line|
    if line =~ /^VmRSS:/
      line.gsub!(/.*:\s*(\d+).*/, '\1')
      return line.to_i / 1024.0
    end
  end
  return -1;
end

n = 500000

@items = []

def item(label, &block)
  @items << [label, block]
end

def report(index=0)
  width = @items.collect do |label, _|
    label.length
  end.max

  label, block = @items[index]
  if label.nil?
    puts "unavailable report ID: #{index}"
    puts "available IDs:"
    @items.each_with_index do |(label, block), i|
      puts "#{i}: #{label}"
    end
    exit 1
  end

  if index.zero?
    puts(" " * (width - 1) + Benchmark::Tms::CAPTION.rstrip + "memory".rjust(14))
  end
  # GC.disable
  before = memory_usage
  result = Benchmark.measure(&block)
  # GC.enable
  GC.start
  size = memory_usage - before

  formatted_size = "%10.3f" % size
  puts "#{label.ljust(width)} #{result.to_s.strip} #{formatted_size}MB"
end

values = []
n.times do |i|
  values << "%08d" % i
end

item("Hash") do
  hash = {}
  values.each do |value|
    hash[value] = value
  end
end

begin
  base_dir = File.expand_path(File.join(File.dirname(__FILE__), ".."))
  $LOAD_PATH.unshift(File.join(base_dir, "src"))
  $LOAD_PATH.unshift(File.join(base_dir, "src", "lib"))

  require 'groonga'
  Groonga::Database.create

  item("groonga: Hash: memory") do
    hash = Groonga::Hash.create(:key_type => "<shorttext>",
                                :value_size => 8)
    values.each do |value|
      hash[value] = value
    end
  end

  item("groonga: Trie: memory") do
    hash = Groonga::PatriciaTrie.create(:key_type => "<shorttext>",
                                        :value_size => 8)
    values.each do |value|
      hash[value] = value
    end
  end

  hash_file = Tempfile.new("groonga-hash")
  item("groonga: Hash: file") do
    hash = Groonga::Hash.create(:key_type => "<shorttext>",
                                :value_size => 8,
                                :path => hash_file.path)
    values.each do |value|
      hash[value] = value
    end
  end

  trie_file = Tempfile.new("groonga-trie")
  item("groonga: Trie: file") do
    hash = Groonga::PatriciaTrie.create(:key_type => "<shorttext>",
                                        :value_size => 8,
                                        :path => trie_file.path)
    values.each do |value|
      hash[value] = value
    end
  end
rescue LoadError
end

begin
  require 'tokyocabinet'

  item("TC: Hash: memory") do
    db = TokyoCabinet::ADB::new
    db.open("*#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      db.put(value, value)
    end
  end

  item("TC: Tree: memory") do
    db = TokyoCabinet::ADB::new
    db.open("+#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      db.put(value, value)
    end
  end

  hash_file = Tempfile.new(["tc-hash", ".tch"])
  item("TC: Hash: file") do
    db = TokyoCabinet::ADB::new
    db.open("#{hash_file.path}#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      db.put(value, value)
    end
  end

  tree_file = Tempfile.new(["tc-tree", ".tcb"])
  item("TC: Tree: file") do
    db = TokyoCabinet::ADB::new
    db.open("#{tree_file.path}#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      db.put(value, value)
    end
  end
rescue LoadError
end

report(Integer(ARGV[0] || 0))
