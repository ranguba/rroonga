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

#define SELF(object) (RVAL2GRNCONTEXT(object))

static VALUE cGrnContext;

grn_ctx *
rb_grn_context_from_ruby_object (VALUE object)
{
    grn_ctx *context;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, cGrnContext))) {
	rb_raise(rb_eTypeError, "not a groonga context");
    }

    Data_Get_Struct(object, grn_ctx, context);
    if (!context)
	rb_raise(rb_eGrnError, "groonga context is NULL");
    return context;
}

static void
rb_grn_context_free (void *pointer)
{
    grn_ctx *context = pointer;

    if (context->stat != GRN_CTX_FIN)
	grn_ctx_fin(context);
    xfree(context);
}

static VALUE
rb_grn_context_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_context_free, NULL);
}

VALUE
rb_grn_context_to_exception (grn_ctx *context, VALUE related_object)
{
    VALUE exception, exception_class;
    const char *message;
    grn_obj bulk;

    if (context->rc == GRN_SUCCESS)
	return Qnil;

    exception_class = rb_grn_rc_to_exception(context->rc);
    message = rb_grn_rc_to_message(context->rc);

    grn_bulk_init(context, &bulk, 0);
    GRN_BULK_PUTS(context, &bulk, message);
    GRN_BULK_PUTS(context, &bulk, ": ");
    GRN_BULK_PUTS(context, &bulk, context->errbuf);
    if (!NIL_P(related_object)) {
	GRN_BULK_PUTS(context, &bulk, ": ");
	GRN_BULK_PUTS(context, &bulk, rb_grn_inspect(related_object));
    }
    GRN_BULK_PUTS(context, &bulk, "\n");
    GRN_BULK_PUTS(context, &bulk, context->errfile);
    GRN_BULK_PUTS(context, &bulk, ":");
    grn_bulk_itoa(context, &bulk, context->errline);
    GRN_BULK_PUTS(context, &bulk, ": ");
    GRN_BULK_PUTS(context, &bulk, context->errfunc);
    GRN_BULK_PUTS(context, &bulk, "()");
    exception = rb_funcall(exception_class, rb_intern("new"), 1,
			   rb_str_new(GRN_BULK_HEAD(&bulk),
				      GRN_BULK_VSIZE(&bulk)));
    grn_obj_close(context, &bulk);

    return exception;
}

void
rb_grn_context_check (grn_ctx *context, VALUE related_object)
{
    VALUE exception;

    exception = rb_grn_context_to_exception(context, related_object);
    if (NIL_P(exception))
	return;

    rb_exc_raise(exception);
}

grn_ctx *
rb_grn_context_ensure (VALUE context)
{
    if (NIL_P(context))
	context = rb_grn_context_get_default();
    return SELF(context);
}

static VALUE
rb_grn_context_s_get_default (VALUE self)
{
    VALUE context;

    context = rb_cv_get(self, "@@default");
    if (NIL_P(context)) {
	context = rb_funcall(cGrnContext, rb_intern("new"), 0);
	rb_cv_set(self, "@@default", context);
    }
    return context;
}

VALUE
rb_grn_context_get_default (void)
{
    return rb_grn_context_s_get_default(cGrnContext);
}

static VALUE
rb_grn_context_s_set_default (VALUE self, VALUE context)
{
    rb_cv_set(self, "@@default", context);
    return Qnil;
}

static VALUE
rb_grn_context_s_get_default_options (VALUE self)
{
    return rb_cv_get(self, "@@default_options");
}

static VALUE
rb_grn_context_s_set_default_options (VALUE self, VALUE options)
{
    rb_cv_set(self, "@@default_options", options);
    return Qnil;
}

static VALUE
rb_grn_context_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    int flags = 0;
    grn_rc rc;
    VALUE options, default_options;
    VALUE use_ql, batch_mode, rb_encoding;

    rb_scan_args(argc, argv, "01", &options);
    default_options = rb_grn_context_s_get_default_options(rb_obj_class(self));
    if (NIL_P(default_options))
	default_options = rb_hash_new();

    if (NIL_P(options))
	options = rb_hash_new();
    options = rb_funcall(default_options, rb_intern("merge"), 1, options);

    rb_grn_scan_options(options,
			"use_ql", &use_ql,
			"batch_mode", &batch_mode,
			"encoding", &rb_encoding,
			NULL);

    if (RVAL2CBOOL(use_ql))
	flags |= GRN_CTX_USE_QL;
    if (RVAL2CBOOL(batch_mode))
	flags |= GRN_CTX_BATCH_MODE;

    context = ALLOC(grn_ctx);
    DATA_PTR(self) = context;
    rc = grn_ctx_init(context, flags);
    rb_grn_context_check(context, self);

    if (!NIL_P(rb_encoding)) {
	grn_encoding encoding;

	encoding = RVAL2GRNENCODING(rb_encoding, NULL);
	GRN_CTX_SET_ENCODING(context, encoding);
    }

    return Qnil;
}

static VALUE
rb_grn_context_inspect (VALUE self)
{
    VALUE inspected;
    grn_ctx *context;
    grn_obj *database;

    context = SELF(self);

    inspected = rb_str_new2("#<");
    rb_str_concat(inspected, rb_inspect(rb_obj_class(self)));
    rb_str_cat2(inspected, " ");

    rb_str_cat2(inspected, "use_ql: <");
    if (context->flags & GRN_CTX_USE_QL)
	rb_str_cat2(inspected, "true");
    else
	rb_str_cat2(inspected, "false");
    rb_str_cat2(inspected, ">, ");

    rb_str_cat2(inspected, "batch_mode: <");
    if (context->flags & GRN_CTX_BATCH_MODE)
	rb_str_cat2(inspected, "true");
    else
	rb_str_cat2(inspected, "false");
    rb_str_cat2(inspected, ">, ");

    rb_str_cat2(inspected, "encoding: <");
    rb_str_concat(inspected, rb_inspect(GRNENCODING2RVAL(context->encoding)));
    rb_str_cat2(inspected, ">, ");

    rb_str_cat2(inspected, "database: <");
    database = grn_ctx_db(context);
    rb_str_concat(inspected, rb_inspect(GRNDB2RVAL(context, database)));
    rb_str_cat2(inspected, ">");

    rb_str_cat2(inspected, ">");
    return inspected;
}

static VALUE
rb_grn_context_use_ql_p (VALUE self)
{
    return CBOOL2RVAL(SELF(self)->flags & GRN_CTX_USE_QL);
}

static VALUE
rb_grn_context_batch_mode_p (VALUE self)
{
    return CBOOL2RVAL(SELF(self)->flags & GRN_CTX_BATCH_MODE);
}

static VALUE
rb_grn_context_get_encoding (VALUE self)
{
    return GRNENCODING2RVAL(GRN_CTX_GET_ENCODING(SELF(self)));
}

static VALUE
rb_grn_context_set_encoding (VALUE self, VALUE rb_encoding)
{
    grn_ctx *context;
    grn_encoding encoding;

    context = SELF(self);
    encoding = RVAL2GRNENCODING(rb_encoding, NULL);
    GRN_CTX_SET_ENCODING(context, encoding);

    return rb_encoding;
}

static VALUE
rb_grn_context_get_database (VALUE self)
{
    grn_ctx *context;

    context = SELF(self);
    return GRNDB2RVAL(context, grn_ctx_db(context));
}

static VALUE
rb_grn_context_array_reference (VALUE self, VALUE name_or_id)
{
    grn_ctx *context;
    grn_obj *object;

    context = SELF(self);
    if (RVAL2CBOOL(rb_obj_is_kind_of(name_or_id, rb_cString))) {
	const char *name;
	unsigned name_size;

	name = StringValuePtr(name_or_id);
	name_size = RSTRING_LEN(name_or_id);
	object = grn_ctx_lookup(context, name, name_size);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(name_or_id, rb_cInteger))) {
	unsigned id;
	id = NUM2UINT(name_or_id);
	object = grn_ctx_get(context, id);
    } else {
	rb_raise(rb_eArgError, "should be string or unsigned integer: %s",
		 rb_grn_inspect(name_or_id));
    }

    return GRNOBJECT2RVAL(Qnil, context, object);
}

void
rb_grn_init_context (VALUE mGrn)
{
    cGrnContext = rb_define_class_under(mGrn, "Context", rb_cObject);
    rb_define_alloc_func(cGrnContext, rb_grn_context_alloc);

    rb_cv_set(cGrnContext, "@@default", Qnil);
    rb_cv_set(cGrnContext, "@@default_options", Qnil);

    rb_define_singleton_method(cGrnContext, "default",
			       rb_grn_context_s_get_default, 0);
    rb_define_singleton_method(cGrnContext, "default=",
			       rb_grn_context_s_set_default, 1);
    rb_define_singleton_method(cGrnContext, "default_options",
			       rb_grn_context_s_get_default_options, 0);
    rb_define_singleton_method(cGrnContext, "default_options=",
			       rb_grn_context_s_set_default_options, 1);

    rb_define_method(cGrnContext, "initialize", rb_grn_context_initialize, -1);

    rb_define_method(cGrnContext, "inspect", rb_grn_context_inspect, 0);

    rb_define_method(cGrnContext, "use_ql?", rb_grn_context_use_ql_p, 0);
    rb_define_method(cGrnContext, "batch_mode?", rb_grn_context_batch_mode_p, 0);
    rb_define_method(cGrnContext, "encoding", rb_grn_context_get_encoding, 0);
    rb_define_method(cGrnContext, "encoding=", rb_grn_context_set_encoding, 1);

    rb_define_method(cGrnContext, "database", rb_grn_context_get_database, 0);

    rb_define_method(cGrnContext, "[]", rb_grn_context_array_reference, 1);
}
