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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

module Groonga
  class PatriciaTrie
    # _text_ を走査し、レコードのキーとマッチする部分文字列ごとに
    # そのレコードが _record_ として、その部分文字列が _word_ として、
    # ブロックが呼び出される。ブロックから返された文字列が元の部
    # 分文字列と置換される。全てのヒットに対してのその置換処理が
    # 行われた文字列が返される。
    #
    # @param options [::Hash] The name and value
    #   pairs. Omitted names are initialized as the default value.
    # @option options [Proc] :other_text_handler The other_text_handler
    #
    #  マッチした部分文字列の前後の文字列を変換するProcを指定する。
    #
    # @example
    #   include ERB::Util
    #   Groonga::Context.default_options = {:encoding => "utf-8"}
    #   words = Groonga::PatriciaTrie.create(:key_type => "ShortText",
    #                                        :key_normalize => true)
    #   words.add('ｶﾞｯ')
    #   words.add('ＭＵＴＥＫＩ')
    #
    #   text = 'muTEki マッチしない <> ガッ'
    #   other_text_handler = Proc.new do |string|
    #     h(string)
    #   end
    #   words.tag_keys(text) do |record, word|
    #     "<span class=\"keyword\">#{h(word)}(#{h(record.key)})</span>\n"
    #   end
    #   # =>
    #   # "<span class=\"keyword\">muTEki(muteki)</span>\n" +
    #   # " マッチしない &lt;&gt; " +
    #   # "<span class=\"keyword\">ガッ(ガッ)</span>\n"
    def tag_keys(text, options={})
      options ||= {}
      other_text_handler = options[:other_text_handler]
      position = 0
      result = ''
      if text.respond_to?(:encoding)
        encoding = text.encoding
        bytes = text.dup.force_encoding("ascii-8bit")
      else
        encoding = nil
        bytes = text
      end
      scan(text) do |record, word, start, length|
        previous_text = bytes[position...start]
        previous_text.force_encoding(encoding) if encoding
        if other_text_handler
          previous_text = other_text_handler.call(previous_text)
        end
        result << previous_text
        result << yield(record, word)
        position = start + length
      end
      last_text = bytes[position..-1]
      unless last_text.empty?
        last_text.force_encoding(encoding) if encoding
        last_text = other_text_handler.call(last_text) if other_text_handler
        result << last_text
      end
      result
    end
  end
end
