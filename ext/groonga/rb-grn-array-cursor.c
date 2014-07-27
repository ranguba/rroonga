/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>

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

VALUE rb_cGrnArrayCursor;

/*
 * Document-class: Groonga::ArrayCursor < Groonga::TableCursor
 *
 * {Groonga::Array} に登録されているレコードを順番に取り出すため
 * のオブジェクト。利用できるメソッドは {Groonga::TableCursor} を
 * 参照。
 */

void
rb_grn_init_array_cursor (VALUE mGrn)
{
    rb_cGrnArrayCursor =
        rb_define_class_under(mGrn, "ArrayCursor", rb_cGrnTableCursor);
}
