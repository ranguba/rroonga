# -*- coding: utf-8 -*-
#
# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

class SnippetTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_new_without_arguments
    assert_nothing_raised do
      Groonga::Snippet.new
    end
  end

  def test_execute
    snippet = Groonga::Snippet.new(:encoding => :utf8)
    snippet.add_keyword("検索", :open_tag => "[[", :close_tag => "]]")
    assert_equal(["groonga は組み込み型の全文[[検索]]エンジンライブラリです。DBMSやスクリプト",
                  "組み込むことによって、その全文[[検索]]機能を強化することができます。ま"],
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
