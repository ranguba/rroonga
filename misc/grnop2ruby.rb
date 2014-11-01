# -*- coding: utf-8 -*-
#
# Copyright (C) 2009  Yuto Hayamizu <y.hayamizu@gmail.com>
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

#
# This script expects "groonga.h" as a first argument, extracts the
# 'grn_operator', and write 'rb_define_const's of 'grn_operator' to
# standard outout.
#
# Usage:
#   ruby grnop2ruby.rb /path/to/groonga.h
#

replace_dictionary = {
  "VAR" => "VARIABLE",
  "EXPR" => "EXPRESSION",
  "NOP" => "NO_OPERATION",
  "REF" => "REFERENCE",
  "OBJ" => "OBJECT",
  "INCR" => "INCREMENT",
  "DECR" => "DECREMENT",
  "MOD" => "MODULO",
  "LCP" => "LONGEST_COMMON_PREFIX",
}

ARGF.each_line do |line|
  case line
  when /\A\s+(GRN_OP_\w+)/
    operator = $1
    rb_operator = operator.gsub(/\AGRN_OP_/, "").split("_").map {|word|
      replace_dictionary[word] || word
    }.join("_")
    puts "    rb_define_const(rb_mGrnOperation, \"%s\",
                    UINT2NUM(%s));" % [rb_operator, operator]
  end
end
