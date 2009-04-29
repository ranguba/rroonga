#!/usr/bin/ruby

# This benchmark is based on Tokyo Cabinet's benchmark at
# http://alpha.mixi.co.jp/blog/?p=791
#
# On my environment at 2009/04/27:
# % for x in {0..8}; do ruby benchmark/small-many-items.rb $x; done
#                           user     system      total        real        memory
# Hash                  4.750000   0.300000   5.050000 (  6.950539)    129.273MB
# groonga: Hash: memory 4.980000   0.200000   5.180000 (  6.877130)     23.613MB
# groonga: Trie: memory 5.660000   0.180000   5.840000 (  7.843830)     15.684MB
# groonga: Hash: file   5.010000   0.300000   5.310000 (  7.292904)     23.617MB
# groonga: Trie: file   5.570000   0.220000   5.790000 (  8.004818)     15.688MB
# TC: Hash: memory      3.020000   0.210000   3.230000 (  4.087278)     53.461MB
# TC: Tree: memory      2.820000   0.160000   2.980000 (  4.108988)     33.297MB
# TC: Hash: file        3.860000   3.470000   7.330000 (  9.590320)      0.285MB
# TC: Tree: file        3.540000   0.270000   3.810000 (  5.926145)      6.582MB
#
# Ruby: Debian GNU/Linux sid at 2009/04/27:
#       ruby 1.8.7 (2008-08-11 patchlevel 72) [x86_64-linux]
# groonga: HEAD at 2009/04/27: f2104358b27b80ff9e3aeff7cea09723f59c07b4
# Ruby/Groonga: trunk at 2009/04/27: r186
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
