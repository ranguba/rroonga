/* -*- c-file-style: "ruby" -*- */
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

#define SELF(object) ((RbGrnFixSizeColumn *)DATA_PTR(object))

VALUE rb_cGrnFixSizeColumn;

/*
 * Document-class: Groonga::FixSizeColumn < Groonga::Column
 *
 * 固定長データ用のカラム。
 */

void
rb_grn_fix_size_column_unbind (RbGrnFixSizeColumn *rb_grn_fix_size_column)
{
    RbGrnObject *rb_grn_object;
    grn_ctx *context;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_fix_size_column);
    context = rb_grn_object->context;

    if (context)
	grn_obj_close(context, rb_grn_fix_size_column->value);

    rb_grn_object_unbind(rb_grn_object);
}

static void
rb_grn_fix_size_column_free (void *object)
{
    RbGrnFixSizeColumn *rb_grn_fix_size_column = object;

    rb_grn_fix_size_column_unbind(rb_grn_fix_size_column);
    xfree(rb_grn_fix_size_column);
}

VALUE
rb_grn_fix_size_column_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_fix_size_column_free, NULL);
}

void
rb_grn_fix_size_column_bind (RbGrnFixSizeColumn *rb_grn_fix_size_column,
			     grn_ctx *context, grn_obj *column,
			     rb_grn_boolean owner)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_fix_size_column);
    rb_grn_object_bind(rb_grn_object, context, column, owner);
    rb_grn_object->unbind =
	RB_GRN_UNBIND_FUNCTION(rb_grn_fix_size_column_unbind);

    rb_grn_fix_size_column->value = grn_obj_open(context, GRN_BULK, 0,
						 rb_grn_object->range_id);
}

void
rb_grn_fix_size_column_assign (VALUE self, VALUE rb_context,
		      grn_ctx *context, grn_obj *column,
		      rb_grn_boolean owner)
{
    RbGrnFixSizeColumn *rb_grn_fix_size_column;

    rb_grn_fix_size_column = ALLOC(RbGrnFixSizeColumn);
    DATA_PTR(self) = rb_grn_fix_size_column;
    rb_grn_fix_size_column_bind(rb_grn_fix_size_column, context, column, owner);

    rb_iv_set(self, "context", rb_context);
}

void
rb_grn_fix_size_column_deconstruct (RbGrnFixSizeColumn *rb_grn_fix_size_column,
				    grn_obj **column,
				    grn_ctx **context,
				    grn_id *domain_id,
				    grn_obj **domain,
				    grn_obj **value,
				    grn_id *range_id,
				    grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_fix_size_column);
    rb_grn_object_deconstruct(rb_grn_object, column, context,
			      domain_id, domain,
			      range_id, range);

    if (value)
	*value = rb_grn_fix_size_column->value;
}

/*
 * call-seq:
 *   column[id] -> 値
 *
 * _column_の_id_に対応する値を返す。
 */
VALUE
rb_grn_fix_size_column_array_reference (VALUE self, VALUE rb_id)
{
    grn_id id;
    grn_ctx *context;
    grn_obj *fix_size_column;
    grn_obj *range;
    grn_obj *value;

    rb_grn_fix_size_column_deconstruct(SELF(self), &fix_size_column, &context,
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
 * _column_の_id_に対応する値を設定する。
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

    rb_grn_fix_size_column_deconstruct(SELF(self), &column, &context,
				       &domain_id, &domain,
				       &value, &range_id, &range);

    id = NUM2UINT(rb_id);

    if (RVAL2CBOOL(rb_obj_is_kind_of(rb_value, rb_cGrnRecord))) {
	VALUE rb_id, rb_table;
	grn_obj *table;

	if (!range)
	    rb_raise(rb_eArgError,
		     "%s isn't associated with any table: %s",
		     rb_grn_inspect(self), rb_grn_inspect(rb_value));

	rb_id = rb_funcall(rb_value, rb_intern("id"), 0);
	rb_table = rb_funcall(rb_value, rb_intern("table"), 0);
	table = RVAL2GRNTABLE(rb_table, &context);
	if (grn_obj_id(context, table) != range_id)
	    rb_raise(rb_eArgError,
		     "%s isn't associated with passed record's table: %s",
		     rb_grn_inspect(self),
		     rb_grn_inspect(rb_value));

	rb_value = rb_id;
    } else if (range) {
	switch (range->header.type) {
	  case GRN_TABLE_HASH_KEY:
	  case GRN_TABLE_PAT_KEY:
	    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_value, rb_cFixnum))) {
		VALUE rb_range;
		grn_id id;

		rb_range = GRNOBJECT2RVAL(Qnil, context, range, RB_GRN_FALSE);
		id = rb_grn_table_key_support_get(rb_range, rb_value);
		rb_value = UINT2NUM(id);
	    }
	    break;
	  default:
	    break;
	}
    }

    GRN_BULK_REWIND(value);
    RVAL2GRNBULK(rb_value, context, value);

    rc = grn_obj_set_value(context, column, id, value, GRN_OBJ_SET);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   column.increment!(id, delta=nil)
 *
 * _column_の_id_に対応する値を_delta_だけ増加する。_delta_
 * が+nil+の場合は1増加する。
 */
static VALUE
rb_grn_fix_size_column_increment (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj *value;
    grn_rc rc;
    grn_id id;
    VALUE rb_id, rb_delta;

    rb_scan_args(argc, argv, "11", &rb_id, &rb_delta);

    rb_grn_fix_size_column_deconstruct(SELF(self), &column, &context,
				       NULL, NULL,
				       &value, NULL, NULL);

    id = NUM2UINT(rb_id);
    if (NIL_P(rb_delta))
	rb_delta = INT2NUM(1);

    GRN_BULK_REWIND(value);
    RVAL2GRNBULK(rb_delta, context, value);

    rc = grn_obj_set_value(context, column, id, value, GRN_OBJ_INCR);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

void
rb_grn_init_fix_size_column (VALUE mGrn)
{
    rb_cGrnFixSizeColumn =
	rb_define_class_under(mGrn, "FixSizeColumn", rb_cGrnColumn);
    rb_define_alloc_func(rb_cGrnFixSizeColumn, rb_grn_fix_size_column_alloc);

    rb_define_method(rb_cGrnFixSizeColumn, "[]",
		     rb_grn_fix_size_column_array_reference, 1);
    rb_define_method(rb_cGrnFixSizeColumn, "[]=",
		     rb_grn_fix_size_column_array_set, 2);

    rb_define_method(rb_cGrnFixSizeColumn, "increment!",
		     rb_grn_fix_size_column_increment, -1);
}
