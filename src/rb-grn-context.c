/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-groonga-private.h"

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
rb_grn_context_free (void *context)
{
    grn_ctx_fin(context);
    free(context);
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
rb_grn_context_initialize (VALUE argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    int flags = 0;
    grn_encoding encoding = GRN_ENC_DEFAULT;
    grn_rc rc;
    VALUE options, default_options;

    rb_scan_args(argc, argv, "01", &options);
    default_options = rb_grn_context_s_get_default_options(rb_obj_class(self));
    if (NIL_P(default_options))
	default_options = rb_hash_new();

    if (NIL_P(options))
	options = rb_hash_new();
    options = rb_funcall(default_options, rb_intern("merge"), 1, options);

    if (RVAL2CBOOL(rb_hash_aref(options, RB_GRN_INTERN("use_ql"))))
	flags |= GRN_CTX_USE_QL;
    if (RVAL2CBOOL(rb_hash_aref(options, RB_GRN_INTERN("batch_mode"))))
	flags |= GRN_CTX_BATCH_MODE;
    encoding =
	RVAL2GRNENCODING(rb_hash_aref(options, RB_GRN_INTERN("encoding")));

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

void
rb_grn_init_context (VALUE mGroonga)
{
    cGrnContext = rb_define_class_under(mGroonga, "Context", rb_cObject);
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
}
