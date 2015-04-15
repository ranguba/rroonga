#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

base_dir = File.join(File.dirname(__FILE__), "..")
groonga_ext_dir = File.join(base_dir, "ext", "groonga")
groonga_lib_dir = File.join(base_dir, "lib")
$LOAD_PATH.unshift(groonga_ext_dir)
$LOAD_PATH.unshift(groonga_lib_dir)

require "groonga"

require "time"

# Groonga::Logger.register(:level => :debug) do |level, time, title, message, location|
#   p [level, time, title, message, location]
# end

Groonga::Context.default_options = {:encoding => :utf8}

path = ARGV[0]
if path.nil?
  require "tmpdir"
  require "fileutils"
  temporary_directory = File.join(Dir.tmpdir, "rroonga")
  FileUtils.mkdir_p(temporary_directory)
  at_exit {FileUtils.rm_rf(temporary_directory)}
  path = File.join(temporary_directory, "db")
end
persistent = true

p Groonga::Database.create(:path => path)

p Groonga::Schema.create_table("Items", :type => :hash)
p(items = Groonga["Items"])

p items.size

p items.add("http://ja.wikipedia.org/wiki/Ruby")
p items.add("http://www.ruby-lang.org/ja/")

p items.size

p items["http://ja.wikipedia.org/wiki/Ruby"]

p(Groonga::Schema.change_table("Items") do |table|
    table.text("title")
  end)
p(title_column = Groonga["Items.title"])

p(Groonga::Schema.create_table("Terms",
                               :type => :patricia_trie,
                               :key_normalize => true,
                               :default_tokenizer => "TokenBigram"))
p(Groonga::Schema.change_table("Terms") do |table|
    table.index("Items.title")
  end)

p(items["http://ja.wikipedia.org/wiki/Ruby"].title = "Ruby")
p(items["http://www.ruby-lang.org/ja/"].title = "オブジェクトスクリプト言語Ruby")

p(ruby_items = items.select {|record| record.title =~ "Ruby"})
p(ruby_items.collect {|record| record.key.key})
p(ruby_items.collect {|record| record["_key"]})

p(Groonga::Schema.create_table("Users", :type => :hash) do |table|
    table.text("name")
  end)
p(Groonga::Schema.create_table("Comments") do |table|
    table.reference("item")
    table.reference("author", "Users")
    table.text("content")
    table.time("issued")
  end)

p(Groonga::Schema.change_table("Terms") do |table|
    table.index("Comments.content")
  end)

p(users = Groonga["Users"])
p(users.add("moritan", :name => "モリタン"))
p(users.add("taporobo", :name => "タポロボ"))

p(items.has_key?("http://www.rubyist.net/~matz/"))
p(items.add("http://www.rubyist.net/~matz/",
            :title => "Matzにっき"))

require "time"

p(comments = Groonga["Comments"])
p(comments.add(:item => "http://www.rubyist.net/~matz/",
               :author => "moritan",
               :content => "Ruby Matz",
               :issued => Time.parse("2010-11-20T18:01:22+09:00")))

p(@items = items)
p(@comments = comments)
def add_bookmark(url, title, author, content, issued)
  item = @items[url] || @items.add(url, :title => title)
  @comments.add(:item => item,
                :author => author,
                :content => content,
                :issued => issued)
end

p(add_bookmark("http://jp.rubyist.net/magazine/",
               "Rubyist Magazine - るびま", "moritan", "Ruby 記事",
               Time.parse("2010-10-07T14:18:28+09:00")))
p(add_bookmark("http://groonga.rubyforge.org/",
               "RubyでGroonga使って全文検索 - ラングバ", "taporobo",
               "Ruby groonga 全文検索",
               Time.parse("2010-11-11T12:39:59+09:00")))
p(add_bookmark("http://www.rubyist.net/~matz/",
               "Matz日記", "taporobo", "Ruby 日記",
               Time.parse("2010-07-28T20:46:23+09:00")))

p(records = comments.select do |record|
    record["content"] =~ "Ruby"
  end)

records.each do |record|
  comment = record.key
  p [comment.id,
     comment.issued,
     comment.item.title,
     comment.author.name,
     comment.content]
end

p records

records.sort([{:key => "issued", :order => "descending"}]).each do |record|
  comment = record.value
  p [comment.id,
     comment.issued,
     comment.item.title,
     comment.author.name,
     comment.content]
end

records.group("item").each do |record|
  item = record.key
  p [record.n_sub_records,
     item.key,
     item.title]
end

p(ruby_comments = @comments.select {|record| record.content =~ "Ruby"})
p(ruby_items = @items.select do |record|
    target = record.match_target do |match_record|
      match_record.title * 10
    end
    target =~ "Ruby"
  end)

p(ruby_items = ruby_comments.group("item").union!(ruby_items))
ruby_items.sort([{:key => "_score", :order => "descending"}]).each do |record|
  p [record.score, record.title]
end
