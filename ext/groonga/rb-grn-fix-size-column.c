/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>

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

#define SELF(object) ((RbGrnColumn *)DATA_PTR(object))

VALUE rb_cGrnFixSizeColumn;

/*
 * Document-class: Groonga::FixSizeColumn < Groonga::Column
 *
 * 固定長データ用のカラム。
 */

/*
 * call-seq:
 *   column[id] -> 値
 *
 * _column_ の _id_ に対応する値を返す。
 */
VALUE
rb_grn_fix_size_column_array_reference (VALUE self, VALUE rb_id)
{
    grn_id id;
    grn_ctx *context;
    grn_obj *fix_size_column;
    grn_obj *range;
    grn_obj *value;

    rb_grn_column_deconstruct(SELF(self), &fix_size_column, &context,
			      NULL, NULL,
			      &value, NULL, &range);

    id = NUM2UINT(rb_id);
    GRN_BULK_REWIND(value);
    grn_obj_get_value(context, fix_size_column, id, value);
    rb_grn_context_check(context, self);

    return GRNVALUE2RVAL(context, value, range, self);
}

/*
 * call-seq:
 *   column[id] = value
 *
 * _column_ の _id_ に対応する値を設定する。
 */
static VALUE
rb_grn_fix_size_column_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_id domain_id, range_id;
    grn_obj *domain, *range;
    grn_obj *value;
    grn_rc rc;
    grn_id id;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
			      &domain_id, &domain,
			      &value, &range_id, &range);

    id = NUM2UINT(rb_id);
    RVAL2GRNBULK_WITH_TYPE(rb_value, context, value, range_id, range);

    rc = grn_obj_set_value(context, column, id, value, GRN_OBJ_SET);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

static VALUE
rb_grn_fix_size_column_integer_set (int argc, VALUE *argv, VALUE self, int flags)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj *value;
    grn_rc rc;
    grn_id id;
    VALUE rb_id, rb_delta;

    rb_scan_args(argc, argv, "11", &rb_id, &rb_delta);

    rb_grn_column_deconstruct(SELF(self), &column, &context,
			      NULL, NULL,
			      &value, NULL, NULL);

    id = NUM2UINT(rb_id);
    if (NIL_P(rb_delta))
	rb_delta = INT2NUM(1);

    GRN_BULK_REWIND(value);
    RVAL2GRNBULK(rb_delta, context, value);

    rc = grn_obj_set_value(context, column, id, value, flags);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   column.increment!(id, delta=nil)
 *
 * _column_ の _id_ に対応する値を _delta_ だけ増加する。 _delta_
 * が +nil+ の場合は1増加する。
 */
static VALUE
rb_grn_fix_size_column_increment (int argc, VALUE *argv, VALUE self)
{
    return rb_grn_fix_size_column_integer_set(argc, argv, self, GRN_OBJ_INCR);
}

/*
 * call-seq:
 *   column.decrement!(id, delta=nil)
 *
 * _column_ の _id_ に対応する値を _delta_ だけ減少する。 _delta_
 * が +nil+ の場合は1減少する。
 */
static VALUE
rb_grn_fix_size_column_decrement (int argc, VALUE *argv, VALUE self)
{
    return rb_grn_fix_size_column_integer_set(argc, argv, self, GRN_OBJ_DECR);
}

void
rb_grn_init_fix_size_column (VALUE mGrn)
{
    rb_cGrnFixSizeColumn =
	rb_define_class_under(mGrn, "FixSizeColumn", rb_cGrnColumn);

    rb_define_method(rb_cGrnFixSizeColumn, "[]",
		     rb_grn_fix_size_column_array_reference, 1);
    rb_define_method(rb_cGrnFixSizeColumn, "[]=",
		     rb_grn_fix_size_column_array_set, 2);

    rb_define_method(rb_cGrnFixSizeColumn, "increment!",
		     rb_grn_fix_size_column_increment, -1);
    rb_define_method(rb_cGrnFixSizeColumn, "decrement!",
		     rb_grn_fix_size_column_decrement, -1);
}
