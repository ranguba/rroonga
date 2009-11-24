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

/*
 * Document-module: Groonga::Table::KeySupport
 *
 * 主キーを持つテーブルであるGroonga::Hashと
 * Groonga::PatriciaTrieに主キーの機能を提供するモジュール。
 */

void
rb_grn_table_key_support_deconstruct (RbGrnTableKeySupport *rb_grn_table_key_support,
				      grn_obj **table_key_support,
				      grn_ctx **context,
				      grn_obj **key,
				      grn_id *domain_id,
				      grn_obj **domain,
				      grn_obj **value,
				      grn_id *range_id,
				      grn_obj **range)
{
    RbGrnTable *rb_grn_table;

    rb_grn_table = RB_GRN_TABLE(rb_grn_table_key_support);

    rb_grn_table_deconstruct(rb_grn_table, table_key_support, context,
			     domain_id, domain,
			     value, range_id, range);

    if (key)
	*key = rb_grn_table_key_support->key;
}

void
rb_grn_table_key_support_finalizer (grn_ctx *context,
				    grn_obj *grn_object,
				    RbGrnTableKeySupport *rb_grn_table_key_support)
{
    if (!context)
	return;

    if (rb_grn_table_key_support->key)
	grn_obj_close(context, rb_grn_table_key_support->key);
    rb_grn_table_key_support->key = NULL;

    rb_grn_table_finalizer(context, grn_object,
			   RB_GRN_TABLE(rb_grn_table_key_support));
}

void
rb_grn_table_key_support_bind (RbGrnTableKeySupport *rb_grn_table_key_support,
			       grn_ctx *context,
			       grn_obj *table_key_support)
{
    RbGrnObject *rb_grn_object;
    RbGrnTable *rb_grn_table;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_table_key_support);

    rb_grn_table = RB_GRN_TABLE(rb_grn_table_key_support);
    rb_grn_table_bind(rb_grn_table, context, table_key_support);

    rb_grn_table_key_support->key =
	grn_obj_open(context, GRN_BULK, 0, rb_grn_object->domain_id);
}

static VALUE
rb_grn_table_key_support_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    VALUE rb_context;

    table = rb_grn_table_open_raw(argc, argv, &context, &rb_context);
    rb_grn_context_check(context, self);
    rb_grn_object_assign(Qnil, self, rb_context, context, table);

    return Qnil;
}

static grn_id
rb_grn_table_key_support_add_raw (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table;
    grn_id id, domain_id;
    grn_obj *key, *domain;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 &key, &domain_id, &domain,
					 NULL, NULL, NULL);

    GRN_BULK_REWIND(key);
    RVAL2GRNKEY(rb_key, context, key, domain_id, domain, self);
    id = grn_table_add(context, table,
                       GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key), NULL);
    rb_grn_context_check(context, self);

    return id;
}

/*
 * call-seq:
 *   table.add(key, values=nil) -> Groonga::Recordまたはnil
 *
 * 主キーが_key_のレコード追加し、追加したレコードを返す。レ
 * コードの追加に失敗した場合は+nil+を返す。
 *
 * _values_にはレコードのカラムに設定する値を指定する。省略
 * した場合または+nil+を指定した場合はカラムは設定しない。カ
 * ラムの値は<tt>{:カラム名1 => 値1, :カラム名2 => 値2,
 * ...}</tt>と指定する。
 */
static VALUE
rb_grn_table_key_support_add (int argc, VALUE *argv, VALUE self)
{
    grn_id id;
    VALUE key, values;

    rb_scan_args(argc, argv, "11", &key, &values);
    id = rb_grn_table_key_support_add_raw(self, key);
    if (GRN_ID_NIL == id)
	return Qnil;
    else
	return rb_grn_record_new(self, id, values);
}

/*
 * call-seq:
 *   table.key(id) -> 主キー
 *
 * テーブルの_id_に対応する主キーを返す。
 */
static VALUE
rb_grn_table_key_support_get_key (VALUE self, VALUE rb_id)
{
    grn_ctx *context;
    grn_obj *table, *key;
    grn_id id;
    int key_size;
    VALUE rb_key;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 &key, NULL, NULL,
					 NULL, NULL, NULL);

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
rb_grn_table_key_support_has_key (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table, *key, *domain;
    grn_id id, domain_id;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 &key, &domain_id, &domain,
					 NULL, NULL, NULL);

    GRN_BULK_REWIND(key);
    RVAL2GRNKEY(rb_key, context, key, domain_id, domain, self);
    id = grn_table_get(context, table, GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key));

    return id == GRN_ID_NIL ? Qfalse : Qtrue;
}

static VALUE
rb_grn_table_key_support_delete_by_key (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table;
    grn_id domain_id;
    grn_obj *key, *domain;
    grn_rc rc;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 &key, &domain_id, &domain,
					 NULL, NULL, NULL);

    GRN_BULK_REWIND(key);
    RVAL2GRNKEY(rb_key, context, key, domain_id, domain, self);
    rc = grn_table_delete(context, table,
			  GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key));
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   table.delete(id)
 *   table.delete(key)
 *
 * テーブルの_id_または_key_に対応するレコードを削除する。
 */
static VALUE
rb_grn_table_key_support_delete (VALUE self, VALUE rb_id_or_key)
{
    if (FIXNUM_P(rb_id_or_key)) {
	return rb_grn_table_delete(self, rb_id_or_key);
    } else {
	return rb_grn_table_key_support_delete_by_key(self, rb_id_or_key);
    }
}

grn_id
rb_grn_table_key_support_get (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table, *key, *domain;
    grn_id id, domain_id;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 &key, &domain_id, &domain,
					 NULL, NULL, NULL);

    GRN_BULK_REWIND(key);
    RVAL2GRNKEY(rb_key, context, key, domain_id, domain, self);
    id = grn_table_get(context, table,
                       GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key));
    rb_grn_context_check(context, self);

    return id;
}

/*
 * call-seq:
 *   table.find(key) -> Groonga::Record
 *
 * テーブルの_key_に対応するレコードを返す。
 */
static VALUE
rb_grn_table_key_support_find (VALUE self, VALUE rb_key)
{
    grn_id id;

    id = rb_grn_table_key_support_get(self, rb_key);

    if (id == GRN_ID_NIL)
	return Qnil;
    else
	return rb_grn_record_new(self, id, Qnil);
}

static VALUE
rb_grn_table_key_support_array_reference_by_key (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table, *value, *range;
    grn_id id;

    id = rb_grn_table_key_support_get(self, rb_key);

    if (id == GRN_ID_NIL)
	return Qnil;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 NULL, NULL, NULL,
					 &value, NULL, &range);
    GRN_BULK_REWIND(value);
    grn_obj_get_value(context, table, id, value);
    rb_grn_context_check(context, self);

    if (GRN_BULK_EMPTYP(value))
	return Qnil;
    else
	return rb_str_new(GRN_BULK_HEAD(value), GRN_BULK_VSIZE(value));
}

static VALUE
rb_grn_table_key_support_array_reference (VALUE self, VALUE rb_id_or_key)
{
    if (FIXNUM_P(rb_id_or_key)) {
	return rb_grn_table_array_reference(self, rb_id_or_key);
    } else {
	return rb_grn_table_key_support_array_reference_by_key(self,
							       rb_id_or_key);
    }
}

static VALUE
rb_grn_table_key_support_array_set_by_key (VALUE self,
					   VALUE rb_key, VALUE rb_value)
{
    grn_ctx *context;
    grn_obj *table;
    grn_id id;
    grn_obj *value;
    grn_rc rc;

    if (NIL_P(rb_key))
	rb_raise(rb_eArgError, "key should not be nil: <%s>",
		 rb_grn_inspect(self));

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 NULL, NULL, NULL,
					 &value, NULL, NULL);

    id = rb_grn_table_key_support_add_raw(self, rb_key);
    if (GRN_ID_NIL == id)
	rb_raise(rb_eGrnError,
		 "failed to add new record with key: <%s>: <%s>",
		 rb_grn_inspect(rb_key),
		 rb_grn_inspect(self));

    GRN_BULK_REWIND(value);
    RVAL2GRNBULK(rb_value, context, value);
    rc = grn_obj_set_value(context, table, id, value, GRN_OBJ_SET);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return rb_value;
}

static VALUE
rb_grn_table_key_support_array_set (VALUE self,
				    VALUE rb_id_or_key, VALUE rb_value)
{
    if (FIXNUM_P(rb_id_or_key)) {
	return rb_grn_table_array_set(self, rb_id_or_key, rb_value);
    } else {
	return rb_grn_table_key_support_array_set_by_key(self,
							 rb_id_or_key,
							 rb_value);
    }
}

static VALUE
rb_grn_table_key_support_get_default_tokenizer (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *tokenizer = NULL;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 NULL, NULL, NULL,
					 NULL, NULL, NULL);

    tokenizer = grn_obj_get_info(context, table, GRN_INFO_DEFAULT_TOKENIZER,
				 NULL);
    rb_grn_context_check(context, self);

    return GRNOBJECT2RVAL(Qnil, context, tokenizer, RB_GRN_FALSE);
}

static VALUE
rb_grn_table_key_support_set_default_tokenizer (VALUE self, VALUE rb_tokenizer)
{
    grn_ctx *context;
    grn_obj *table;
    grn_obj *tokenizer;
    grn_rc rc;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 NULL, NULL, NULL,
					 NULL, NULL, NULL);

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
    rb_include_module(rb_mGrnTableKeySupport, rb_mGrnEncodingSupport);

    rb_define_method(rb_mGrnTableKeySupport, "initialize",
		     rb_grn_table_key_support_initialize, -1);

    rb_define_method(rb_mGrnTableKeySupport, "add",
		     rb_grn_table_key_support_add, -1);
    rb_define_method(rb_mGrnTableKeySupport, "key",
		     rb_grn_table_key_support_get_key, 1);
    rb_define_method(rb_mGrnTableKeySupport, "has_key?",
		     rb_grn_table_key_support_has_key, 1);

    rb_define_method(rb_mGrnTableKeySupport, "delete",
		     rb_grn_table_key_support_delete, 1);

    rb_define_method(rb_mGrnTableKeySupport, "find",
		     rb_grn_table_key_support_find, 1);
    rb_define_method(rb_mGrnTableKeySupport, "[]",
		     rb_grn_table_key_support_array_reference, 1);
    rb_define_method(rb_mGrnTableKeySupport, "[]=",
		     rb_grn_table_key_support_array_set, 2);

    rb_define_method(rb_mGrnTableKeySupport, "default_tokenizer",
		     rb_grn_table_key_support_get_default_tokenizer, 0);
    rb_define_method(rb_mGrnTableKeySupport, "default_tokenizer=",
		     rb_grn_table_key_support_set_default_tokenizer, 1);
}
