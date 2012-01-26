/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>

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
				      grn_obj **range,
				      VALUE *columns)
{
    RbGrnTable *rb_grn_table;

    rb_grn_table = RB_GRN_TABLE(rb_grn_table_key_support);

    rb_grn_table_deconstruct(rb_grn_table, table_key_support, context,
			     domain_id, domain,
			     value, range_id, range,
			     columns);

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
	grn_obj_unlink(context, rb_grn_table_key_support->key);
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

static grn_id
rb_grn_table_key_support_add_raw (VALUE self, VALUE rb_key, int *added)
{
    grn_ctx *context;
    grn_obj *table;
    grn_id id, domain_id;
    grn_obj *key, *domain;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 &key, &domain_id, &domain,
					 NULL, NULL, NULL,
					 NULL);

    GRN_BULK_REWIND(key);
    RVAL2GRNKEY(rb_key, context, key, domain_id, domain, self);
    id = grn_table_add(context, table,
                       GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key), added);
    rb_grn_context_check(context, self);

    return id;
}

/*
 * call-seq:
 *   table.add(key, values=nil) -> Groonga::Recordまたはnil
 *
 * 主キーが _key_ のレコード追加し、追加したレコードを返す。レ
 * コードの追加に失敗した場合は +nil+ を返す。
 *
 * すでに同じキーのレコードが存在する場合は追加せずに同じレ
 * コードを返す。追加されたかどうかは
 * +Groonga::Record#added?+ で調べることができる。 +true+ を返
 * したら追加されたということを示す。
 *
 * _values_ にはレコードのカラムに設定する値を指定する。省略
 * した場合または +nil+ を指定した場合はカラムは設定しない。カ
 * ラムの値は<tt>{:カラム名1 => 値1, :カラム名2 => 値2,
 * ...}</tt>と指定する。
 */
static VALUE
rb_grn_table_key_support_add (int argc, VALUE *argv, VALUE self)
{
    grn_id id;
    VALUE key, values;
    int added = GRN_FALSE;

    rb_scan_args(argc, argv, "11", &key, &values);
    id = rb_grn_table_key_support_add_raw(self, key, &added);
    if (GRN_ID_NIL == id) {
	return Qnil;
    } else {
	if (added) {
	    return rb_grn_record_new_added(self, id, values);
	} else {
	    return rb_grn_record_new(self, id, values);
	}
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
					 NULL, NULL, NULL,
					 NULL);

    GRN_BULK_REWIND(key);
    RVAL2GRNKEY(rb_key, context, key, domain_id, domain, self);
    id = grn_table_get(context, table,
                       GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key));
    rb_grn_context_check(context, self);

    return id;
}

/*
 * call-seq:
 *   table.id -> テーブルID
 *   table.id(key) -> レコードID
 *
 * _key_ を指定しない場合はテーブルのIDを返す。
 *
 * _key_ を指定した場合はテーブルの _key_ に対応するレコードの
 * IDを返す。
 */
static VALUE
rb_grn_table_key_support_get_id (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_key;

    rb_scan_args(argc, argv, "01", &rb_key);
    if (NIL_P(rb_key)) {
	return rb_grn_object_get_id(self);
    } else {
	grn_id id;

	id = rb_grn_table_key_support_get(self, rb_key);
	if (id == GRN_ID_NIL) {
	    return Qnil;
	} else {
	    return UINT2NUM(id);
	}
    }
}

/*
 * call-seq:
 *   table.key(id) -> 主キー
 *
 * テーブルの _id_ に対応する主キーを返す。
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
					 NULL, NULL, NULL,
					 NULL);

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

/*
 * call-seq:
 *   table.has_key?(key) -> true/false
 *
 * テーブルに主キーが _key_ のレコードがあるならtrueを返す。
 */
static VALUE
rb_grn_table_key_support_has_key (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table, *key, *domain;
    grn_id id, domain_id;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 &key, &domain_id, &domain,
					 NULL, NULL, NULL,
					 NULL);

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
					 NULL, NULL, NULL,
					 NULL);

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
 * テーブルの _id_ または _key_ に対応するレコードを削除する。
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

/*
 * Document-method: []
 *
 * call-seq:
 *   table[key] -> Groonga::Record
 *
 * _table_ の _key_ に対応するGroonga::Recordを返す。
 *
 * 0.9.0から値ではなくGroonga::Recordを返すようになった。
 */
static VALUE
rb_grn_table_key_support_array_reference (VALUE self, VALUE rb_key)
{
    grn_id id;

    id = rb_grn_table_key_support_get(self, rb_key);
    if (id == GRN_ID_NIL) {
	return Qnil;
    } else {
	return rb_grn_record_new(self, id, Qnil);
    }
}

typedef struct _SetValueData
{
    VALUE self;
    grn_id id;
    grn_obj *table;
    RbGrnObject rb_grn_object;
} SetValueData;

static VALUE
set_value (VALUE args, SetValueData *data)
{
    VALUE rb_name, rb_value, rb_column;
    RbGrnObject *rb_grn_object;

    rb_name = rb_ary_entry(args, 0);
    rb_value = rb_ary_entry(args, 1);

    rb_column = rb_grn_table_get_column(data->self, rb_name);
    if (NIL_P(rb_column)) {
	rb_raise(rb_eGrnNoSuchColumn,
		 "no such column: <%s>: <%s>",
		 rb_grn_inspect(rb_name), rb_grn_inspect(data->self));
    }

    rb_grn_object = RB_GRN_OBJECT(DATA_PTR(rb_column));
    return rb_grn_object_set_raw(rb_grn_object,
				 data->id, rb_value, GRN_OBJ_SET, data->self);
}

/*
 * Document-method: []=
 *
 * call-seq:
 *   table[key] = {:column_name => value, ...}
 *
 * _table_ の _key_ に対応するカラム _column_name_ の値を設定する。
 * _key_ に対応するレコードがない場合は新しく作成される。
 *
 * 0.9.0から値ではなくカラムの値を設定するようになった。
 */
static VALUE
rb_grn_table_key_support_array_set (VALUE self, VALUE rb_key, VALUE rb_values)
{
    grn_id id;
    SetValueData data;
    grn_ctx *context;
    grn_obj *table;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 NULL, NULL, NULL,
					 NULL, NULL, NULL,
					 NULL);

    id = rb_grn_table_key_support_add_raw(self, rb_key, NULL);

    if (id == GRN_ID_NIL) {
	rb_raise(rb_eGrnError,
		 "failed to add record: %s",
		 rb_grn_inspect(rb_ary_new3(3, self, rb_key, rb_values)));
    }

    data.self = self;
    data.id = id;
    data.table = table;
    data.rb_grn_object.context = context;
    rb_iterate(rb_each, rb_values, set_value, (VALUE)&data);

    return Qnil;
}

/*
 * call-seq:
 *   table.set_column_value(key, name, value)
 *   table.set_column_value(id, name, value, :id => true)
 *
 * _table_ の _key_ に対応するカラム _name_ の値を設定する。
 * _key_ に対応するレコードがない場合は新しく作成される。
 */
static VALUE
rb_grn_table_key_support_set_column_value (int argc, VALUE *argv, VALUE self)
{
    grn_id id;
    VALUE rb_key, rb_id_or_key, rb_name, rb_value, rb_options;

    rb_scan_args(argc, argv, "31",
		 &rb_id_or_key, &rb_name, &rb_value, &rb_options);
    if (!NIL_P(rb_options)) {
	VALUE rb_option_id;
	rb_grn_scan_options(rb_options,
			    "id", &rb_option_id,
			    NULL);
	if (RVAL2CBOOL(rb_option_id)) {
	    VALUE rb_id = rb_id_or_key;
	    return rb_grn_table_set_column_value(self, rb_id, rb_name, rb_value);
	}
    }

    rb_key = rb_id_or_key;
    id = rb_grn_table_key_support_add_raw(self, rb_key, NULL);
    if (id == GRN_ID_NIL) {
	rb_raise(rb_eGrnError,
		 "failed to add record: %s",
		 rb_grn_inspect(rb_ary_new3(4,
					    self, rb_key,
					    rb_name, rb_value)));
    }

    return rb_grn_table_set_column_value_raw(self, id, rb_name, rb_value);
}

/*
 * call-seq:
 *   table.column_value(key, name)
 *   table.column_value(id, name, :id => true)
 *
 * _table_ の _key_ に対応するカラム _name_ の値を設定する。
 *
 * TODO: _key_ に対応するレコードがない場合は例外？
 */
static VALUE
rb_grn_table_key_support_get_column_value (int argc, VALUE *argv, VALUE self)
{
    grn_id id;
    VALUE rb_key, rb_id_or_key, rb_name, rb_options;

    rb_scan_args(argc, argv, "21", &rb_id_or_key, &rb_name, &rb_options);
    if (!NIL_P(rb_options)) {
	VALUE rb_option_id;
	rb_grn_scan_options(rb_options,
			    "id", &rb_option_id,
			    NULL);
	if (RVAL2CBOOL(rb_option_id)) {
	    VALUE rb_id = rb_id_or_key;
	    return rb_grn_table_get_column_value(self, rb_id, rb_name);
	}
    }

    rb_key = rb_id_or_key;
    id = rb_grn_table_key_support_get(self, rb_key);
    if (id == GRN_ID_NIL) {
	return Qnil;
    }

    return rb_grn_table_get_column_value_raw(self, id, rb_name);
}

static VALUE
rb_grn_table_key_support_get_value_by_key (VALUE self, VALUE rb_key)
{
    grn_ctx *context;
    grn_obj *table, *value, *range;
    grn_id id;

    id = rb_grn_table_key_support_get(self, rb_key);

    if (id == GRN_ID_NIL)
	return Qnil;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 NULL, NULL, NULL,
					 &value, NULL, &range,
					 NULL);
    GRN_BULK_REWIND(value);
    grn_obj_get_value(context, table, id, value);
    rb_grn_context_check(context, self);

    return GRNBULK2RVAL(context, value, range, self);
}

/*
 * call-seq:
 *   table.value(id, :id => true) -> 値
 *   table.value(key) -> 値
 *
 * _table_ の _id_ または _key_ に対応する値を返す。
 */
static VALUE
rb_grn_table_key_support_get_value (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_id_or_key, rb_options;
    grn_bool use_key;

    rb_scan_args(argc, argv, "11", &rb_id_or_key, &rb_options);

    if (NIL_P(rb_options)) {
	use_key = GRN_TRUE;
    } else {
	VALUE rb_option_id;

	rb_grn_scan_options(rb_options,
			    "id", &rb_option_id,
			    NULL);
	use_key = !RVAL2CBOOL(rb_option_id);
    }

    if (use_key) {
	return rb_grn_table_key_support_get_value_by_key(self, rb_id_or_key);
    } else {
	return rb_grn_table_get_value(self, rb_id_or_key);
    }
}

static VALUE
rb_grn_table_key_support_set_value_by_key (VALUE self,
					   VALUE rb_key, VALUE rb_value)
{
    grn_ctx *context;
    grn_obj *table;
    grn_id id;
    grn_obj *value;
    grn_rc rc;

    if (NIL_P(rb_key)) {
	rb_raise(rb_eArgError, "key should not be nil: <%s>",
		 rb_grn_inspect(self));
    }

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 NULL, NULL, NULL,
					 &value, NULL, NULL,
					 NULL);

    id = rb_grn_table_key_support_add_raw(self, rb_key, NULL);
    if (GRN_ID_NIL == id) {
	rb_raise(rb_eGrnError,
		 "failed to add new record with key: <%s>: <%s>",
		 rb_grn_inspect(rb_key),
		 rb_grn_inspect(self));
    }

    GRN_BULK_REWIND(value);
    RVAL2GRNBULK(rb_value, context, value);
    rc = grn_obj_set_value(context, table, id, value, GRN_OBJ_SET);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return rb_value;
}

/*
 * call-seq:
 *   table.set_value(id, value, :id => true)
 *   table.set_value(key, value)
 *
 * _table_ の _id_ または _key_ に対応する値を _value_ に設定する。
 * 既存の値は上書きされる。
 */
static VALUE
rb_grn_table_key_support_set_value (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_id_or_key, rb_value, rb_options;
    grn_bool use_key;

    rb_scan_args(argc, argv, "21", &rb_id_or_key, &rb_value, &rb_options);

    if (NIL_P(rb_options)) {
	use_key = GRN_TRUE;
    } else {
	VALUE rb_option_id;

	rb_grn_scan_options(rb_options,
			    "id", &rb_option_id,
			    NULL);
	use_key = !RVAL2CBOOL(rb_option_id);
    }

    if (use_key) {
	return rb_grn_table_key_support_set_value_by_key(self,
							 rb_id_or_key,
							 rb_value);
    } else {
	return rb_grn_table_set_value(self, rb_id_or_key, rb_value);
    }
}

/*
 * call-seq:
 *   table.default_tokenizer -> nilまたはGroonga::Procedure
 *
 * Groonga::IndexColumnで使用するトークナイザを返す。
 */
static VALUE
rb_grn_table_key_support_get_default_tokenizer (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *tokenizer = NULL;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 NULL, NULL, NULL,
					 NULL, NULL, NULL,
					 NULL);

    tokenizer = grn_obj_get_info(context, table, GRN_INFO_DEFAULT_TOKENIZER,
				 NULL);
    rb_grn_context_check(context, self);

    return GRNOBJECT2RVAL(Qnil, context, tokenizer, GRN_FALSE);
}

/*
 * call-seq:
 *   table.default_tokenizer = トークナイザ
 *
 * Groonga::IndexColumnで使用するトークナイザを設定する。
 *
 * @example
 *   # 2-gramを使用。
 *   table.default_tokenizer = "TokenBigram"
 *   # オブジェクトで指定
 *   table.default_tokenizer = Groonga::Context.default["TokenBigram"]
 *   # オブジェクトIDで指定
 *   table.default_tokenizer = Groonga::Type::BIGRAM
 *   # N-gram用のトークナイザを使うときはGroonga::IndexColumn
 *   # には自動的に:with_section => trueが指定される。
 *   index = table.define_index_column("blog_content", "Blogs",
 *                                     :source => "content")
 *   p index # -> #<Groonga::IndexColumn ... flags: <WITH_POSITION|...>>
 *
 *   # MeCabを使用
 *   table.default_tokenizer = "TokenMecab"
 */
static VALUE
rb_grn_table_key_support_set_default_tokenizer (VALUE self, VALUE rb_tokenizer)
{
    grn_ctx *context;
    grn_obj *table;
    grn_obj *tokenizer;
    grn_rc rc;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 NULL, NULL, NULL,
					 NULL, NULL, NULL,
					 NULL);

    tokenizer = RVAL2GRNOBJECT(rb_tokenizer, &context);
    rc = grn_obj_set_info(context, table,
			  GRN_INFO_DEFAULT_TOKENIZER, tokenizer);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   table.normalize_key? -> true/false
 *
 * キーを正規化する場合は +true+ 、正規化しない場合は +false+ を返
 * す。
 */
static VALUE
rb_grn_table_key_normalize_key_p (VALUE self)
{
    grn_obj *table;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, NULL,
					 NULL, NULL, NULL,
					 NULL, NULL, NULL,
					 NULL);
    return CBOOL2RVAL(table->header.flags & GRN_OBJ_KEY_NORMALIZE);
}

/*
 * call-seq:
 *   table.support_key? -> true/false
 *
 * テーブルでキーを使えるなら +true+ 、使えないなら +false+ を返
 * す。キーを使えないテーブルはGroonga::Arrayだけ。
 */
static VALUE
rb_grn_table_key_support_support_key_p (VALUE self)
{
    grn_obj *table;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, NULL,
					 NULL, NULL, NULL,
					 NULL, NULL, NULL,
					 NULL);
    return table->header.domain == GRN_ID_NIL ? Qfalse : Qtrue;
}

void
rb_grn_init_table_key_support (VALUE mGrn)
{
    rb_mGrnTableKeySupport = rb_define_module_under(rb_cGrnTable, "KeySupport");
    rb_include_module(rb_mGrnTableKeySupport, rb_mGrnEncodingSupport);

    rb_define_method(rb_mGrnTableKeySupport, "add",
		     rb_grn_table_key_support_add, -1);
    rb_define_method(rb_mGrnTableKeySupport, "id",
		     rb_grn_table_key_support_get_id, -1);
    rb_define_method(rb_mGrnTableKeySupport, "key",
		     rb_grn_table_key_support_get_key, 1);
    rb_define_method(rb_mGrnTableKeySupport, "has_key?",
		     rb_grn_table_key_support_has_key, 1);

    rb_define_method(rb_mGrnTableKeySupport, "delete",
		     rb_grn_table_key_support_delete, 1);

    rb_define_method(rb_mGrnTableKeySupport, "[]",
		     rb_grn_table_key_support_array_reference, 1);
    rb_define_method(rb_mGrnTableKeySupport, "[]=",
		     rb_grn_table_key_support_array_set, 2);

    rb_define_method(rb_mGrnTableKeySupport, "column_value",
		     rb_grn_table_key_support_get_column_value, -1);
    rb_define_method(rb_mGrnTableKeySupport, "set_column_value",
		     rb_grn_table_key_support_set_column_value, -1);

    rb_define_method(rb_mGrnTableKeySupport, "value",
		     rb_grn_table_key_support_get_value, -1);
    rb_define_method(rb_mGrnTableKeySupport, "set_value",
		     rb_grn_table_key_support_set_value, -1);

    rb_define_method(rb_mGrnTableKeySupport, "default_tokenizer",
		     rb_grn_table_key_support_get_default_tokenizer, 0);
    rb_define_method(rb_mGrnTableKeySupport, "default_tokenizer=",
		     rb_grn_table_key_support_set_default_tokenizer, 1);

    rb_define_method(rb_mGrnTableKeySupport, "normalize_key?",
		     rb_grn_table_key_normalize_key_p, 0);

    rb_define_method(rb_mGrnTableKeySupport, "support_key?",
		     rb_grn_table_key_support_support_key_p, 0);
}
