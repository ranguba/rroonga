#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

require "rubygems"
require "groonga"

$KCODE = "UTF-8"
Groonga::Context.default_options = {:encoding => :utf8}

path = ARGV[0]
persistent = !path.nil?

Groonga::Database.create(:path => path)

items = Groonga::Hash.create(:name => "<items>", :persistent => persistent)

items.add("http://ja.wikipedia.org/wiki/Ruby")
items.add("http://www.ruby-lang.org/")

title_column = items.define_column("title", "<text>",
                                   :persistent => persistent)

terms = Groonga::Hash.create(:name => "<terms>",
                             :key_type => "<shorttext>",
                             :persistent => persistent)
terms.default_tokenizer = "<token:bigram>"
title_index_column = terms.define_index_column("item_title", items,
                                               :with_position => true,
                                               :persistent => persistent)
title_index_column.source = title_column

items.find("http://ja.wikipedia.org/wiki/Ruby")["title"] = "Ruby"
items.find("http://www.ruby-lang.org/")["title"] = "オブジェクトスクリプト言語Ruby"

p terms.collect(&:key)
p title_index_column.search("Ruby").size
p title_index_column.search("ruby").size
