# -*- mode: ruby; coding: utf-8 -*-

base_dir = File.join(File.dirname(__FILE__), "..", "..")
$LOAD_PATH.unshift(File.join(base_dir, "ext"))
$LOAD_PATH.unshift(File.join(base_dir, "lib"))

require 'rubygems'
require 'rack'
require 'groonga'

use Rack::CommonLogger
use Rack::Static, :urls => ["/css", "/images"], :root => "public"

Groonga::Database.new("data/database")

class Searcher
  include Rack::Utils

  def initialize
    @documents = Groonga::Context.default["documents"]
  end

  def call(env)
    request = Rack::Request.new(env)
    response = Rack::Response.new
    response["Content-Type"] = "text/html"

    response.write(<<-EOH)
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
       "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="ja" lang="ja">
<head>
  <meta http-equiv="content-type" content="text/html;charset=UTF-8" />
  <meta name="robot" content="noindex,nofollow" />
  <title>groongaで全文検索</title>
  <link rel="stylesheet" href="css/groonga.css" type="text/css" media="all" />
</head>
<body>
<div class="header">
  <h1>groongaで全文検索</h1>
</div>

<div class="content">
EOH

    render_search_box(request, response)
    render_search_result(request, response)

    response.write(<<-EOF)
</div>

<div class="footer">
<p class="powered-by-groonga">
  <a href="http://groonga.org/">groonga</a>
  #{Groonga::VERSION.join('.')}を使っています。
</p>
<p class="powered-by-ruby-groonga">
  <a href="http://groonga.rubyforge.org/">Ruby/groonga</a>
  #{Groonga::BINDINGS_VERSION.join('.')}を使っています。
</p>
</div>

</body>
</html>
EOF
    response.to_a
  end

  private
  def query(request)
    request['query'] || ''
  end

  def words(request)
    query(request).split
  end

  def render_search_box(request, response)
    response.write(<<-EOF)
<form method="get" action="./">
  <p>
    <a href="."><img src="images/mini-groonga.png" alt="groonga" /></a>
    <input name="query" type="text" value="#{escape_html(query(request))}" />
    <input type="submit" value="検索" />
  </p>
</form>
EOF
  end

  def render_search_result(request, response)
    _words = words(request)
    if _words.empty?
      records = []
      response.write(<<-EOS)
  <div class='search-summary'>
    <p>Rubyでgroonga使って全文検索</p>
  </div>
EOS
    else
      offset = 0
      options = {}
      before = Time.now
      records = @documents.select do |record|
        expression = nil
        _words.each do |word|
          sub_expression = record["content"] =~ word
          if expression.nil?
            expression = sub_expression
          else
            expression &= sub_expression
          end
        end
        expression
      end
      total_records = records.size
      records = records.sort([[".:score", "descending"],
                              [".last-modified", "descending"]],
                             :limit => 20)
      elapsed = Time.now - before

      response.write(<<-EOS)
  <div class='search-summary'>
    <p>
      <span class="keyword">#{escape_html(query(request))}</span>の検索結果:
      <span class="total-entries">#{total_records}</span>件中
      <span class="display-range">
        #{total_records.zero? ? 0 : offset + 1}
        -
        #{offset + records.size}
      </span>
      件（#{elapsed}秒）
    </p>
  </div>
EOS
    end

    response.write("  <div class='records'>\n")
    records.each do |record|
      render_record(request, response, record)
    end
    response.write("  </div>\n")
  end

  def render_record(request, response, record)
    response.write("    <div class='record'>\n")
    href = escape_html(record['.path'])
    title = escape_html(record['.title'])
    last_modified = escape_html(record['.last-modified'].iso8601)
    score = record.score
    response.write("      <h2><a href='#{href}'>#{title}</a>(#{score})</h2>\n")
    render_snippet(request, response, record)
    response.write(<<-EOM)
      <p class="metadata">
        <span class="url">#{unescape(href)}</span>
        -
        <span class="last-modified">#{last_modified}</span>
      </p>
EOM
    response.write("    </div>\n")
  end

  def render_snippet(request, response, record)
    open_tag = "<span class=\"keyword\">"
    close_tag = "</span>"
    snippet = Groonga::Snippet.new(:width => 100,
                                   :default_open_tag => open_tag,
                                   :default_close_tag => close_tag,
                                   :html_escape => true,
                                   :normalize => true)
    words(request).each do |word|
      snippet.add_keyword(word)
    end
    separator = "\n<span class='separator'>...</span>\n"
    response.write(<<-EOS)
      <p class="snippet">
        #{snippet.execute(record[".content"]).join(separator)}
      </p>
EOS
  end
end

run Searcher.new
