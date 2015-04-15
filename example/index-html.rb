#!/usr/bin/env ruby

if ARGV.size < 2 or ARGV.find {|option| option == "-h" or option == "--help"}
  puts "Usage: #{$0} DATABASE_FILE [FILE_OR_DIRECTORY ...]"
  exit
end

require "pathname"

base_directory = Pathname(__FILE__).dirname + ".."
$LOAD_PATH.unshift((base_directory + "ext").to_s)
$LOAD_PATH.unshift((base_directory + "lib").to_s)

require "groonga"
require "nokogiri"

database_file, *targets = ARGV

database_file = Pathname(database_file)
database_directory = database_file.dirname
database_directory.mkpath unless database_directory.exist?

if database_file.exist?
  Groonga::Database.open(database_file.to_s)
else
  Groonga::Database.create(:path => database_file.to_s)
  Groonga::Schema.define do |schema|
    schema.create_table("documents") do |table|
      table.string("title")
      table.text("content")
      table.string("path")
      table.time("last-modified")
    end

    schema.create_table("terms",
                        :type => :patricia_trie,
                        :key_normalize => true,
                        :default_tokenizer => "TokenBigram") do |table|
      table.index("documents.title")
      table.index("documents.content")
    end
  end
end

documents = Groonga::Context.default["documents"]

targets.each do |target|
  target = Pathname(target)
  target.find do |path|
    throw :prune if path.basename.to_s == ".svn"
    if path.file? and path.extname == ".html"
      path.open do |html|
        values = {:path => path.relative_path_from(target).to_s}
        _documents = documents.select do |record|
          record["path"] == values[:path]
        end
        if _documents.size.zero?
          document = documents.add
        else
          document = _documents.to_a[0].key
        end

        html_document = Nokogiri::HTML(html)
        html_document.css("title").each do |title|
          values[:title] = title.text
        end
        contents = []
        html_document.css("body").each do |body|
          contents << body.text
        end
        html_document.css("img").each do |image|
          image_content = []
          title = image["title"]
          alt = image["alt"]
          image_content << title if title and !title.empty?
          image_content << alt if alt and !alt.empty?
          contents.concat(image_content) unless image_content.empty?
        end
        values[:content] = contents.join("\n")
        values["last-modified"] = path.mtime

        values.each do |key, value|
          document[key] = value
        end
      end
    end
  end
end
