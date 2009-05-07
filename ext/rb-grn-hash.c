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

#define SELF(object) (RVAL2GRNTABLE(object))

VALUE rb_cGrnHash;

static void
rb_rb_grn_hash_free (void *object)
{
    RbGrnObject *rb_grn_object = object;
    RbGrnTable *rb_grn_table = object;
    RbGrnTableKeySupport *rb_grn_hash = object;
    grn_ctx *context;
    grn_obj *grn_object;

    context = rb_grn_object->context;
    grn_object = rb_grn_object->object;

    if (rb_grn_object->owner && context && grn_object &&
	rb_grn_context_alive_p(context)) {
	const char *path;

	path = grn_obj_path(context, grn_object);
	if (path == NULL || (path && grn_ctx_db(context))) {
	    grn_obj_close(rb_grn_object->context, rb_grn_object->object);
	}
    }

    if (context && rb_grn_context_alive_p(context)) {
	grn_obj_close(context, rb_grn_table->value);
	grn_obj_close(context, rb_grn_hash->key);
    }
    xfree(rb_grn_hash);
}

static VALUE
rb_grn_hash_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_rb_grn_hash_free, NULL);
}

static void
rb_grn_hash_initialize (VALUE self, VALUE rb_context,
			grn_ctx *context, grn_obj *hash)
{
    RbGrnObject *rb_grn_object;
    RbGrnTable *rb_grn_table;
    RbGrnTableKeySupport *rb_grn_hash;

    rb_grn_hash = ALLOC(RbGrnTableKeySupport);
    DATA_PTR(self) = rb_grn_hash;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_hash);
    rb_grn_object->context = context;
    rb_grn_object->object = hash;
    rb_grn_object->owner = RB_GRN_TRUE;

    rb_grn_table = RB_GRN_TABLE(rb_grn_hash);
    rb_grn_table->value = grn_obj_open(context, GRN_BULK, 0, GRN_ID_NIL);

    rb_grn_hash->key = grn_obj_open(context, GRN_BULK, 0, GRN_ID_NIL);

    rb_iv_set(self, "context", rb_context);
}

static VALUE
rb_grn_hash_s_create (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *key_type = NULL, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0, value_size = 0;
    grn_obj_flags flags = GRN_TABLE_HASH_KEY;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_key_type, rb_value_size;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"name", &rb_name,
                        "path", &rb_path,
			"persistent", &rb_persistent,
			"key_type", &rb_key_type,
			"value_size", &rb_value_size,
			NULL);

    context = rb_grn_context_ensure(&rb_context);

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
	flags |= GRN_OBJ_KEY_VAR_SIZE;
    } else {
	key_type = RVAL2GRNOBJECT(rb_key_type, &context);
    }

    if (!NIL_P(rb_value_size))
	value_size = NUM2UINT(rb_value_size);

    table = grn_table_create(context, name, name_size, path,
			     flags, key_type, value_size);
    rb_table = rb_grn_hash_alloc(self);
    rb_grn_hash_initialize(rb_table, rb_context, context, table);
    rb_grn_context_check(context, rb_table);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

void
rb_grn_init_hash (VALUE mGrn)
{
    rb_cGrnHash = rb_define_class_under(mGrn, "Hash", rb_cGrnTable);
    rb_define_alloc_func(rb_cGrnHash, rb_grn_hash_alloc);

    rb_include_module(rb_cGrnHash, rb_mGrnTableKeySupport);

    rb_define_singleton_method(rb_cGrnHash, "create",
			       rb_grn_hash_s_create, -1);
}
