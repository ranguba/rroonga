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

#define SELF(object) ((RbGrnVariable *)DATA_PTR(object))

VALUE rb_cGrnVariable;

/*
 * Document-class: Groonga::Variable < Groonga::Object
 *
 * Groonga::Expressionで使われる変数。
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
 * call-seq:
 *   variable.value -> Groonga::Object
 *
 * 変数の値を返す。
 */
static VALUE
rb_grn_variable_get_value (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *variable;

    rb_grn_variable_deconstruct(SELF(self), &variable, &context,
                                NULL, NULL,
                                NULL, NULL);

    return GRNOBJ2RVAL(Qnil, context, variable, self);
}

/*
 * call-seq:
 *   variable.value=(value)
 *
 * 変数の値を _value_ に設定する。
 */
static VALUE
rb_grn_variable_set_value (VALUE self, VALUE value)
{
    grn_ctx *context = NULL;
    grn_obj *variable;

    rb_grn_variable_deconstruct(SELF(self), &variable, &context,
                                NULL, NULL,
                                NULL, NULL);
    RVAL2GRNOBJ(value, context, &variable);
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
