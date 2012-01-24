#!/usr/bin/ruby

# This benchmark is based on Tokyo Cabinet's benchmark at
# http://alpha.mixi.co.jp/blog/?p=791
#
# On my environment at 2012/01/25:
# % for x in {0..8}; do ruby1.9.1 benchmark/write-many-small-items.rb $x; done
#                              user     system      total        real     memory
# Hash                     0.320000   0.030000   0.350000 (  0.345441)  44.910MB
# groonga: Hash            0.490000   0.020000   0.510000 (  0.512336)  23.305MB
# groonga: PatriciaTrie    0.560000   0.010000   0.570000 (  0.572077)  19.242MB
# groonga: DoubleArrayTrie 0.700000   0.020000   0.720000 (  0.730448)  39.789MB
# Localmemcache: file      0.510000   0.040000   0.550000 (  0.549663)  45.898MB
# TC: Hash: memory         0.270000   0.020000   0.290000 (  0.292051)  40.148MB
# TC: Tree: memory         0.220000   0.010000   0.230000 (  0.229233)  28.727MB
# TC: Hash: file           0.610000   0.630000   1.240000 (  1.247094)   2.129MB
# TC: Tree: file           0.340000   0.040000   0.380000 (  0.377910)   4.000MB
#
# CPU: Intel(R) Core(TM) i7 CPU         860  @ 2.80GHz stepping 05
# Memory: 8GB
# Ruby: Debian GNU/Linux sid at 2012/01/25:
#       ruby 1.9.3p0 (2011-10-30 revision 33570) [x86_64-linux]
# groonga: HEAD at 2012/01/25: 07dce043cc8cfbbecbcdc43bf7c6d5c2d085237a
# rroonga: master at 2012/01/25: 29099bff93377733d81dc2398c3403e1293301dc
# Localmemcache: 0.4.4
# Tokyo Cabinet: 1.4.37
# Tokyo Cabinet Ruby: 1.31
#
# NOTE:
# * groonga, Localmemcache and Tokyo Cabinet are built with "-O2" option.
# * Ruby bindings of them are built with "-O2" option.

require File.join(File.dirname(__FILE__), "common.rb")

n = 500000

values = []
n.times do |i|
  values << "%08d" % i
end

ruby_hash = {}
item("Hash") do
  values.each do |value|
    ruby_hash[value] = value
  end
end

begin
  base_dir = File.expand_path(File.join(File.dirname(__FILE__), ".."))
  $LOAD_PATH.unshift(File.join(base_dir, "ext", "groonga"))
  $LOAD_PATH.unshift(File.join(base_dir, "lib"))

  require 'groonga'
  tmp_dir = "/tmp/groonga"
  FileUtils.rm_rf(tmp_dir)
  FileUtils.mkdir(tmp_dir)
  Groonga::Context.default.encoding = :none
  @database = Groonga::Database.create(:path => "#{tmp_dir}/db")
  column_name = "value"

  groonga_hash = Groonga::Hash.create(:name => "Hash",
                                      :key_type => "ShortText")
  groonga_hash.define_column(column_name, "ShortText")
  item("groonga: Hash") do
    values.each do |value|
      groonga_hash.set_column_value(value, column_name, value)
    end
  end

  patricia_trie = Groonga::PatriciaTrie.create(:name => "PatriciaTrie",
                                               :key_type => "ShortText")
  patricia_trie.define_column(column_name, "ShortText")
  item("groonga: PatriciaTrie") do
    values.each do |value|
      patricia_trie.set_column_value(value, column_name, value)
    end
  end

  double_array_trie = Groonga::DoubleArrayTrie.create(:name => "DoubleArrayTrie",
                                                      :key_type => "ShortText")
  double_array_trie.define_column(column_name, "ShortText")
  item("groonga: DoubleArrayTrie") do
    values.each do |value|
      double_array_trie.set_column_value(value, column_name, value)
    end
  end
rescue LoadError
end

begin
  require 'localmemcache'

  LocalMemCache.drop(:namespace => "write-many-small-items", :force => true)
  mem_cache = LocalMemCache.new(:namespace => "write-many-small-items")
  item("Localmemcache: file") do
    values.each do |value|
      mem_cache[value] = value
    end
  end
rescue LoadError
end

begin
  require 'tokyocabinet'

  tc_hash_memory = TokyoCabinet::ADB::new
  tc_hash_memory.open("*#bnum=#{n}#mode=wct#xmsiz=0")
  item("TC: Hash: memory") do
    values.each do |value|
      tc_hash_memory.put(value, value)
    end
  end

  tc_tree_memory = TokyoCabinet::ADB::new
  tc_tree_memory.open("+#bnum=#{n}#mode=wct#xmsiz=0")
  item("TC: Tree: memory") do
    values.each do |value|
      tc_tree_memory.put(value, value)
    end
  end

  hash_file = Tempfile.new(["tc-hash", ".tch"])
  tc_hash_file = TokyoCabinet::ADB::new
  tc_hash_file.open("#{hash_file.path}#bnum=#{n}#mode=wct#xmsiz=0")
  item("TC: Hash: file") do
    values.each do |value|
      tc_hash_file.put(value, value)
    end
  end

  tree_file = Tempfile.new(["tc-tree", ".tcb"])
  tc_tree_file = TokyoCabinet::ADB::new
  tc_tree_file.open("#{tree_file.path}#bnum=#{n}#mode=wct#xmsiz=0")
  item("TC: Tree: file") do
    values.each do |value|
      tc_tree_file.put(value, value)
    end
  end
rescue LoadError
end

report(Integer(ARGV[0] || 0))
