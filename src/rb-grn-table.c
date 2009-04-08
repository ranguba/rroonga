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

#define DEFAULT_VALUE_SIZE 512

#define SELF(object) (RVAL2GRNTABLE(object))

VALUE rb_cGrnTable;
VALUE rb_cGrnHash;
VALUE rb_cGrnPatriciaTrie;
VALUE rb_cGrnArray;

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
rb_grn_table_s_create (int argc, VALUE *argv, VALUE klass,
		       grn_obj_flags key_store)
{
    grn_ctx *context;
    grn_obj *key_type = NULL, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0, value_size;
    grn_obj_flags flags = key_store;
    grn_encoding encoding;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_key_normalize, rb_key_with_sis, rb_key_type;
    VALUE rb_value_size, rb_encoding;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"name", &rb_name,
                        "path", &rb_path,
			"persistent", &rb_persistent,
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

    if (RVAL2CBOOL(rb_key_normalize))
	flags |= GRN_OBJ_KEY_NORMALIZE;

    if (RVAL2CBOOL(rb_key_with_sis))
	flags |= GRN_OBJ_KEY_WITH_SIS;

    if (NIL_P(rb_key_type)) {
	flags |= GRN_OBJ_KEY_VAR_SIZE;
    } else {
	key_type = RVAL2GRNOBJECT(rb_key_type, context);
    }

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
rb_grn_hash_s_create (int argc, VALUE *argv, VALUE self)
{
    return rb_grn_table_s_create(argc, argv, self, GRN_TABLE_HASH_KEY);
}

static VALUE
rb_grn_patricia_trie_s_create (int argc, VALUE *argv, VALUE self)
{
    return rb_grn_table_s_create(argc, argv, self, GRN_TABLE_PAT_KEY);
}

static VALUE
rb_grn_array_s_create (int argc, VALUE *argv, VALUE self)
{
    return rb_grn_table_s_create(argc, argv, self, GRN_TABLE_NO_KEY);
}

static grn_obj *
rb_grn_table_open (int argc, VALUE *argv, grn_ctx **context)
{
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

    *context = rb_grn_context_ensure(rb_context);

    if (!NIL_P(rb_name)) {
	name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
    }

    if (!NIL_P(rb_path))
	path = StringValueCStr(rb_path);

    table = grn_table_open(*context, name, name_size, path);
    return table;
}

static VALUE
rb_grn_table_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;

    table = rb_grn_table_open(argc, argv, &context);
    rb_grn_object_initialize(self, context, table);
    rb_grn_context_check(context);

    return Qnil;
}

static VALUE
rb_grn_table_s_open (int argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    VALUE rb_table, rb_class;

    table = rb_grn_table_open(argc, argv, &context);
    rb_grn_context_check(context);
    if (klass != rb_cGrnTable) {
	rb_class = GRNOBJECT2RCLASS(table);
	if (klass != rb_class)
	    rb_raise(rb_eTypeError,
		     "unexpected existing table type: %s: expected %s",
		     rb_grn_inspect(rb_class),
		     rb_grn_inspect(klass));
    }

    rb_table = rb_grn_object_alloc(klass);
    rb_grn_table_initialize(argc, argv, rb_table);
    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

static VALUE
rb_grn_table_define_column (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *value_type, *column;
    char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_obj_flags flags = 0;
    rb_grn_boolean use_default_type = RB_GRN_FALSE;
    VALUE rb_name, rb_value_type;
    VALUE options, rb_path, rb_persistent, rb_type;
    VALUE rb_compress, rb_with_section, rb_with_weight, rb_with_position;

    rb_scan_args(argc, argv, "21", &rb_name, &rb_value_type, &options);

    name = StringValuePtr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    rb_grn_scan_options(options,
			"path", &rb_path,
			"persistent", &rb_persistent,
			"type", &rb_type,
			"compress", &rb_compress,
			"with_section", &rb_with_section,
			"with_weight", &rb_with_weight,
			"with_position", &rb_with_position,
			NULL);

    context = rb_grn_object_ensure_context(self, Qnil);

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

    return GRNCOLUMN2RVAL(Qnil, context, column);
}

static VALUE
rb_grn_table_add_column (VALUE self, VALUE rb_name, VALUE rb_value_type,
			 VALUE rb_path)
{
    grn_ctx *context;
    grn_obj *value_type, *column;
    char *name = NULL, *path = NULL;
    unsigned name_size = 0;

    name = StringValuePtr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    context = rb_grn_object_ensure_context(self, Qnil);

    value_type = RVAL2GRNOBJECT(rb_value_type, context);

    path = StringValueCStr(rb_path);

    column = grn_column_open(context, SELF(self), name, name_size,
			     path, value_type);
    rb_grn_context_check(context);

    return GRNCOLUMN2RVAL(Qnil, context, column);
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

    return GRNCOLUMN2RVAL(Qnil, context, column);
}

static VALUE
rb_grn_table_get_columns (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *columns;
    grn_rc rc;
    int n;
    grn_table_cursor *cursor;
    VALUE rb_name, rb_columns;
    char *name = NULL;
    unsigned name_size = 0;

    rb_scan_args(argc, argv, "01", &rb_name);

    if (!NIL_P(rb_name)) {
	name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
    }

    context = rb_grn_object_ensure_context(self, Qnil);

    columns = grn_table_create(context, NULL, 0, NULL, GRN_TABLE_HASH_KEY,
			       NULL, 0, GRN_ENC_DEFAULT);
    n = grn_table_columns(context, SELF(self), name, name_size, columns);
    rb_grn_context_check(context);

    rb_columns = rb_ary_new2(n);
    if (n == 0)
	return rb_columns;

    cursor = grn_table_cursor_open(context, columns, NULL, 0, NULL, 0,
				   GRN_CURSOR_ASCENDING);
    rb_grn_context_check(context);
    while (grn_table_cursor_next(context, cursor) != GRN_ID_NIL) {
	void *key;
	grn_id *column_id;
	grn_obj *column;

	grn_table_cursor_get_key(context, cursor, &key);
	column_id = key;
	column = grn_ctx_get(context, *column_id);
	rb_ary_push(rb_columns, GRNOBJECT2RVAL(Qnil, context, column));
    }
    rc = grn_table_cursor_close(context, cursor);
    if (rc != GRN_SUCCESS) {
	rb_grn_context_check(context);
	rb_grn_check_rc(rc);
    }

    return rb_columns;
}

static VALUE
rb_grn_table_open_cursor (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_table_cursor *cursor;
    VALUE rb_cursor;
    void *min_key = NULL, *max_key = NULL;
    unsigned min_key_size = 0, max_key_size = 0;
    int flags = 0;
    VALUE options, rb_min, rb_max, rb_order, rb_greater_than, rb_less_than;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"min", &rb_min,
                        "max", &rb_max,
			"order", &rb_order,
			"greater_than", &rb_greater_than,
			"less_than", &rb_less_than,
			NULL);

    context = rb_grn_object_ensure_context(self, Qnil);

    if (!NIL_P(rb_min)) {
	min_key = StringValuePtr(rb_min);
	min_key_size = RSTRING_LEN(rb_min);
    }
    if (!NIL_P(rb_max)) {
	max_key = StringValuePtr(rb_max);
	max_key_size = RSTRING_LEN(rb_max);
    }

    if (NIL_P(rb_order)) {
    } else if (rb_grn_equal_option(rb_order, "asc") ||
	       rb_grn_equal_option(rb_order, "ascending")) {
	flags |= GRN_CURSOR_ASCENDING;
    } else if (rb_grn_equal_option(rb_order, "desc") ||
	       rb_grn_equal_option(rb_order, "descending")) {
	flags |= GRN_CURSOR_DESCENDING;
    } else {
	rb_raise(rb_eArgError,
		 "order should be one of "
		 "[:asc, :ascending, :desc, :descending]: %s",
		 rb_grn_inspect(rb_order));
    }

    if (RVAL2CBOOL(rb_greater_than))
	flags |= GRN_CURSOR_GT;
    if (RVAL2CBOOL(rb_less_than))
	flags |= GRN_CURSOR_LT;

    cursor = grn_table_cursor_open(context, SELF(self),
				   min_key, min_key_size,
				   max_key, max_key_size,
				   flags);
    rb_grn_context_check(context);

    rb_cursor = GRNTABLECURSOR2RVAL(Qnil, context, cursor);
    rb_iv_set(rb_cursor, "@table", self);
    if (rb_block_given_p())
	return rb_ensure(rb_yield, rb_cursor,
			 rb_grn_table_cursor_close, rb_cursor);
    else
	return rb_cursor;
}

static VALUE
rb_grn_table_add_with_key (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_id id;
    void *key;
    grn_id id_key = 0;
    char *string_key = NULL;
    unsigned key_size = 0;
    grn_search_flags flags;

    if (RVAL2CBOOL(rb_obj_is_kind_of(rb_key, rb_cInteger))) {
	id_key = NUM2UINT(rb_key);
	key = &id_key;
	key_size = sizeof(grn_id);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(rb_key, rb_cString))) {
	string_key = StringValuePtr(rb_key);
	key = string_key;
	key_size = RSTRING_LEN(rb_key);
    } else {
	rb_raise(rb_eArgError, "key should be integer or string: %s",
		 rb_grn_inspect(rb_key));
    }

    context = rb_grn_object_ensure_context(self, Qnil);

    flags = GRN_SEARCH_EXACT | GRN_TABLE_ADD;
    id = grn_table_lookup(context, SELF(self), key, key_size, &flags);
    rb_grn_context_check(context);

    if (GRN_ID_NIL == id)
	return Qnil;
    else
	return rb_grn_record_new(self, id);
}

static VALUE
rb_grn_array_add (VALUE self)
{
    grn_ctx *context;
    grn_id id;

    context = rb_grn_object_ensure_context(self, Qnil);

    id = grn_table_add(context, SELF(self));
    rb_grn_context_check(context);

    if (GRN_ID_NIL == id)
	return Qnil;
    else
	return rb_grn_record_new(self, id);
}

void
rb_grn_init_table (VALUE mGrn)
{
    rb_cGrnTable = rb_define_class_under(mGrn, "Table", rb_cGrnObject);
    rb_cGrnHash = rb_define_class_under(mGrn, "Hash", rb_cGrnTable);
    rb_cGrnPatriciaTrie =
	rb_define_class_under(mGrn, "PatriciaTrie", rb_cGrnTable);
    rb_cGrnArray = rb_define_class_under(mGrn, "Array", rb_cGrnTable);

    rb_define_singleton_method(rb_cGrnTable, "open",
			       rb_grn_table_s_open, -1);

    rb_define_singleton_method(rb_cGrnHash, "create",
			       rb_grn_hash_s_create, -1);
    rb_define_singleton_method(rb_cGrnPatriciaTrie, "create",
			       rb_grn_patricia_trie_s_create, -1);
    rb_define_singleton_method(rb_cGrnArray, "create",
			       rb_grn_array_s_create, -1);

    rb_define_method(rb_cGrnTable, "initialize", rb_grn_table_initialize, -1);

    rb_define_method(rb_cGrnTable, "define_column",
		     rb_grn_table_define_column, -1);
    rb_define_method(rb_cGrnTable, "add_column",
		     rb_grn_table_add_column, 3);
    rb_define_method(rb_cGrnTable, "column",
		     rb_grn_table_get_column, 1);
    rb_define_method(rb_cGrnTable, "columns",
		     rb_grn_table_get_columns, -1);

    rb_define_method(rb_cGrnTable, "open_cursor", rb_grn_table_open_cursor, -1);

    rb_define_method(rb_cGrnHash, "add", rb_grn_table_add_with_key, 1);
    rb_define_method(rb_cGrnPatriciaTrie, "add", rb_grn_table_add_with_key, 1);
    rb_define_method(rb_cGrnArray, "add", rb_grn_array_add, 0);
}
