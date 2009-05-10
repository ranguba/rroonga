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

#define SELF(object) ((RbGrnTableKeySupport *)DATA_PTR(object))

VALUE rb_mGrnTableKeySupport;

/* FIXME */
grn_rc grn_table_get_info(grn_ctx *ctx, grn_obj *table, grn_obj_flags *flags,
                          grn_encoding *encoding, grn_obj **tokenizer);

static void
rb_grn_table_key_support_deconstruct (VALUE self,
				      grn_obj **table,
				      grn_ctx **context,
				      grn_obj **key,
				      grn_obj **domain,
				      grn_obj **value,
				      grn_obj **range)
{
    RbGrnObject *rb_grn_object;
    RbGrnTable *rb_grn_table;
    RbGrnTableKeySupport *rb_grn_table_key_support;

    rb_grn_table_key_support = SELF(self);
    rb_grn_table = RB_GRN_TABLE(rb_grn_table_key_support);
    rb_grn_object = RB_GRN_OBJECT(rb_grn_table_key_support);

    if (table)
	*table = rb_grn_object->object;
    if (context)
	*context = rb_grn_object->context;
    if (domain)
	*domain = rb_grn_object->domain;
    if (range)
	*range = rb_grn_object->range;
    if (key)
	*key = rb_grn_table_key_support->key;
    if (value)
	*value = rb_grn_table->value;
}

static void
rb_rb_grn_table_key_support_free (void *object)
{
    RbGrnObject *rb_grn_object = object;
    RbGrnTable *rb_grn_table = object;
    RbGrnTableKeySupport *rb_grn_table_key_support = object;
    grn_ctx *context;
    grn_obj *grn_object;

    context = rb_grn_object->context;
    grn_object = rb_grn_object->object;

    if (rb_grn_object->owner && context && grn_object &&
	rb_grn_context_alive_p(context)) {
	const char *path;

	path = grn_obj_path(context, grn_object);
	if (path == NULL || (path && grn_ctx_db(context))) {
	    grn_obj_close(rb_grn_object->context, grn_object);
	}
    }

    if (context && rb_grn_context_alive_p(context)) {
	grn_obj_close(context, rb_grn_table->value);
	grn_obj_close(context, rb_grn_table_key_support->key);
    }
    xfree(rb_grn_table_key_support);
}

VALUE
rb_grn_table_key_support_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_rb_grn_table_key_support_free, NULL);
}

void
rb_grn_table_key_support_assign (VALUE self, VALUE rb_context,
				 grn_ctx *context,
				 grn_obj *table_key_support,
				 rb_grn_boolean owner)
{
    RbGrnObject *rb_grn_object;
    RbGrnTable *rb_grn_table;
    RbGrnTableKeySupport *rb_grn_table_key_support;
    grn_id domain_id = GRN_ID_NIL;
    grn_id range_id = GRN_ID_NIL;

    rb_grn_table_key_support = ALLOC(RbGrnTableKeySupport);
    DATA_PTR(self) = rb_grn_table_key_support;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_table_key_support);
    rb_grn_object->context = context;
    rb_grn_object->object = table_key_support;

    if (table_key_support)
	domain_id = table_key_support->header.domain;
    if (domain_id == GRN_ID_NIL)
	rb_grn_object->domain = NULL;
    else
	rb_grn_object->domain = grn_ctx_get(context, domain_id);

    if (table_key_support)
	range_id = grn_obj_get_range(context, table_key_support);
    if (range_id == GRN_ID_NIL)
	rb_grn_object->range = NULL;
    else
	rb_grn_object->range = grn_ctx_get(context, range_id);

    rb_grn_object->owner = owner;

    rb_grn_table = RB_GRN_TABLE(rb_grn_table_key_support);
    rb_grn_table->value = grn_obj_open(context, GRN_BULK, 0, GRN_ID_NIL);

    rb_grn_table_key_support->key =
	grn_obj_open(context, GRN_BULK, 0, GRN_ID_NIL);

    rb_iv_set(self, "context", rb_context);
}

static VALUE
rb_grn_table_key_support_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    VALUE rb_context;

    table = rb_grn_table_open_raw(argc, argv, &context, &rb_context);
    rb_grn_table_key_support_assign(self, rb_context, context,
				    table, RB_GRN_TRUE);
    rb_grn_context_check(context, self);

    return Qnil;
}

static grn_id
rb_grn_table_key_support_add_raw (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table;
    grn_id id;
    grn_obj *key, *domain;
    grn_search_flags flags;

    rb_grn_table_key_support_deconstruct(self, &table, &context, &key, &domain,
					 NULL, NULL);

    GRN_BULK_REWIND(key);
    RVAL2GRNKEY(rb_key, context, key, domain, self);
    flags = GRN_SEARCH_EXACT | GRN_TABLE_ADD;
    id = grn_table_lookup(context, table,
			  GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key),
			  &flags);
    rb_grn_context_check(context, self);

    return id;
}

static VALUE
rb_grn_table_key_support_add (VALUE self, VALUE rb_key)
{
    grn_id id;

    id = rb_grn_table_key_support_add_raw(self, rb_key);
    if (GRN_ID_NIL == id)
	return Qnil;
    else
	return rb_grn_record_new(self, id);
}

static VALUE
rb_grn_table_key_support_get_key (VALUE self, VALUE rb_id)
{
    grn_ctx *context;
    grn_obj *table, *key;
    grn_id id;
    int key_size;
    VALUE rb_key;

    rb_grn_table_key_support_deconstruct(self, &table, &context, &key, NULL,
					 NULL, NULL);

    id = NUM2UINT(rb_id);
    GRN_BULK_REWIND(key);
    key_size = grn_table_get_key(context, table, id,
				 GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key));
    if (key_size == 0)
	return Qnil;

    if (GRN_BULK_VSIZE(key) < key_size) {
	grn_bulk_reserve(context, key, key_size);
	grn_table_get_key(context, table, id, GRN_BULK_HEAD(key), key_size);
    }

    rb_key = GRNKEY2RVAL(context, GRN_BULK_HEAD(key), key_size, table, self);
    return rb_key;
}

static VALUE
rb_grn_table_key_support_delete_by_key (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table;
    grn_obj *key, *domain;
    grn_rc rc;

    rb_grn_table_key_support_deconstruct(self, &table, &context, &key, &domain,
					 NULL, NULL);

    GRN_BULK_REWIND(key);
    RVAL2GRNKEY(rb_key, context, key, domain, self);
    rc = grn_table_delete(context, table,
			  GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key));
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

static VALUE
rb_grn_table_key_support_delete (VALUE self, VALUE rb_id_or_key)
{
    if (FIXNUM_P(rb_id_or_key)) {
	int argc = 1;
	VALUE argv[1];
	argv[0] = rb_id_or_key;
	return rb_call_super(argc, argv);
    } else {
	return rb_grn_table_key_support_delete_by_key(self, rb_id_or_key);
    }
}

static grn_id
rb_grn_table_key_support_lookup_raw (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table, *key, *domain;
    grn_id id;
    grn_search_flags flags = 0;

    rb_grn_table_key_support_deconstruct(self, &table, &context, &key, &domain,
					 NULL, NULL);

    GRN_BULK_REWIND(key);
    RVAL2GRNKEY(rb_key, context, key, domain, self);
    flags = GRN_SEARCH_EXACT;
    id = grn_table_lookup(context, table,
			  GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key),
			  &flags);
    rb_grn_context_check(context, self);

    return id;
}

static VALUE
rb_grn_table_key_support_lookup (VALUE self, VALUE rb_key)
{
    grn_id id;

    id = rb_grn_table_key_support_lookup_raw(self, rb_key);

    if (id == GRN_ID_NIL)
	return Qnil;
    else
	return rb_grn_record_new(self, id);
}

static VALUE
rb_grn_table_key_support_array_reference_by_key (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table, *value, *range;
    grn_id id;

    id = rb_grn_table_key_support_lookup_raw(self, rb_key);

    if (id == GRN_ID_NIL)
	return Qnil;

    rb_grn_table_key_support_deconstruct(self, &table, &context, NULL, NULL,
					 &value, &range);
    grn_obj_get_value(context, table, id, value);
    rb_grn_context_check(context, self);

    return GRNVALUE2RVAL(context, value, range, self);
}

static VALUE
rb_grn_table_key_support_array_reference (VALUE self, VALUE rb_id_or_key)
{
    if (FIXNUM_P(rb_id_or_key)) {
	int argc = 1;
	VALUE argv[1];
	argv[0] = rb_id_or_key;
	return rb_call_super(argc, argv);
    } else {
	return rb_grn_table_key_support_array_reference_by_key(self,
							       rb_id_or_key);
    }
}

static VALUE
rb_grn_table_key_support_array_set_by_key (VALUE self,
					   VALUE rb_key, VALUE rb_value)
{
    VALUE exception;
    grn_ctx *context;
    grn_obj *table;
    grn_id id;
    grn_obj *value;
    grn_rc rc;

    if (NIL_P(rb_key))
	rb_raise(rb_eArgError, "key should not be nil: <%s>",
		 rb_grn_inspect(self));

    rb_grn_table_key_support_deconstruct(self, &table, &context, NULL, NULL,
					 &value, NULL);

    id = rb_grn_table_key_support_add_raw(self, rb_key);
    if (GRN_ID_NIL == id)
	rb_raise(rb_eGrnError,
		 "failed to add new record with key: <%s>: <%s>",
		 rb_grn_inspect(rb_key),
		 rb_grn_inspect(self));

    GRN_BULK_REWIND(value);
    RVAL2GRNBULK(rb_value, context, value);
    rc = grn_obj_set_value(context, table, id, value, GRN_OBJ_SET);
    exception = rb_grn_context_to_exception(context, self);
    if (!NIL_P(exception))
	rb_exc_raise(exception);
    rb_grn_rc_check(rc, self);

    return rb_value;
}

static VALUE
rb_grn_table_key_support_array_set (VALUE self,
				    VALUE rb_id_or_key, VALUE rb_value)
{
    if (FIXNUM_P(rb_id_or_key)) {
	int argc = 2;
	VALUE argv[2];
	argv[0] = rb_id_or_key;
	argv[1] = rb_value;
	return rb_call_super(argc, argv);
    } else {
	return rb_grn_table_key_support_array_set_by_key(self,
							 rb_id_or_key,
							 rb_value);
    }
}

static VALUE
rb_grn_table_key_support_get_encoding (VALUE self)
{
    grn_ctx *context;
    grn_obj *table;
    grn_encoding encoding = GRN_ENC_NONE;
    grn_obj *value;

    rb_grn_table_key_support_deconstruct(self, &table, &context, NULL, NULL,
					 &value, NULL);

    GRN_BULK_REWIND(value);
    grn_bulk_reserve(context, value, sizeof(grn_encoding));
    grn_obj_get_info(context, table, GRN_INFO_ENCODING, value);
    rb_grn_context_check(context, self);
    memcpy(&encoding, GRN_BULK_HEAD(value), sizeof(grn_encoding));

    return GRNENCODING2RVAL(encoding);
}

static VALUE
rb_grn_table_key_support_get_default_tokenizer (VALUE self)
{
    grn_ctx *context;
    grn_obj *table;
    grn_obj *tokenizer;
    grn_rc rc;

    rb_grn_table_key_support_deconstruct(self, &table, &context, NULL, NULL,
					 NULL, NULL);

    rc = grn_table_get_info(context, table, NULL, NULL, &tokenizer);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return GRNOBJECT2RVAL(Qnil, context, tokenizer, RB_GRN_FALSE);
}

static VALUE
rb_grn_table_key_support_set_default_tokenizer (VALUE self, VALUE rb_tokenizer)
{
    grn_ctx *context;
    grn_obj *table;
    grn_obj *tokenizer;
    grn_rc rc;

    rb_grn_table_key_support_deconstruct(self, &table, &context, NULL, NULL,
					 NULL, NULL);

    tokenizer = RVAL2GRNOBJECT(rb_tokenizer, &context);
    rc = grn_obj_set_info(context, table,
			  GRN_INFO_DEFAULT_TOKENIZER, tokenizer);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

void
rb_grn_init_table_key_support (VALUE mGrn)
{
    rb_mGrnTableKeySupport = rb_define_module_under(rb_cGrnTable, "KeySupport");

    rb_define_method(rb_mGrnTableKeySupport, "initialize",
		     rb_grn_table_key_support_initialize, -1);

    rb_define_method(rb_mGrnTableKeySupport, "add",
		     rb_grn_table_key_support_add, 1);
    rb_define_method(rb_mGrnTableKeySupport, "key",
		     rb_grn_table_key_support_get_key, 1);

    rb_define_method(rb_mGrnTableKeySupport, "delete",
		     rb_grn_table_key_support_delete, 1);

    rb_define_method(rb_mGrnTableKeySupport, "lookup",
		     rb_grn_table_key_support_lookup, 1);
    rb_define_method(rb_mGrnTableKeySupport, "[]",
		     rb_grn_table_key_support_array_reference, 1);
    rb_define_method(rb_mGrnTableKeySupport, "[]=",
		     rb_grn_table_key_support_array_set, 2);

    rb_define_method(rb_mGrnTableKeySupport, "encoding",
		     rb_grn_table_key_support_get_encoding, 0);

    rb_define_method(rb_mGrnTableKeySupport, "default_tokenizer",
		     rb_grn_table_key_support_get_default_tokenizer, 0);

    rb_define_method(rb_mGrnTableKeySupport, "default_tokenizer=",
		     rb_grn_table_key_support_set_default_tokenizer, 1);
}
