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

#include "rb-grn.h"

#define SELF(object) (RVAL2GRNTABLE(object))

static VALUE cGrnTable;

grn_obj *
rb_grn_table_from_ruby_object (VALUE object)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, cGrnTable))) {
	rb_raise(rb_eTypeError, "not a groonga table");
    }

    return RVAL2GRNOBJECT(object);
}

VALUE
rb_grn_table_to_ruby_object (grn_ctx *context, grn_obj *table)
{
    return GRNOBJECT2RVAL(cGrnTable, context, table);
}

static VALUE
rb_grn_table_s_create (VALUE argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context;
    grn_obj *key_type, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0, value_size;
    int flags = 0;
    grn_encoding encoding;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_key_store, rb_key_normalize, rb_key_with_sis, rb_key_type;
    VALUE rb_value_size, rb_encoding;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"name", &rb_name,
                        "path", &rb_path,
			"persistent", &rb_persistent,
			"key_store", &rb_key_store,
			"key_normalize", &rb_key_normalize,
			"key_with_sis", &rb_key_with_sis,
			"key_type", &rb_key_type,
			"value_size", &rb_value_size,
                        "encoding", &rb_encoding,
			NULL);

    context = rb_grn_context_ensure(rb_context);

    if (!NIL_P(rb_name)) {
        name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
    }

    if (!NIL_P(rb_path)) {
        path = StringValueCStr(rb_path);
	flags |= GRN_OBJ_PERSISTENT;
    }

    if (RVAL2CBOOL(rb_persistent))
	flags |= GRN_OBJ_PERSISTENT;

    if (NIL_P(rb_key_type)) {
    } else if (RVAL2CBOOL(rb_funcall(rb_key_store, rb_intern("=="),
				     1, RB_GRN_INTERN("pat")))) {
	flags |= GRN_OBJ_TABLE_PAT_KEY;
    } else if (RVAL2CBOOL(rb_funcall(rb_key_store, rb_intern("=="),
				     1, RB_GRN_INTERN("hash")))) {
	flags |= GRN_OBJ_TABLE_HASH_KEY;
    } else {
	rb_raise(rb_eArgError, ":key_store should be one of [:pat, :hash]: %s",
		 rb_grn_inspect(rb_key_store));
    }

    if (RVAL2CBOOL(rb_key_normalize))
	flags |= GRN_OBJ_KEY_NORMALIZE;

    if (RVAL2CBOOL(rb_key_with_sis))
	flags |= GRN_OBJ_KEY_WITH_SIS;

    key_type = RVAL2GRNOBJECT(rb_key_type);

    if (NIL_P(rb_value_size)) {
	value_size = 256;
    } else {
	value_size = NUM2UINT(rb_value_size);
    }

    encoding = RVAL2GRNENCODING(rb_encoding);

    table = grn_table_create(context, name, name_size, path,
			     flags, key_type, value_size,
			     encoding);
    rb_table = GRNOBJECT2RVAL(klass, context, table);
    rb_grn_context_check(context);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

static VALUE
rb_grn_table_initialize (VALUE argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *table;
    const char *path;
    char *name = NULL;
    unsigned name_size = 0;
    VALUE rb_path, options, rb_context, rb_name;

    rb_scan_args(argc, argv, "11", &rb_path, &options);

    path = StringValueCStr(rb_path);
    rb_grn_scan_options(options,
			"context", &rb_context,
			"name", &rb_name,
			"path", &rb_path,
			NULL);

    context = rb_grn_context_ensure(rb_context);
    if (!NIL_P(rb_name)) {
	name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
    }

    table = grn_table_open(context, name, name_size, path);
    rb_grn_object_initialize(self, context, table);
    rb_grn_context_check(context);

    return Qnil;
}

static VALUE
rb_grn_table_s_open (VALUE argc, VALUE *argv, VALUE klass)
{
    VALUE table;

    table = rb_grn_object_alloc(klass);
    rb_grn_table_initialize(argc, argv, table);
    if (rb_block_given_p())
        return rb_ensure(rb_yield, table, rb_grn_object_close, table);
    else
        return table;
}

void
rb_grn_init_table (VALUE mGrn)
{
    cGrnTable = rb_define_class_under(mGrn, "Table", rb_cGrnObject);

    rb_define_singleton_method(cGrnTable, "create",
			       rb_grn_table_s_create, -1);
    rb_define_singleton_method(cGrnTable, "open",
			       rb_grn_table_s_open, -1);

    rb_define_method(cGrnTable, "initialize", rb_grn_table_initialize, -1);
}
