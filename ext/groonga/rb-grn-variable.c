/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2021  Sutou Kouhei <kou@clear-code.com>

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

#define SELF(object) ((RbGrnVariable *)RTYPEDDATA_DATA(object))

VALUE rb_cGrnVariable;

/*
 * Document-class: Groonga::Variable < Groonga::Object
 *
 * {Groonga::Expression} で使われる変数。
 */

grn_obj *
rb_grn_variable_from_ruby_object (VALUE variable, grn_ctx **context)
{
    return rb_grn_object_from_ruby_object(variable, context);
}

VALUE
rb_grn_variable_to_ruby_object (grn_ctx *context, grn_obj *variable)
{
    return rb_grn_object_to_ruby_object(rb_cGrnVariable, context, variable,
                                        GRN_TRUE);
}

void
rb_grn_variable_deconstruct (RbGrnVariable *rb_grn_variable,
                             grn_obj **variable,
                             grn_ctx **context,
                             grn_id *domain_id,
                             grn_obj **domain,
                             grn_id *range_id,
                             grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_variable);
    rb_grn_object_deconstruct(rb_grn_object, variable, context,
                              domain_id, domain,
                              range_id, range);
}

/*
 * 変数の値を返す。
 *
 * @overload value
 *   @return [Groonga::Object]
 */
static VALUE
rb_grn_variable_get_value (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *variable, *value;

    rb_grn_variable_deconstruct(SELF(self), &variable, &context,
                                NULL, NULL,
                                NULL, NULL);

    if (variable->header.type == GRN_PTR) {
        value = GRN_PTR_VALUE(variable);
    } else {
        value = variable;
    }

    return GRNOBJ2RVAL(Qnil, context, value, self);
}

/*
 * 変数の値を _value_ に設定する。
 *
 * @overload value=(value)
 */
static VALUE
rb_grn_variable_set_value (VALUE self, VALUE rb_value)
{
    grn_ctx *context = NULL;
    grn_obj *variable;

    rb_grn_variable_deconstruct(SELF(self), &variable, &context,
                                NULL, NULL,
                                NULL, NULL);
    if (variable->header.type == GRN_PTR) {
        grn_obj *value = NULL;
        RVAL2GRNOBJ(rb_value, context, &value);
        GRN_PTR_SET(context, variable, value);
    } else {
        RVAL2GRNOBJ(rb_value, context, &variable);
    }

    return Qnil;
}

void
rb_grn_init_variable (VALUE mGrn)
{
    rb_cGrnVariable = rb_define_class_under(mGrn, "Variable", rb_cGrnObject);

    rb_define_method(rb_cGrnVariable, "value",
                     rb_grn_variable_get_value, 0);
    rb_define_method(rb_cGrnVariable, "value=",
                     rb_grn_variable_set_value, 1);
}
