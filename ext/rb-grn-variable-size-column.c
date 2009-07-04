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

#define SELF(object) ((RbGrnVariableSizeColumn *)DATA_PTR(object))

VALUE rb_cGrnVariableSizeColumn;

/*
 * Document-class: Groonga::FixSizeColumn < Groonga::Column
 *
 * 可変長データ用のカラム。
 */

void
rb_grn_variable_size_column_unbind (RbGrnVariableSizeColumn *rb_column)
{
    RbGrnObject *rb_grn_object;
    grn_ctx *context;

    rb_grn_object = RB_GRN_OBJECT(rb_column);
    context = rb_grn_object->context;

    if (context)
	grn_obj_close(context, rb_column->value);

    rb_grn_object_unbind(rb_grn_object);
}

static void
rb_grn_variable_size_column_free (void *object)
{
    RbGrnVariableSizeColumn *rb_column = object;

    rb_grn_variable_size_column_unbind(rb_column);
    xfree(rb_column);
}

VALUE
rb_grn_variable_size_column_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_variable_size_column_free, NULL);
}

void
rb_grn_variable_size_column_bind (RbGrnVariableSizeColumn *rb_column,
                                  grn_ctx *context, grn_obj *column,
                                  rb_grn_boolean owner)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_column);
    rb_grn_object_bind(rb_grn_object, context, column, owner);
    rb_grn_object->unbind =
	RB_GRN_UNBIND_FUNCTION(rb_grn_variable_size_column_unbind);

    rb_column->value = grn_obj_open(context, GRN_BULK, 0,
                                    rb_grn_object->range_id);
}

void
rb_grn_variable_size_column_assign (VALUE self, VALUE rb_context,
                                    grn_ctx *context, grn_obj *column,
                                    rb_grn_boolean owner)
{
    RbGrnVariableSizeColumn *rb_column;

    rb_column = ALLOC(RbGrnVariableSizeColumn);
    DATA_PTR(self) = rb_column;
    rb_grn_variable_size_column_bind(rb_column, context, column, owner);

    rb_iv_set(self, "context", rb_context);
}

void
rb_grn_variable_size_column_deconstruct (RbGrnVariableSizeColumn *rb_column,
                                         grn_obj **column,
                                         grn_ctx **context,
                                         grn_id *domain_id,
                                         grn_obj **domain,
                                         grn_obj **value,
                                         grn_id *range_id,
                                         grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_column);
    rb_grn_object_deconstruct(rb_grn_object, column, context,
			      domain_id, domain,
			      range_id, range);

    if (value)
	*value = rb_column->value;
}

void
rb_grn_init_variable_size_column (VALUE mGrn)
{
    rb_cGrnVariableSizeColumn =
	rb_define_class_under(mGrn, "VariableSizeColumn", rb_cGrnColumn);
    rb_define_alloc_func(rb_cGrnVariableSizeColumn,
                         rb_grn_variable_size_column_alloc);
}
