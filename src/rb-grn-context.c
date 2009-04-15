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

void
rb_grn_context_check (grn_ctx *context)
{
    VALUE exception;
    const char *message;

    if (context->rc == GRN_SUCCESS)
	return;

    exception = rb_grn_rc_to_exception(context->rc);
    message = rb_grn_rc_to_message(context->rc);
    rb_raise(exception, "%s: %s\n%s:%d: %s()",
	     message, context->errbuf,
	     context->errfile, context->errline, context->errfunc);
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
    grn_encoding encoding = GRN_ENC_DEFAULT;
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
    encoding = RVAL2GRNENCODING(rb_encoding);

    context = ALLOC(grn_ctx);
    DATA_PTR(self) = context;
    rc = grn_ctx_init(context, flags, encoding);
    rb_grn_context_check(context);
    return Qnil;
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
    return GRNENCODING2RVAL(SELF(self)->encoding);
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

    rb_define_method(cGrnContext, "use_ql?", rb_grn_context_use_ql_p, 0);
    rb_define_method(cGrnContext, "batch_mode?", rb_grn_context_batch_mode_p, 0);
    rb_define_method(cGrnContext, "encoding", rb_grn_context_get_encoding, 0);

    rb_define_method(cGrnContext, "database", rb_grn_context_get_database, 0);

    rb_define_method(cGrnContext, "[]", rb_grn_context_array_reference, 1);
}
