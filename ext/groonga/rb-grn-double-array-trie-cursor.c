/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2011  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-grn.h"

VALUE rb_cGrnDoubleArrayTrieCursor;

/*
 * Document-class: Groonga::DoubleArrayCursor < Groonga::TableCursor
 *
 * Groonga::DoubleArrayに登録されているレコードを順番に取り
 * 出すためのオブジェクト。利用できるメソッドは
 * Groonga::TableCursorとGroonga::TableCursor::KeySupportを
 * 参照。
 */

void
rb_grn_init_double_array_trie_cursor (VALUE mGrn)
{
    rb_cGrnDoubleArrayTrieCursor =
        rb_define_class_under(mGrn, "DoubleArrayTrieCursor", rb_cGrnTableCursor);

    rb_include_module(rb_cGrnDoubleArrayTrieCursor,
                      rb_mGrnTableCursorKeySupport);
}
