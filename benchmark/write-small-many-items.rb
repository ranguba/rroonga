#!/usr/bin/ruby

# This benchmark is based on Tokyo Cabinet's benchmark at
# http://alpha.mixi.co.jp/blog/?p=791
#
# On my environment at 2009/05/16:
# % for x in {0..9}; do ruby benchmark/write-small-many-items.rb $x; done
#                           user     system      total        real        memory
# Hash                  1.090000   0.180000   1.270000 (  1.565159)     47.090MB
# groonga: Hash: memory 0.930000   0.140000   1.070000 (  1.364338)     23.473MB
# groonga: Trie: memory 1.170000   0.120000   1.290000 (  1.601229)     15.480MB
# groonga: Hash: file   0.980000   0.170000   1.150000 (  1.398126)     23.477MB
# groonga: Trie: file   1.220000   0.130000   1.350000 (  1.744243)     15.484MB
# Localmemcache: file   1.100000   0.210000   1.310000 (  1.734014)     39.301MB
# TC: Hash: memory      0.710000   0.130000   0.840000 (  1.061472)     42.055MB
# TC: Tree: memory      0.640000   0.150000   0.790000 (  1.040223)     30.566MB
# TC: Hash: file        1.310000   2.370000   3.680000 (  5.028978)      2.133MB
# TC: Tree: file        1.030000   0.200000   1.230000 (  1.666725)      6.086MB
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

require File.join(File.dirname(__FILE__), "common.rb")

n = 500000

values = []
n.times do |i|
  values << "%08d" % i
end

item("Hash") do
  @hash = {}
  values.each do |value|
    @hash[value] = value
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
    end
  end

  item("groonga: Trie: memory") do
    @hash = Groonga::PatriciaTrie.create(:key_type => "<shorttext>",
                                         :value_size => 8)
    values.each do |value|
      @hash[value] = value
    end
  end

  @hash_file = Tempfile.new("groonga-hash")
  item("groonga: Hash: file") do
    @hash = Groonga::Hash.create(:key_type => "<shorttext>",
                                 :value_size => 8,
                                 :path => @hash_file.path)
    values.each do |value|
      @hash[value] = value
    end
  end

  trie_file = Tempfile.new("groonga-trie")
  item("groonga: Trie: file") do
    @hash = Groonga::PatriciaTrie.create(:key_type => "<shorttext>",
                                         :value_size => 8,
                                         :path => trie_file.path)
    values.each do |value|
      @hash[value] = value
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
    end
  end

  item("TC: Tree: memory") do
    @db = TokyoCabinet::ADB::new
    @db.open("+#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      @db.put(value, value)
    end
  end

  hash_file = Tempfile.new(["tc-hash", ".tch"])
  item("TC: Hash: file") do
    @db = TokyoCabinet::ADB::new
    @db.open("#{hash_file.path}#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      @db.put(value, value)
    end
  end

  tree_file = Tempfile.new(["tc-tree", ".tcb"])
  item("TC: Tree: file") do
    @db = TokyoCabinet::ADB::new
    @db.open("#{tree_file.path}#bnum=#{n}#mode=wct#xmsiz=0")
    values.each do |value|
      @db.put(value, value)
    end
  end
rescue LoadError
end

report(Integer(ARGV[0] || 0))
