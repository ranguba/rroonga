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

module Groonga
  class PatriciaTrie
    # call-seq:
    #   patricia_trie.tag_keys(text) {|record, word| ...} -> String
    #
    # _text_を走査し、レコードのキーとマッチする部分文字列ごとに
    # そのレコードが_record_として、その部分文字列が_word_として、
    # ブロックが呼び出される。ブロックから返された文字列が元の部
    # 分文字列と置換される。全てのヒットに対してのその置換処理が
    # 行われた文字列が返される。
    def tag_keys(text)
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
        result << previous_text
        result << yield(record, word)
        position = start + length
      end
      last_text = bytes[position..-1]
      unless last_text.empty?
        last_text.force_encoding(encoding) if encoding
        result << last_text
      end
      result
    end
  end
end
