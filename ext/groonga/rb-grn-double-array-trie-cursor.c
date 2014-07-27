/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "rb-grn.h"

VALUE rb_cGrnDoubleArrayTrieCursor;

/*
 * Document-class: Groonga::DoubleArrayTrieCursor < Groonga::TableCursor
 *
 * {Groonga::DoubleArrayTrie} に登録されているレコードを順番に取り
 * 出すためのオブジェクト。利用できるメソッドは
 * {Groonga::TableCursor} と {Groonga::TableCursor::KeySupport} を
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
