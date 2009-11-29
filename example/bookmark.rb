#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

base_dir = File.join(File.dirname(__FILE__), "..")
groonga_ext_dir = File.join(base_dir, 'ext')
groonga_lib_dir = File.join(base_dir, 'lib')
$LOAD_PATH.unshift(groonga_ext_dir)
$LOAD_PATH.unshift(groonga_lib_dir)

begin
  require "groonga"
rescue LoadError
  require "rubygems"
  require "groonga"
end

require 'time'

# Groonga::Logger.register(:level => :debug) do |level, time, title, message, location|
#   p [level, time, title, message, location]
# end

$KCODE = "UTF-8"
Groonga::Context.default_options = {:encoding => :utf8}

path = ARGV[0]
if path.nil?
  require 'tmpdir'
  require 'fileutils'
  temporary_directory = File.join(Dir.tmpdir, "ruby-groonga")
  FileUtils.mkdir_p(temporary_directory)
  at_exit {FileUtils.rm_rf(temporary_directory)}
  path = File.join(temporary_directory, "db")
end
persistent = true

p Groonga::Database.create(:path => path)

p(items = Groonga::Hash.create(:name => "<items>",
                               :key_type => "ShortText",
                               :persistent => persistent))

p items.add("http://ja.wikipedia.org/wiki/Ruby")
p items.add("http://www.ruby-lang.org/")

p items.define_column("title", "<text>", :persistent => persistent)

p(terms = Groonga::Hash.create(:name => "<terms>",
                               :key_type => "<shorttext>",
                               :persistent => persistent,
                               :default_tokenizer => "TokenBigram"))
p terms.define_index_column("item_title", items,
                            :persistent => persistent,
                            :with_weight => true,
                            :with_section => true,
                            :with_position => true,
                            :source => "<items>.title")

p items.find("http://ja.wikipedia.org/wiki/Ruby")["title"] = "Ruby"
p items.find("http://www.ruby-lang.org/")["title"] = "オブジェクト指向スクリプト言語Ruby"

p(users = Groonga::Hash.create(:name => "<users>",
                               :key_type => "<shorttext>",
                               :persistent => persistent))
p users.define_column("name", "<text>",
                      :persistent => persistent)

p(comments = Groonga::Array.create(:name => "<comments>",
                                   :persistent => persistent))
p comments.define_column("item", items)
p comments.define_column("author", users)
p comments.define_column("content", "<text>")
p comments.define_column("issued", "<time>")

p terms.define_index_column("comment_content", comments,
                            :persistent => persistent,
                            :with_weight => true,
                            :with_section => true,
                            :with_position => true,
                            :source => "<comments>.content")

p users.add("moritan", :name => "モリタン")
p users.add("taporobo", :name => "タポロボ")


p items.find("http://d.hatena.ne.jp/brazil/20050829/1125321936")

p items.add("http://d.hatena.ne.jp/brazil/20050829/1125321936",
            :title => "[翻訳]JavaScript: 世界で最も誤解されたプログラミング言語")

p comments.add(:item => "http://d.hatena.ne.jp/brazil/20050829/1125321936",
               :author => "moritan",
               :content => "JavaScript LISP",
               :issued => 1187430026)

@items = items
@comments = comments
def add_bookmark(url, title, author, content, issued)
  item = @items.find(url) || @items.add(url, :title => title)
  @comments.add(:item => item,
                :author => author,
                :content => content,
                :issued => issued)
end

p add_bookmark("http://practical-scheme.net/docs/cont-j.html",
               "なんでも継続", "moritan", "継続 LISP Scheme", 1187568692)
p add_bookmark("http://d.hatena.ne.jp/higepon/20070815/1187192864",
               "末尾再帰", "taporobo", "末尾再帰 Scheme LISP", 1187568793)
p add_bookmark("http://practical-scheme.net/docs/cont-j.html",
               "なんでも継続", "taporobo", "トランポリン LISP continuation",
               1187568692.98765)
p add_bookmark("http://jp.rubyist.net/managinze",
               "るびま", "moritan", "Ruby ドキュメント",
               Time.now)
p add_bookmark("http://jp.rubyist.net/managinze",
               "るびま", "taporobo", "Ruby 雑誌",
               Time.now)
p add_bookmark("http://groonga.rubyforge.org/",
               "ラングバ", "moritan", "Ruby groonga",
               Time.parse("2009-07-19"))


records = comments.select do |record|
  record["content"] =~ "LISP"
end

records.each do |record|
  record = record.key
  p [record.id,
     record[".issued"],
     record[".item.title"],
     record[".author.name"],
     record[".content"]]
end

p records

records.sort([{:key => ".issued", :order => "descending"}]).each do |record|
  record = record.key
  p [record.id,
     record[".issued"],
     record[".item.title"],
     record[".author.name"],
     record[".content"]]
end

records.group([".item"]).each do |record|
  item = record.key
  p [record["._nsubrecs"],
     item.key,
     item[".title"]]
end

p ruby_comments = @comments.select {|record| record["content"] =~ "Ruby"}
p ruby_items = @items.select("*W1:50 title:%Ruby")

p ruby_items = ruby_comments.group([".item"]).union!(ruby_items)
ruby_items.sort([{:key => "._score", :order => "descending"}]).each do |record|
  p [record["._score"], record[".title"]]
end
