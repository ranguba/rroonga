#!/usr/bin/ruby

# This benchmark is based on Tokyo Cabinet's benchmark at
# http://alpha.mixi.co.jp/blog/?p=791
#
# On my environment at 2009/07/14:
# % for x in {0..9}; do ruby benchmark/read-write-many-small-items.rb $x; done
#                           user     system      total        real        memory
# Hash                  0.850000   0.170000   1.020000 (  1.050703)     46.957MB
# groonga: Hash: memory 1.080000   0.180000   1.260000 (  1.251981)     30.152MB
# groonga: Trie: memory 1.300000   0.120000   1.420000 (  1.429549)     22.199MB
# groonga: Hash: file   1.070000   0.190000   1.260000 (  1.276910)     30.156MB
# groonga: Trie: file   1.280000   0.150000   1.430000 (  1.423066)     22.203MB
# Localmemcache: file   1.320000   0.150000   1.470000 (  1.497013)     45.984MB
# TC: Hash: memory      0.940000   0.250000   1.190000 (  1.248141)     48.758MB
# TC: Tree: memory      0.890000   0.140000   1.030000 (  1.067693)     37.270MB
# TC: Hash: file        1.740000   2.390000   4.130000 (  4.141555)      8.848MB
# TC: Tree: file        1.290000   0.180000   1.470000 (  1.476853)     12.805MB
#
# CPU: Intel(R) Core(TM)2 Duo 2.33GHz
# Memory: 2GB
# Ruby: Debian GNU/Linux sid at 2009/07/14:
#       ruby 1.8.7 (2009-06-12 patchlevel 174) [x86_64-linux]
# groonga: HEAD at 2009/07/14: fdaf58df5dd0195c10624eabee3e3f522f4af3f9
# Ruby/Groonga: trunk at 2009/07/14: r479
# Localmemcache: HEAD at 2009/07/14: 3121629016344dfd10f7533ca8ec68a0006cca21
# Tokyo Cabinet: 1.4.29
# Tokyo Cabinet Ruby: 1.27
#
# NOTE:
# * groonga, Localmemcache and Tokyo Cabinet are built with "-O3" option.
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
    ruby_hash[value]
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
      groonga_hash.column_value(value, column_name)
    end
  end

  patricia_trie = Groonga::PatriciaTrie.create(:name => "PatriciaTrie",
                                               :key_type => "ShortText")
  patricia_trie.define_column(column_name, "ShortText")
  item("groonga: PatriciaTrie") do
    values.each do |value|
      patricia_trie.set_column_value(value, column_name, value)
      patricia_trie.column_value(value, column_name)
    end
  end

  double_array_trie = Groonga::DoubleArrayTrie.create(:name => "DoubleArrayTrie",
                                                      :key_type => "ShortText")
  double_array_trie.define_column(column_name, "ShortText")
  item("groonga: DoubleArrayTrie") do
    values.each do |value|
      patricia_trie.set_column_value(value, column_name, value)
      patricia_trie.column_value(value, column_name)
    end
  end
rescue LoadError
end

begin
  require 'localmemcache'

  LocalMemCache.drop(:namespace => "read-write-many-small-items",
                     :force => true)
  mem_cache = LocalMemCache.new(:namespace => "read-write-many-small-items")
  item("Localmemcache: file") do
    values.each do |value|
      mem_cache[value] = value
      mem_cache[value]
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
      tc_hash_memory.get(value)
    end
  end

  tc_tree_memory = TokyoCabinet::ADB::new
  tc_tree_memory.open("+#bnum=#{n}#mode=wct#xmsiz=0")
  item("TC: Tree: memory") do
    values.each do |value|
      tc_tree_memory.put(value, value)
      tc_tree_memory.get(value)
    end
  end

  hash_file = Tempfile.new(["tc-hash", ".tch"])
  tc_hash_file = TokyoCabinet::ADB::new
  tc_hash_file.open("#{hash_file.path}#bnum=#{n}#mode=wct#xmsiz=0")
  item("TC: Hash: file") do
    values.each do |value|
      tc_hash_file.put(value, value)
      tc_hash_file.get(value)
    end
  end

  tree_file = Tempfile.new(["tc-tree", ".tcb"])
  tc_tree_file = TokyoCabinet::ADB::new
  tc_tree_file.open("#{tree_file.path}#bnum=#{n}#mode=wct#xmsiz=0")
  item("TC: Tree: file") do
    values.each do |value|
      tc_tree_file.put(value, value)
      tc_tree_file.get(value)
    end
  end
rescue LoadError
end

report(Integer(ARGV[0] || 0))
