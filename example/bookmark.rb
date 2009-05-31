#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

require "rubygems"
require "groonga"

$KCODE = "UTF-8"
Groonga::Context.default_options = {:encoding => :utf8}

path = ARGV[0]
persistent = !path.nil?

p Groonga::Database.create(:path => path)

p(items = Groonga::Hash.create(:name => "<items>", :persistent => persistent))

p items.add("http://ja.wikipedia.org/wiki/Ruby")
p items.add("http://www.ruby-lang.org/")

p items.define_column("title", "<text>", :persistent => persistent)

p(terms = Groonga::Hash.create(:name => "<terms>",
                               :key_type => "<shorttext>",
                               :persistent => persistent,
                               :default_tokenizer => "<token:bigram>"))
p terms.define_index_column("item_title", items,
                            :persistent => persistent,
                            :source => "<items>.title")

p items.find("http://ja.wikipedia.org/wiki/Ruby")["title"] = "Ruby"
p items.find("http://www.ruby-lang.org/")["title"] = "オブジェクトスクリプト言語Ruby"

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
               1187568692)

terms.column("comment_content").search("LISP").each do |record|
  record = record.key
  p [record[".issued"], record[".item.title"], record[".author.name"],
     record[".content"]]
end
