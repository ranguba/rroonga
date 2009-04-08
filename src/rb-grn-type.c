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

#define SELF(object) (RVAL2GRNTYPE(object))

VALUE rb_cGrnType;

grn_obj *
rb_grn_type_from_ruby_object (VALUE object)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnType))) {
	rb_raise(rb_eTypeError, "not a groonga type");
    }

    return RVAL2GRNOBJECT(object, NULL);
}

VALUE
rb_grn_type_to_ruby_object (grn_ctx *context, grn_obj *type)
{
    return GRNOBJECT2RVAL(rb_cGrnType, context, type);
}

static VALUE
rb_grn_type_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *type;
    const char *name = NULL;
    unsigned name_size, size = sizeof(grn_id);
    grn_obj_flags flags = 0;
    VALUE rb_name, options, rb_context, rb_key_type, rb_size;

    rb_scan_args(argc, argv, "11", &rb_name, &options);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"type", &rb_key_type,
			"size", &rb_size,
			NULL);

    name = StringValuePtr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    context = rb_grn_context_ensure(rb_context);

    if (NIL_P(rb_key_type)) {
        flags = GRN_OBJ_KEY_VAR_SIZE;
    } else if (rb_grn_equal_option(rb_key_type, "integer") ||
               rb_grn_equal_option(rb_key_type, "int")) {
	flags = GRN_OBJ_KEY_INT;
        size = sizeof(int);
    } else if (rb_grn_equal_option(rb_key_type, "uint")) {
	flags = GRN_OBJ_KEY_UINT;
        size = sizeof(unsigned int);
    } else if (rb_grn_equal_option(rb_key_type, "float")) {
	flags = GRN_OBJ_KEY_FLOAT;
        size = sizeof(double);
    }

    if (NIL_P(rb_size)) {
        if (flags == GRN_OBJ_KEY_VAR_SIZE)
            rb_raise(rb_eArgError, "size is missing: %s",
                     rb_grn_inspect(options));
    } else {
        size = NUM2UINT(rb_size);
    }

    type = grn_type_create(context, name, name_size, flags, size);
    rb_grn_object_initialize(self, context, type);
    rb_grn_context_check(context);

    return Qnil;
}

void
rb_grn_init_type (VALUE mGrn)
{
    rb_cGrnType = rb_define_class_under(mGrn, "Type", rb_cGrnObject);

    rb_define_method(rb_cGrnType, "initialize", rb_grn_type_initialize, -1);

    rb_define_const(rb_cGrnType, "INT", INT2NUM(GRN_DB_INT));
    rb_define_const(rb_cGrnType, "UINT", INT2NUM(GRN_DB_UINT));
    rb_define_const(rb_cGrnType, "INT64", INT2NUM(GRN_DB_INT64));
    rb_define_const(rb_cGrnType, "FLOAT", INT2NUM(GRN_DB_FLOAT));
    rb_define_const(rb_cGrnType, "TIME", INT2NUM(GRN_DB_TIME));
    rb_define_const(rb_cGrnType, "SHORT_TEXT", INT2NUM(GRN_DB_SHORTTEXT));
    rb_define_const(rb_cGrnType, "TEXT", INT2NUM(GRN_DB_TEXT));
    rb_define_const(rb_cGrnType, "LONG_TEXT", INT2NUM(GRN_DB_LONGTEXT));
}
