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

#define SELF(object) ((RbGrnExpression *)DATA_PTR(object))

VALUE rb_cGrnExpression;

void
rb_grn_expression_unbind (RbGrnExpression *rb_grn_expression)
{
    grn_ctx *context;
    grn_obj *value;

    context = RB_GRN_OBJECT(rb_grn_expression)->context;
    value = rb_grn_expression->value;
    if (context && value) {
	grn_obj_close(context, value);
	rb_grn_expression->value = NULL;
    }

    rb_grn_object_unbind(RB_GRN_OBJECT(rb_grn_expression));
}

static void
rb_grn_expression_free (void *object)
{
    RbGrnExpression *rb_grn_expression = object;

    RB_GRN_OBJECT(rb_grn_expression)->unbind(rb_grn_expression);
    xfree(rb_grn_expression);
}

VALUE
rb_grn_expression_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_expression_free, NULL);
}

void
rb_grn_expression_bind (RbGrnExpression *rb_grn_expression,
                        grn_ctx *context, grn_obj *expression,
                        rb_grn_boolean owner)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_expression);
    rb_grn_object_bind(rb_grn_object, context, expression, owner);
    rb_grn_object->unbind = RB_GRN_UNBIND_FUNCTION(rb_grn_expression_unbind);

    rb_grn_expression->value = grn_obj_open(context, GRN_BULK, 0,
                                            rb_grn_object->range_id);
}

void
rb_grn_expression_assign (VALUE self, VALUE rb_context,
                          grn_ctx *context, grn_obj *expression,
                          rb_grn_boolean owner)
{
    RbGrnExpression *rb_grn_expression;

    rb_grn_expression = ALLOC(RbGrnExpression);
    DATA_PTR(self) = rb_grn_expression;
    rb_grn_expression_bind(rb_grn_expression, context, expression, owner);

    rb_iv_set(self, "context", rb_context);
}

void
rb_grn_expression_deconstruct (RbGrnExpression *rb_grn_expression,
                               grn_obj **expression,
                               grn_ctx **context,
                               grn_id *domain_id,
                               grn_obj **domain,
			       grn_obj **value,
                               grn_id *range_id,
                               grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_expression);
    rb_grn_object_deconstruct(rb_grn_object, expression, context,
			      domain_id, domain,
			      range_id, range);

    if (value)
	*value = rb_grn_expression->value;
}

static VALUE
rb_grn_expression_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression;
    VALUE options, rb_context, rb_name;
    char *name = NULL;
    unsigned name_size = 0;

    rb_scan_args(argc, argv, "01", &options);
    rb_grn_scan_options(options,
                        "context", &rb_context,
                        "name", &rb_name,
                        NULL);

    context = rb_grn_context_ensure(&rb_context);

    if (!NIL_P(rb_name)) {
	name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
    }

    expression = grn_expr_create(context, name, name_size);
    rb_grn_expression_assign(self, rb_context, context, expression, RB_GRN_TRUE);
    rb_grn_context_check(context, self);

    return Qnil;
}

static VALUE
rb_grn_expression_define_variable (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression, *variable;
    char *name = NULL;
    unsigned name_size = 0;
    VALUE rb_name, rb_variable;

    rb_scan_args(argc, argv, "01", &rb_name);

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    if (!NIL_P(rb_name)) {
	name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
    }

    variable = grn_expr_add_var(context, expression, name, name_size);
    rb_variable = GRNVARIABLE2RVAL(context, variable);

    return rb_variable;
}

static VALUE
rb_grn_expression_get_value (VALUE self, VALUE rb_offset)
{
    grn_ctx *context = NULL;
    grn_obj *value, *expression;
    int offset;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    offset = NUM2INT(rb_offset);
    value = grn_expr_get_value(context, expression, offset);
    return GRNBULK2RVAL(context, value, self);
}

static VALUE
rb_grn_expression_append_object (VALUE self, VALUE rb_object)
{
    grn_ctx *context = NULL;
    grn_obj *expression, *object;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    object = RVAL2GRNOBJECT(rb_object, &context);
    grn_expr_append_const(context, expression, object);
    rb_grn_context_check(context, self);
    return self;
}

static VALUE
rb_grn_expression_append_constant (VALUE self, VALUE rb_constant)
{
    grn_ctx *context = NULL;
    grn_obj *expression, *constant = NULL;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL, NULL,
				  NULL, NULL);

    RVAL2GRNOBJ(rb_constant, context, &constant);
    grn_expr_append_const(context, expression, constant);
    grn_obj_close(context, constant);
    rb_grn_context_check(context, self);
    return self;
}

static VALUE
rb_grn_expression_append_operation (VALUE self, VALUE rb_operation,
				    VALUE rb_n_arguments)
{
    grn_ctx *context = NULL;
    grn_obj *expression;
    grn_op operation;
    int n_arguments = 0;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    operation = NUM2INT(rb_operation);
    n_arguments = NUM2INT(rb_n_arguments);
    grn_expr_append_op(context, expression, operation, n_arguments);
    rb_grn_context_check(context, self);
    return Qnil;
}

static VALUE
rb_grn_expression_execute (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression, *result;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    result = grn_expr_exec(context, expression);
    return GRNOBJ2RVAL(Qnil, context, result, self);
}

static VALUE
rb_grn_expression_compile (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression;
    grn_rc rc;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    rc = grn_expr_compile(context, expression);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

static VALUE
rb_grn_expression_array_reference (VALUE self, VALUE rb_name_or_offset)
{
    grn_ctx *context = NULL;
    grn_obj *expression, *variable, *value;
    char *name = NULL;
    unsigned name_size = 0;
    int offset;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    switch (TYPE(rb_name_or_offset)) {
    case T_STRING:
        name = RSTRING_PTR(rb_name_or_offset);
        name_size = RSTRING_LEN(rb_name_or_offset);
        variable = grn_expr_get_var(context, expression, name, name_size);
        return GRNBULK2RVAL(context, variable, self);
        break;
    case T_FIXNUM:
        offset = NUM2INT(rb_name_or_offset);
        value = grn_expr_get_var_by_offset(context, expression, offset);
        return GRNBULK2RVAL(context, value, self);
        break;
    default:
        rb_raise(rb_eArgError, "xxx");
        break;
    }

    return Qnil;
}

void
rb_grn_init_expression (VALUE mGrn)
{
    rb_cGrnExpression = rb_define_class_under(mGrn, "Expression", rb_cGrnObject);
    rb_define_alloc_func(rb_cGrnExpression, rb_grn_expression_alloc);

    rb_define_method(rb_cGrnExpression, "initialize",
                     rb_grn_expression_initialize, -1);

    rb_define_method(rb_cGrnExpression, "define_variable",
                     rb_grn_expression_define_variable, -1);
    rb_define_method(rb_cGrnExpression, "append_object",
                     rb_grn_expression_append_object, 1);
    rb_define_method(rb_cGrnExpression, "append_constant",
                     rb_grn_expression_append_constant, 1);
    rb_define_method(rb_cGrnExpression, "append_operation",
                     rb_grn_expression_append_operation, 2);

    rb_define_method(rb_cGrnExpression, "execute",
                     rb_grn_expression_execute, 0);
    rb_define_method(rb_cGrnExpression, "compile",
                     rb_grn_expression_compile, 0);

    rb_define_method(rb_cGrnExpression, "value",
                     rb_grn_expression_get_value, 1);

    rb_define_method(rb_cGrnExpression, "[]",
                     rb_grn_expression_array_reference, 1);
}
