/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>

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

VALUE rb_cGrnViewCursor;

/*
 * Document-class: Groonga::ViewCursor < Groonga::TableCursor
 *
 * Groonga::Viewからレコードを順番に取り出すためのオブジェク
 * ト。利用できるメソッドはGroonga::TableCursorを参照。
 */

void
rb_grn_init_view_cursor (VALUE mGrn)
{
    rb_cGrnViewCursor =
        rb_define_class_under(mGrn, "ViewCursor", rb_cGrnTableCursor);
}
