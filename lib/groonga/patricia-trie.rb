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
      result
    end
  end
end
