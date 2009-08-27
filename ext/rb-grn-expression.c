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
rb_grn_expression_finalizer (grn_ctx *context, grn_obj *object,
			     RbGrnExpression *rb_grn_expression)
{
    if (context && rb_grn_expression->value)
	grn_obj_close(context, rb_grn_expression->value);

    rb_grn_expression->value = NULL;
}

void
rb_grn_expression_bind (RbGrnExpression *rb_grn_expression,
                        grn_ctx *context, grn_obj *expression)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_expression);

    rb_grn_expression->value = grn_obj_open(context, GRN_BULK, 0,
                                            rb_grn_object->range_id);
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
    rb_grn_object_assign(Qnil, self, rb_context, context, expression);
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
    VALUE options, rb_name, rb_domain, rb_variable;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    rb_grn_scan_options(options,
			"name", &rb_name,
			"domain", &rb_domain,
			NULL);

    if (!NIL_P(rb_name)) {
	name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
    }

    variable = grn_expr_add_var(context, expression, name, name_size);
    rb_variable = GRNVARIABLE2RVAL(context, variable);

    if (RVAL2CBOOL(rb_obj_is_kind_of(rb_domain, rb_cGrnTable))) {
	grn_id domain_id;
	domain_id = NUM2UINT(rb_funcall(rb_domain, rb_intern("id"), 0));
	GRN_RECORD_INIT(variable, 0, domain_id);
    }

    return rb_variable;
}

static VALUE
rb_grn_expression_append_object (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_object, rb_operation, rb_n_arguments;
    grn_ctx *context = NULL;
    grn_obj *expression, *object;
    grn_operator operation = GRN_OP_PUSH;
    int n_arguments = 1;

    rb_scan_args(argc, argv, "12", &rb_object, &rb_operation, &rb_n_arguments);
    if (!NIL_P(rb_operation))
        operation = NUM2INT(rb_operation);
    if (!NIL_P(rb_n_arguments))
        n_arguments = NUM2INT(rb_n_arguments);

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    object = RVAL2GRNOBJECT(rb_object, &context);
    grn_expr_append_obj(context, expression, object,
                        operation, n_arguments);
    rb_grn_context_check(context, self);
    return self;
}

static VALUE
rb_grn_expression_append_constant (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_constant, rb_operator, rb_n_arguments;
    grn_ctx *context = NULL;
    grn_obj *expression, *constant = NULL;
    grn_operator operator = GRN_OP_PUSH;
    int n_arguments = 1;

    rb_scan_args(argc, argv, "12", &rb_constant, &rb_operator, &rb_n_arguments);
    if (!NIL_P(rb_operator))
        operator = NUM2INT(rb_operator);
    if (!NIL_P(rb_n_arguments))
        n_arguments = NUM2INT(rb_n_arguments);

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL, NULL,
				  NULL, NULL);

    RVAL2GRNOBJ(rb_constant, context, &constant);
    grn_expr_append_const(context, expression, constant, operator, n_arguments);
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
    grn_operator operation;
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
rb_grn_expression_parse (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression, *default_column = NULL;
    grn_operator default_operator = GRN_OP_AND;
    grn_operator default_mode = GRN_OP_MATCH;
    grn_rc rc;
    char *query = NULL;
    unsigned query_size = 0;
    int parse_level = 0;
    VALUE options, rb_query, rb_default_column, rb_default_operator;
    VALUE rb_default_mode, rb_use_pragma;

    rb_scan_args(argc, argv, "11", &rb_query, &options);
    rb_grn_scan_options(options,
                        "default_column", &rb_default_column,
                        "default_operator", &rb_default_operator,
                        "default_mode", &rb_default_mode,
			"use_pragma", &rb_use_pragma,
                        NULL);

    query = StringValuePtr(rb_query);
    query_size = RSTRING_LEN(rb_query);

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    default_column = RVAL2GRNBULK(rb_default_column, context, default_column);
    if (!NIL_P(rb_default_mode))
	default_mode = RVAL2GRNOPERATOR(rb_default_mode);
    if (!NIL_P(rb_default_operator))
	default_operator = RVAL2GRNOPERATOR(rb_default_operator);
    if (RVAL2CBOOL(rb_use_pragma))
	parse_level = 2;
    rc = grn_expr_parse(context, expression, query, query_size,
			default_column, default_mode, default_operator,
			parse_level);
    if (rc != GRN_SUCCESS)
	rb_grn_context_check(context,
			     rb_ary_new3(2, self, rb_ary_new4(argc, argv)));

    return Qnil;
}

static VALUE
rb_grn_expression_execute (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression;
    grn_rc rc;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    rc = grn_expr_exec(context, expression, 0);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);
    
    return Qnil;
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

/* REMOVE ME */
grn_rc grn_expr_inspect(grn_ctx *ctx, grn_obj *buf, grn_obj *expr);

static VALUE
rb_grn_expression_inspect (VALUE self)
{
    grn_rc rc;
    grn_ctx *context = NULL;
    grn_obj inspected;
    grn_obj *expression;
    VALUE rb_inspected;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    GRN_TEXT_INIT(&inspected, 0);
    GRN_TEXT_PUTS(context, &inspected, "#<Groonga::Expression ");
    rc = grn_expr_inspect(context, &inspected, expression);
    GRN_TEXT_PUTS(context, &inspected, ">");
    rb_inspected = rb_str_new(GRN_TEXT_VALUE(&inspected),
			      GRN_TEXT_LEN(&inspected));
    GRN_OBJ_FIN(context, &inspected);

    return rb_inspected;
}

void
rb_grn_init_expression (VALUE mGrn)
{
    rb_cGrnExpression = rb_define_class_under(mGrn, "Expression", rb_cGrnObject);

    rb_define_method(rb_cGrnExpression, "initialize",
                     rb_grn_expression_initialize, -1);

    rb_define_method(rb_cGrnExpression, "define_variable",
                     rb_grn_expression_define_variable, -1);
    rb_define_method(rb_cGrnExpression, "append_object",
                     rb_grn_expression_append_object, -1);
    rb_define_method(rb_cGrnExpression, "append_constant",
                     rb_grn_expression_append_constant, -1);
    rb_define_method(rb_cGrnExpression, "append_operation",
                     rb_grn_expression_append_operation, 2);

    rb_define_method(rb_cGrnExpression, "parse",
                     rb_grn_expression_parse, -1);

    rb_define_method(rb_cGrnExpression, "execute",
                     rb_grn_expression_execute, 0);
    rb_define_method(rb_cGrnExpression, "compile",
                     rb_grn_expression_compile, 0);

    rb_define_method(rb_cGrnExpression, "[]",
                     rb_grn_expression_array_reference, 1);

    rb_define_method(rb_cGrnExpression, "inspect",
                     rb_grn_expression_inspect, 0);
}
