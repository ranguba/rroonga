/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>

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

VALUE rb_cGrnTableCursor;

/*
 * Document-class: Groonga::TableCursor
 *
 * テーブルに登録されているレコードを順番に取り出すための
 * オブジェクト。Groonga::Table#open_cursorで生成できる。
 */

grn_table_cursor *
rb_grn_table_cursor_from_ruby_object (VALUE object, grn_ctx **context)
{
    grn_table_cursor *table_cursor;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnTableCursor))) {
	rb_raise(rb_eTypeError, "not a groonga table cursor");
    }

    rb_grn_table_cursor_deconstruct(SELF(object), &table_cursor, NULL,
				    NULL, NULL,
				    NULL, NULL);
    return table_cursor;
}

VALUE
rb_grn_table_cursor_to_ruby_object (VALUE klass, grn_ctx *context,
				    grn_table_cursor *cursor,
				    grn_bool owner)
{
    return GRNOBJECT2RVAL(klass, context, cursor, owner);
}

void
rb_grn_table_cursor_deconstruct (RbGrnTableCursor *rb_grn_table_cursor,
				 grn_table_cursor **cursor,
				 grn_ctx **context,
				 grn_id *domain_id,
				 grn_obj **domain,
				 grn_id *range_id,
				 grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_table_cursor);
    rb_grn_object_deconstruct(rb_grn_object, cursor, context,
			      domain_id, domain,
			      range_id, range);
}

int
rb_grn_table_cursor_order_to_flag (VALUE rb_order)
{
    int flag = 0;

    if (NIL_P(rb_order) ||
	rb_grn_equal_option(rb_order, "asc") ||
	rb_grn_equal_option(rb_order, "ascending")) {
	flag |= GRN_CURSOR_ASCENDING;
    } else if (rb_grn_equal_option(rb_order, "desc") ||
	       rb_grn_equal_option(rb_order, "descending")) {
	flag |= GRN_CURSOR_DESCENDING;
    } else {
	rb_raise(rb_eArgError,
		 "order should be one of "
		 "[:asc, :ascending, :desc, :descending]: %s",
		 rb_grn_inspect(rb_order));
    }

    return flag;
}

int
rb_grn_table_cursor_order_by_to_flag (unsigned char table_type,
				      VALUE rb_table,
				      VALUE rb_order_by)
{
    int flag = 0;

    if (NIL_P(rb_order_by)) {
	if (table_type == GRN_TABLE_PAT_KEY) {
	    flag |= GRN_CURSOR_BY_KEY;
	} else {
	    flag |= GRN_CURSOR_BY_ID;
	}
    } else if (rb_grn_equal_option(rb_order_by, "id")) {
	flag |= GRN_CURSOR_BY_ID;
    } else if (rb_grn_equal_option(rb_order_by, "key")) {
	if (table_type != GRN_TABLE_PAT_KEY) {
	    rb_raise(rb_eArgError,
		     "order_by => :key is available "
		     "only for Groonga::PatriciaTrie: %s",
		     rb_grn_inspect(rb_table));
	}
	flag |= GRN_CURSOR_BY_KEY;
    } else {
	rb_raise(rb_eArgError,
		 "order_by should be one of [:id%s]: %s",
		 table_type == GRN_TABLE_PAT_KEY ? ", :key" : "",
		 rb_grn_inspect(rb_order_by));
    }

    return flag;
}

/*
 * call-seq:
 *   cursor.value -> 値
 *
 * カレントレコードの値を返す。
 */
static VALUE
rb_grn_table_cursor_get_value (VALUE self)
{
    grn_ctx *context;
    grn_table_cursor *cursor;
    VALUE rb_value = Qnil;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);
    if (context && cursor) {
        int n;
        void *value;

        n = grn_table_cursor_get_value(context, cursor, &value);
        rb_value = rb_str_new(value, n);
    }

    return rb_value;
}

/*
 * call-seq:
 *   cursor.value = 値
 *
 * カレントレコードの値を設定する。既存の値は上書きされる。
 */
static VALUE
rb_grn_table_cursor_set_value (VALUE self, VALUE value)
{
    grn_ctx *context;
    grn_table_cursor *cursor;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);
    if (context && cursor) {
        grn_rc rc;

        rc = grn_table_cursor_set_value(context,
                                        cursor,
                                        StringValuePtr(value),
					GRN_OBJ_SET);
        rb_grn_rc_check(rc, self);
    }

    return Qnil;
}

/*
 * call-seq:
 *   table_cursor.delete
 *
 * カレントレコードを削除する。
 */
static VALUE
rb_grn_table_cursor_delete (VALUE self)
{
    grn_ctx *context;
    grn_table_cursor *cursor;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);
    if (context && cursor) {
        grn_rc rc;

        rc = grn_table_cursor_delete(context, cursor);
        rb_grn_rc_check(rc, self);
    }

    return Qnil;
}

/*
 * call-seq:
 *   table_cursor.next -> Groonga::Record
 *
 * カレントレコードを一件進めてそのレコードを返す。
 */
static VALUE
rb_grn_table_cursor_next (VALUE self)
{
    VALUE rb_record = Qnil;
    grn_ctx *context;
    grn_table_cursor *cursor;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);
    if (context && cursor) {
        grn_id record_id;

        record_id = grn_table_cursor_next(context, cursor);
        if (record_id != GRN_ID_NIL) /* FIXME: use grn_table_cursor_table */
            rb_record = rb_grn_record_new(rb_iv_get(self, "@table"),
					  record_id, Qnil);
    }

    return rb_record;
}

/*
 * call-seq:
 *   table_cursor.each {|record| ...}
 *
 * カーソルの範囲内にあるレコードを順番にブロックに渡す。
 */
static VALUE
rb_grn_table_cursor_each (VALUE self)
{
    grn_id record_id;
    grn_ctx *context;
    grn_table_cursor *cursor;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);

    if (context && cursor) {
	while ((record_id = grn_table_cursor_next(context, cursor))) {
	    rb_yield(rb_grn_record_new(rb_iv_get(self, "@table"),
				       record_id, Qnil));
	}
    }

    return Qnil;
}

/*
 * Document-method: close
 *
 * call-seq:
 *   cursor.close
 *
 * カーソルが使用しているリソースを開放する。これ以降カーソルを
 * 使うことはできない。
 */

/*
 * Document-method: closed?
 *
 * call-seq:
 *   cursor.closed? -> true/false
 *
 * カーソルが開放済みの場合は +true+ を返し、そうでない場合は
 * +false+ を返す。
 */

void
rb_grn_init_table_cursor (VALUE mGrn)
{
    rb_cGrnTableCursor = rb_define_class_under(mGrn, "TableCursor", rb_cObject);
    rb_define_alloc_func(rb_cGrnTableCursor, rb_grn_object_alloc);

    rb_include_module(rb_cGrnTableCursor, rb_mEnumerable);

    rb_define_method(rb_cGrnTableCursor, "close",
                     rb_grn_object_close, 0);
    rb_define_method(rb_cGrnTableCursor, "closed?",
                     rb_grn_object_closed_p, 0);

    rb_define_method(rb_cGrnTableCursor, "value",
                     rb_grn_table_cursor_get_value, 0);
    rb_define_method(rb_cGrnTableCursor, "value=",
                     rb_grn_table_cursor_set_value, 1);
    rb_define_method(rb_cGrnTableCursor, "delete",
                     rb_grn_table_cursor_delete, 0);
    rb_define_method(rb_cGrnTableCursor, "next",
                     rb_grn_table_cursor_next, 0);

    rb_define_method(rb_cGrnTableCursor, "each",
                     rb_grn_table_cursor_each, 0);

    rb_grn_init_table_cursor_key_support(mGrn);
    rb_grn_init_array_cursor(mGrn);
    rb_grn_init_hash_cursor(mGrn);
    rb_grn_init_patricia_trie_cursor(mGrn);
    rb_grn_init_double_array_trie_cursor(mGrn);
    rb_grn_init_view_cursor(mGrn);
}
