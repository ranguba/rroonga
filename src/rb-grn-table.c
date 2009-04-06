/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-grn.h"

#define DEFAULT_VALUE_SIZE 512

#define SELF(object) (RVAL2GRNTABLE(object))

VALUE rb_cGrnTable;

grn_obj *
rb_grn_table_from_ruby_object (VALUE object)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnTable))) {
	rb_raise(rb_eTypeError, "not a groonga table");
    }

    return RVAL2GRNOBJECT(object, NULL);
}

VALUE
rb_grn_table_to_ruby_object (grn_ctx *context, grn_obj *table)
{
    return GRNOBJECT2RVAL(rb_cGrnTable, context, table);
}

static VALUE
rb_grn_table_s_create (VALUE argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context;
    grn_obj *key_type, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0, value_size;
    grn_obj_flags flags = 0;
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
    } else if (rb_grn_equal_option(rb_key_store, "pat")) {
	flags |= GRN_OBJ_TABLE_PAT_KEY;
    } else if (rb_grn_equal_option(rb_key_store, "hash")) {
	flags |= GRN_OBJ_TABLE_HASH_KEY;
    } else {
	rb_raise(rb_eArgError, ":key_store should be one of [:pat, :hash]: %s",
		 rb_grn_inspect(rb_key_store));
    }

    if (RVAL2CBOOL(rb_key_normalize))
	flags |= GRN_OBJ_KEY_NORMALIZE;

    if (RVAL2CBOOL(rb_key_with_sis))
	flags |= GRN_OBJ_KEY_WITH_SIS;

    key_type = RVAL2GRNOBJECT(rb_key_type, context);

    if (NIL_P(rb_value_size)) {
	value_size = DEFAULT_VALUE_SIZE;
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
    char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    VALUE rb_path, options, rb_context, rb_name;

    rb_scan_args(argc, argv, "01", &options);

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

    if (!NIL_P(rb_path))
	path = StringValueCStr(rb_path);

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

static VALUE
rb_grn_table_define_column (VALUE argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *value_type, *column;
    char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_obj_flags flags = 0;
    rb_grn_boolean use_default_type = RB_GRN_FALSE;
    VALUE rb_name, rb_value_type;
    VALUE options, rb_context, rb_path, rb_persistent, rb_type;
    VALUE rb_compress, rb_with_section, rb_with_weight, rb_with_position;

    rb_scan_args(argc, argv, "21", &rb_name, &rb_value_type, &options);

    name = StringValuePtr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"path", &rb_path,
			"persistent", &rb_persistent,
			"type", &rb_type,
			"compress", &rb_compress,
			"with_section", &rb_with_section,
			"with_weight", &rb_with_weight,
			"with_position", &rb_with_position,
			NULL);

    context = rb_grn_object_ensure_context(self, rb_context);

    value_type = RVAL2GRNOBJECT(rb_value_type, context);

    if (!NIL_P(rb_path)) {
	path = StringValueCStr(rb_path);
	flags |= GRN_OBJ_PERSISTENT;
    }

    if (RVAL2CBOOL(rb_persistent))
	flags |= GRN_OBJ_PERSISTENT;

    if (NIL_P(rb_type)) {
	use_default_type = RB_GRN_TRUE;
    } else if (rb_grn_equal_option(rb_type, "index")) {
	flags |= GRN_OBJ_COLUMN_INDEX;
    } else if (rb_grn_equal_option(rb_type, "scalar")) {
	flags |= GRN_OBJ_COLUMN_SCALAR;
    } else if (rb_grn_equal_option(rb_type, "vector")) {
	flags |= GRN_OBJ_COLUMN_VECTOR;
    } else {
	rb_raise(rb_eArgError,
		 "invalid column type: %s: "
		 "available types: [:index, :scalar, :vector, nil]",
		 rb_grn_inspect(rb_type));
    }

    if (NIL_P(rb_compress)) {
    } else if (rb_grn_equal_option(rb_compress, "zlib")) {
	flags |= GRN_OBJ_COMPRESS_ZLIB;
    } else if (rb_grn_equal_option(rb_compress, "lzo")) {
	flags |= GRN_OBJ_COMPRESS_LZO;
    } else {
	rb_raise(rb_eArgError,
		 "invalid compress type: %s: "
		 "available types: [:zlib, :lzo, nil]",
		 rb_grn_inspect(rb_compress));
    }

    if (RVAL2CBOOL(rb_with_section)) {
	if (use_default_type)
	    flags |= GRN_OBJ_COLUMN_INDEX;
	if (flags & GRN_OBJ_COLUMN_INDEX)
	    flags |= GRN_OBJ_WITH_SECTION;
	else
	    rb_raise(rb_eArgError,
		     "{:with_section => true} requires "
		     "{:type => :index} option.");
    }

    if (RVAL2CBOOL(rb_with_weight)) {
	if (use_default_type)
	    flags |= GRN_OBJ_COLUMN_INDEX;
	if (flags & GRN_OBJ_COLUMN_INDEX)
	    flags |= GRN_OBJ_WITH_WEIGHT;
	else
	    rb_raise(rb_eArgError,
		     "{:with_weight => true} requires "
		     "{:type => :index} option.");
    }

    if (RVAL2CBOOL(rb_with_position)) {
	if (use_default_type)
	    flags |= GRN_OBJ_COLUMN_INDEX;
	if (flags & GRN_OBJ_COLUMN_INDEX)
	    flags |= GRN_OBJ_WITH_POSITION;
	else
	    rb_raise(rb_eArgError,
		     "{:with_position => true} requires "
		     "{:type => :index} option.");
    }

    column = grn_column_create(context, SELF(self), name, name_size,
			       path, flags, value_type);
    rb_grn_context_check(context);

    return GRNCOLUMN2RVAL(context, column);
}

static VALUE
rb_grn_table_get_column (VALUE self, VALUE rb_name)
{
    grn_ctx *context;
    grn_obj *column;
    char *name = NULL;
    unsigned name_size = 0;

    name = StringValuePtr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    context = rb_grn_object_ensure_context(self, Qnil);

    column = grn_table_column(context, SELF(self), name, name_size);
    rb_grn_context_check(context);

    return GRNCOLUMN2RVAL(context, column);
}

void
rb_grn_init_table (VALUE mGrn)
{
    rb_cGrnTable = rb_define_class_under(mGrn, "Table", rb_cGrnObject);

    rb_define_singleton_method(rb_cGrnTable, "create",
			       rb_grn_table_s_create, -1);
    rb_define_singleton_method(rb_cGrnTable, "open",
			       rb_grn_table_s_open, -1);

    rb_define_method(rb_cGrnTable, "initialize", rb_grn_table_initialize, -1);

    rb_define_method(rb_cGrnTable, "define_column",
		     rb_grn_table_define_column, -1);
    rb_define_method(rb_cGrnTable, "column",
		     rb_grn_table_get_column, 1);
}
