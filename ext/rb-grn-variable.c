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

#define SELF(object) ((RbGrnVariable *)DATA_PTR(object))

VALUE rb_cGrnVariable;

grn_obj *
rb_grn_variable_from_ruby_object (VALUE variable, grn_ctx **context)
{
    return rb_grn_object_from_ruby_object(variable, context);
}

VALUE
rb_grn_variable_to_ruby_object (grn_ctx *context, grn_obj *variable)
{
    return rb_grn_object_to_ruby_object(rb_cGrnVariable, context, variable,
                                        RB_GRN_TRUE);
}

static void
rb_grn_variable_unbind (RbGrnVariable *rb_grn_variable)
{
    rb_grn_object_unbind(RB_GRN_OBJECT(rb_grn_variable));
}

static void
rb_grn_variable_free (void *object)
{
    RbGrnVariable *rb_grn_variable = object;

    RB_GRN_OBJECT(rb_grn_variable)->unbind(rb_grn_variable);
    xfree(rb_grn_variable);
}

static VALUE
rb_grn_variable_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_variable_free, NULL);
}

void
rb_grn_variable_bind (RbGrnVariable *rb_grn_variable,
                        grn_ctx *context, grn_obj *variable,
                        rb_grn_boolean owner)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_variable);
    rb_grn_object_bind(rb_grn_object, context, variable, owner);
    rb_grn_object->unbind = RB_GRN_UNBIND_FUNCTION(rb_grn_variable_unbind);
}

void
rb_grn_variable_assign (VALUE self, VALUE rb_context,
                          grn_ctx *context, grn_obj *variable,
                          rb_grn_boolean owner)
{
    RbGrnVariable *rb_grn_variable;

    rb_grn_variable = ALLOC(RbGrnVariable);
    DATA_PTR(self) = rb_grn_variable;
    rb_grn_variable_bind(rb_grn_variable, context, variable, owner);

    rb_iv_set(self, "context", rb_context);
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

static VALUE
rb_grn_variable_get_value (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *variable;

    rb_grn_variable_deconstruct(SELF(self), &variable, &context,
                                NULL, NULL,
                                NULL, NULL);

    return GRNBULK2RVAL(context, variable, self);
}

static VALUE
rb_grn_variable_set_value (VALUE self, VALUE value)
{
    grn_ctx *context = NULL;
    grn_obj *variable;

    rb_grn_variable_deconstruct(SELF(self), &variable, &context,
                                NULL, NULL,
                                NULL, NULL);

    /* value = grn_expr_set_value(context, variable); */
    return Qnil;
}

void
rb_grn_init_variable (VALUE mGrn)
{
    rb_cGrnVariable = rb_define_class_under(mGrn, "Variable", rb_cGrnObject);
    rb_define_alloc_func(rb_cGrnVariable, rb_grn_variable_alloc);

    rb_define_method(rb_cGrnVariable, "value",
                     rb_grn_variable_get_value, 0);
    rb_define_method(rb_cGrnVariable, "value=",
                     rb_grn_variable_set_value, 1);
}
