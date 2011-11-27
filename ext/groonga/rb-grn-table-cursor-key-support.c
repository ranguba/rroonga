/* -*- coding: utf-8; c-file-style: "ruby" -*- */
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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-grn.h"

#define SELF(object) ((RbGrnTableCursor *)DATA_PTR(object))

VALUE rb_mGrnTableCursorKeySupport;

/*
 * Document-module: Groonga::TableCursor::KeySupport
 *
 * 主キーを持つテーブル用のカーソルであるGroonga::HashCursor
 * とGroonga::PatriciaTrieCursorに主キーの機能を提供するモジ
 * ュール。
 */

/*
 * call-seq:
 *   cursor.key -> 主キー
 *
 * カレントレコードの主キーを返す。
 */
static VALUE
rb_grn_table_cursor_get_key (VALUE self)
{
    VALUE rb_key = Qnil;
    grn_ctx *context;
    grn_table_cursor *cursor;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);
    if (context && cursor) {
        void *key;
        int key_size;
	grn_obj *table;

	table = grn_table_cursor_table(context, cursor);
        key_size = grn_table_cursor_get_key(context, cursor, &key);
        rb_key = GRNKEY2RVAL(context, key, key_size, table, self);
    }

    return rb_key;
}

void
rb_grn_init_table_cursor_key_support (VALUE mGrn)
{
    rb_mGrnTableCursorKeySupport =
	rb_define_module_under(rb_cGrnTableCursor, "KeySupport");

    rb_define_method(rb_mGrnTableCursorKeySupport, "key",
                     rb_grn_table_cursor_get_key, 0);
}
