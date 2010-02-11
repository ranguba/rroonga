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
    response["Content-Type"] = "text/html; charset=UTF-8"
    if /\/\z/ !~ request.path_info
      request.path_info += "/"
      response.redirect(request.url)
      return response.to_a
    end

    response.write(<<-EOH)
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
       "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="ja" lang="ja">
<head>
  <meta http-equiv="content-type" content="text/html; charset=UTF-8" />
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

  def page(request)
    (request['page'] || 0).to_i
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
    _query = query(request)
    _page = page(request)
    limit = 20
    if _query.empty?
      records = []
      response.write(<<-EOS)
  <div class='search-summary'>
    <p>Rubyでgroonga使って全文検索</p>
  </div>
EOS
    else
      options = {}
      before = Time.now
      records = @documents.select do |record|
        record["content"].match(_query)
      end
      total_records = records.size
      records = records.sort([["_score", "descending"],
                              ["last-modified", "descending"]],
                             :offset => _page * limit,
                             :limit => limit)
      elapsed = Time.now - before

      response.write(<<-EOS)
  <div class='search-summary'>
    <p>
      <span class="keyword">#{escape_html(query(request))}</span>の検索結果:
      <span class="total-entries">#{total_records}</span>件中
      <span class="display-range">
        #{total_records.zero? ? 0 : (_page * limit) + 1}
        -
        #{(_page * limit) + records.size}
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

    render_pagination(request, response, _page, limit, total_records)
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
    expression = record.table.expression
    snippet = expression.snippet([["<span class=\"keyword\">", "</span>"]],
                                 :width => 100,
                                 :html_escape => true,
                                 :normalize => true)
    separator = "\n<span class='separator'>...</span>\n"
    response.write(<<-EOS)
      <p class="snippet">
        #{snippet.execute(record[".content"]).join(separator)}
      </p>
EOS
  end

  def render_pagination(request, response, page, limit, total_records)
    _query = query(request)
    return if _query.empty?

    return if total_records < limit

    last_page = (total_records / limit.to_f).ceil
    response.write("<div class='pagination'>\n")
    if page > 0
      render_pagination_link(request, response, _query, page - 1, "<<")
    end
    last_page.times do |i|
      if i == page
        response.write(pagination_span(escape_html(i)))
      else
        render_pagination_link(request, response, _query, i, i)
      end
    end
    if page < (last_page - 1)
      render_pagination_link(request, response, _query, page + 1, ">>")
    end
    response.write("</div>\n")
  end

  def render_pagination_link(request, response, query, page, label)
    href = "./?query=#{escape_html(query)};page=#{escape_html(page)}"
    response.write(pagination_span("<a href='#{href}'>#{label}</a>"))
  end

  def pagination_span(content)
    "<span class='pagination-link'>#{content}</span>\n"
  end
end

run Searcher.new
