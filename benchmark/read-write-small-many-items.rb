#!/usr/bin/ruby

# This benchmark is based on Tokyo Cabinet's benchmark at
# http://alpha.mixi.co.jp/blog/?p=791
#
# On my environment at 2009/05/16:
# % for x in {0..9}; do ruby benchmark/read-write-small-many-items.rb $x; done
#                           user     system      total        real        memory
# Hash                  1.440000   0.120000   1.560000 (  2.018838)     47.230MB
# groonga: Hash: memory 1.780000   0.170000   1.950000 (  2.331934)     30.582MB
# groonga: Trie: memory 2.220000   0.150000   2.370000 (  2.788049)     22.594MB
# groonga: Hash: file   1.810000   0.170000   1.980000 (  2.389940)     30.590MB
# groonga: Trie: file   2.170000   0.150000   2.320000 (  2.862775)     22.594MB
# Localmemcache: file   1.710000   0.240000   1.950000 (  2.311414)     46.410MB
# TC: Hash: memory      1.540000   0.180000   1.720000 (  2.048307)     49.180MB
# TC: Tree: memory      1.410000   0.160000   1.570000 (  1.891273)     37.691MB
# TC: Hash: file        2.420000   2.950000   5.370000 (  7.122531)      9.258MB
# TC: Tree: file        2.080000   0.220000   2.300000 (  2.876335)     13.234MB
#
# Ruby: Debian GNU/Linux sid at 2009/05/16:
#       ruby 1.8.7 (2008-08-11 patchlevel 72) [x86_64-linux]
# groonga: HEAD at 2009/05/16: ed49074fbc217b97c0b0b8d675f2417f9490c87e
# Ruby/Groonga: trunk at 2009/05/16: r321
# Localmemcache: HEAD at 2009/05/16: 6f8cfbd3a103c2be8d54834a65deb096d7cda34f
# Tokyo Cabinet: 1.4.20
# Tokyo Cabinet Ruby: 1.23
#
# NOTE: All C source codes are built with "-g -O0"
# option. They will be more faster with "-O2" option.

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
  @hash = {}
  values.each do |value|
    @hash[value] = value
    @hash[value]
  end
end

begin
  base_dir = File.expand_path(File.join(File.dirname(__FILE__), ".."))
  $LOAD_PATH.unshift(File.join(base_dir, "ext"))
  $LOAD_PATH.unshift(File.join(base_dir, "lib"))

  require 'groonga'
  @database = Groonga::Database.create

  item("groonga: Hash: memory") do
    @hash = Groonga::Hash.create(:key_type => "<shorttext>",
                                 :value_size => 8)
    values.each do |value|
      @hash[value] = value
      @hash[value]
    end
  end

  item("groonga: Trie: memory") do
    @hash = Groonga::PatriciaTrie.create(:key_type => "<shorttext>",
                                         :value_size => 8)
    values.each do |value|
      @hash[value] = value
      @hash[value]
    end
  end

  @hash_file = Tempfile.new("groonga-hash")
  item("groonga: Hash: file") do
    @hash = Groonga::Hash.create(:key_type => "<shorttext>",
                                 :value_size => 8,
                                 :path => @hash_file.path)
    values.each do |value|
      @hash[value] = value
      @hash[value]
    end
  end

  trie_file = Tempfile.new("groonga-trie")
  item("groonga: Trie: file") do
    @hash = Groonga::PatriciaTrie.create(:key_type => "<shorttext>",
                                         :value_size => 8,
                                         :path => trie_file.path)
    values.each do |value|
      @hash[value] = value
      @hash[value]
    end
  end
rescue LoadError
end

begin
  require 'localmemcache'

  item("Localmemcache: file") do
    LocalMemCache.drop(:namespace => "write-small-many-items", :force => true)
    @db = LocalMemCache.new(:namespace => "write-small-many-items")
    values.each do |value|
      @db[value] = value
      @db[value]
    end
  end
rescue LoadError
end

begin
  require 'tokyocabinet'

  item("TC: Hash: memory") do
    @db = TokyoCabinet::ADB::new
    @db.open("*#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      @db.put(value, value)
      @db.get(value)
    end
  end

  item("TC: Tree: memory") do
    @db = TokyoCabinet::ADB::new
    @db.open("+#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      @db.put(value, value)
      @db.get(value)
    end
  end

  hash_file = Tempfile.new(["tc-hash", ".tch"])
  item("TC: Hash: file") do
    @db = TokyoCabinet::ADB::new
    @db.open("#{hash_file.path}#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      @db.put(value, value)
      @db.get(value)
    end
  end

  tree_file = Tempfile.new(["tc-tree", ".tcb"])
  item("TC: Tree: file") do
    @db = TokyoCabinet::ADB::new
    @db.open("#{tree_file.path}#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      @db.put(value, value)
      @db.get(value)
    end
  end
rescue LoadError
end

report(Integer(ARGV[0] || 0))
