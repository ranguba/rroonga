# -*- coding: utf-8 -*-
#
# Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

class SnippetTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup
  def setup_database
    @database = Groonga::Database.create
  end

  def setup_encoding
    Groonga::Encoding.default = :utf8
  end

  def test_new_without_arguments
    assert_nothing_raised do
      Groonga::Snippet.new
    end
  end

  def test_execute
    snippet = Groonga::Snippet.new
    snippet.add_keyword("検索", :open_tag => "[[", :close_tag => "]]")
    assert_equal(["groonga は組み込み型の全文[[検索]]エンジンライブラリです。DBMSやスクリプト",
                  "組み込むことによって、その全文[[検索]]機能を強化することができます。ま"],
                 snippet.execute(text))
  end

  def test_execute_with_nil
    snippet = Groonga::Snippet.new
    snippet.add_keyword("検索", :open_tag => "[[", :close_tag => "]]")
    message = "snippet text must be String: <nil>"
    assert_raise(Groonga::InvalidArgument.new(message)) do
      snippet.execute(nil)
    end
  end

  def test_invalid_encoding
    Groonga::Context.default.encoding = :shift_jis
    snippet = Groonga::Snippet.new
    snippet.add_keyword("検索")
    assert_equal([], snippet.execute(text))
  end

  def test_default_tag
    snippet = Groonga::Snippet.new(:default_open_tag => "<<",
                                   :default_close_tag => ">>")
    snippet.add_keyword("全文")
    assert_equal(["groonga は組み込み型の<<全文>>検索エンジンライブラリです。DBMSやスクリプト",
                  "等に組み込むことによって、その<<全文>>検索機能を強化することができます"],
                 snippet.execute(text))
  end

  def test_width
    snippet = Groonga::Snippet.new(:width => 30,
                                   :default_open_tag => "{",
                                   :default_close_tag => "}")
    snippet.add_keyword("データ")
    assert_equal(["基づく{データ}ストア機",
                  "高速\nな{データ}ストア"],
                 snippet.execute(text))
  end

  def test_max_results
    snippet = Groonga::Snippet.new(:width => 30,
                                   :max_results => 1,
                                   :default_open_tag => "{",
                                   :default_close_tag => "}")
    snippet.add_keyword("データ")
    assert_equal(["基づく{データ}ストア機"],
                 snippet.execute(text))
  end

  def test_normalize
    options_without_normalize = {
      :width => 30,
      :default_open_tag => "{",
      :default_close_tag => "}",
    }
    snippet = Groonga::Snippet.new(options_without_normalize)
    snippet.add_keyword("処理系等")
    assert_equal([], snippet.execute(text))

    options_with_normalize = options_without_normalize.merge(:normalize => true)
    snippet = Groonga::Snippet.new(options_with_normalize)
    snippet.add_keyword("処理系等")
    assert_equal(["ト言語{処理\n系等}に組"],
                 snippet.execute(text))
  end

  def test_html_escape
    text = "groonga は組み込み型の全文検索エンジン&データストアライブラリです。"
    options_without_html_escape = {
      :width => 30,
      :default_open_tag => "<",
      :default_close_tag => ">",
    }
    snippet = Groonga::Snippet.new(options_without_html_escape)
    snippet.add_keyword("エンジン")
    assert_equal(["文検索<エンジン>&デー"],
                 snippet.execute(text))

    options_with_html_escape =
      options_without_html_escape.merge(:html_escape => true)
    snippet = Groonga::Snippet.new(options_with_html_escape)
    snippet.add_keyword("エンジン")
    assert_equal(["文検索<エンジン>&amp;デー"],
                 snippet.execute(text))
  end

  private
  def text
    <<-EOT
groonga は組み込み型の全文検索エンジンライブラリです。DBMSやスクリプト言語処理
系等に組み込むことによって、その全文検索機能を強化することができます。また、リ
レーショナルモデルに基づくデータストア機能を内包しており、groonga単体でも高速
なデータストアサーバとして使用することができます。
EOT
  end
end
