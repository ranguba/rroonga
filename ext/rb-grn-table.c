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

#define SELF(object) ((RbGrnTable *)DATA_PTR(object))

VALUE rb_cGrnTable;

/*
 * Document-class: Groonga::Table < Groonga::Object
 *
 * Ruby/groongaが提供するテーブルのベースとなるクラス。このクラス
 * からGroonga::Array, Groonga::Hash, Groonga::PatriciaTrie
 * が継承されている。
 */

grn_obj *
rb_grn_table_from_ruby_object (VALUE object, grn_ctx **context)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnTable))) {
	rb_raise(rb_eTypeError, "not a groonga table");
    }

    return RVAL2GRNOBJECT(object, context);
}

VALUE
rb_grn_table_to_ruby_object (grn_ctx *context, grn_obj *table,
			     rb_grn_boolean owner)
{
    return GRNOBJECT2RVAL(Qnil, context, table, owner);
}

void
rb_grn_table_finalizer (grn_ctx *context, grn_obj *object,
			RbGrnTable *rb_grn_table)
{
    if (context && rb_grn_table->value)
	grn_obj_close(context, rb_grn_table->value);
    rb_grn_table->value = NULL;
}

void
rb_grn_table_bind (RbGrnTable *rb_grn_table,
		   grn_ctx *context, grn_obj *table)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_table);
    rb_grn_table->value = grn_obj_open(context, GRN_BULK, 0,
				       rb_grn_object->range_id);
}

void
rb_grn_table_deconstruct (RbGrnTable *rb_grn_table,
			  grn_obj **table,
			  grn_ctx **context,
			  grn_id *domain_id,
			  grn_obj **domain,
			  grn_obj **value,
			  grn_id *range_id,
			  grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_table);
    rb_grn_object_deconstruct(rb_grn_object, table, context,
			      domain_id, domain,
			      range_id, range);

    if (value)
	*value = rb_grn_table->value;
}

static void
rb_grn_table_mark (void *data)
{
    RbGrnObject *rb_grn_object = data;
    grn_ctx *context;
    grn_obj *table;
    grn_obj *column_ids;
    int n;
    grn_table_cursor *cursor;

    context = rb_grn_object->context;
    table = rb_grn_object->object;
    if (!context || !table)
	return;

    if (!grn_obj_path(context, table))
	return;

    if (grn_obj_name(context, table, NULL, 0) == 0)
	return;

    column_ids = grn_table_create(context, NULL, 0, NULL,
				  GRN_TABLE_HASH_KEY, NULL, 0);
    n = grn_table_columns(context, table, NULL, 0, column_ids);
    if (n == 0) {
	grn_obj_close(context, column_ids);
	return;
    }

    cursor = grn_table_cursor_open(context, column_ids, NULL, 0, NULL, 0,
				   0, -1, GRN_CURSOR_ASCENDING);
    while (grn_table_cursor_next(context, cursor) != GRN_ID_NIL) {
	void *key;
	grn_id *column_id;
	grn_obj *column;
	RbGrnObject *rb_grn_column;

	grn_table_cursor_get_key(context, cursor, &key);
	column_id = key;
	column = grn_ctx_at(context, *column_id);
	rb_grn_column = grn_obj_user_data(context, column)->ptr;
	if (rb_grn_column)
	    rb_gc_mark(rb_grn_column->self);
    }
    grn_table_cursor_close(context, cursor);
    grn_obj_close(context, column_ids);
}

static VALUE
rb_grn_table_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, rb_grn_table_mark, rb_grn_object_free, NULL);
}

VALUE
rb_grn_table_s_create (int argc, VALUE *argv, VALUE klass,
		       grn_obj_flags key_store)
{
    grn_ctx *context;
    grn_obj *key_type = NULL, *value_type = NULL, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_obj_flags flags = key_store;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_key_normalize, rb_key_with_sis, rb_key_type;
    VALUE rb_value_type;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"name", &rb_name,
                        "path", &rb_path,
			"persistent", &rb_persistent,
			"key_normalize", &rb_key_normalize,
			"key_with_sis", &rb_key_with_sis,
			"key_type", &rb_key_type,
			"value_type", &rb_value_type,
			NULL);

    context = rb_grn_context_ensure(&rb_context);

    if (!NIL_P(rb_name)) {
        name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
	flags |= GRN_OBJ_PERSISTENT;
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
	key_type = RVAL2GRNOBJECT(rb_key_type, &context);
    }

    if (!NIL_P(rb_value_type))
	value_type = RVAL2GRNOBJECT(rb_value_type, &context);

    table = grn_table_create(context, name, name_size, path,
			     flags, key_type, value_type);
    rb_table = rb_grn_object_alloc(klass);
    rb_grn_table_assign(rb_table, rb_context, context, table, RB_GRN_TRUE);
    rb_grn_context_check(context, rb_table);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

grn_obj *
rb_grn_table_open_raw (int argc, VALUE *argv,
		       grn_ctx **context, VALUE *rb_context)
{
    grn_obj *table;
    char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    VALUE rb_path, options, rb_name;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"context", rb_context,
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
    VALUE rb_context;

    table = rb_grn_table_open_raw(argc, argv, &context, &rb_context);
    rb_grn_object_assign(Qnil, self, rb_context, context, table);
    rb_grn_context_check(context, self);

    return Qnil;
}

/*
 * call-seq:
 *   Groonga::Table.open(options={})                -> Groonga::Table
 *   Groonga::Table.open(options={}) {|table| ... }
 *
 * 既存のテーブルを開く。ブロックを指定すると、そのブロック
 * に開かれたテーブルが渡され、ブロックを抜けると自動的にテ
 * ーブルが破棄される。
 *
 * _options_に指定可能な値は以下の通り。
 *
 * [+:context+]
 *   テーブルが利用するGroonga::Context。省略すると
 *   Groonga::Context.defaultを用いる。
 *
 * [+:name+]
 *   開こうとするテーブルの名前。
 *
 * [+:path+]
 *   開こうとするテーブルのパス。
 */
static VALUE
rb_grn_table_s_open (int argc, VALUE *argv, VALUE klass)
{
    grn_user_data *user_data;
    VALUE rb_table = Qnil;
    grn_obj *table;
    grn_ctx *context = NULL;
    VALUE rb_context;

    table = rb_grn_table_open_raw(argc, argv, &context, &rb_context);
    rb_grn_context_check(context, rb_ary_new4(argc, argv));

    if (!table)
	rb_raise(rb_eGrnError,
		 "unable to open table: %s: %s",
		 rb_grn_inspect(klass),
		 rb_grn_inspect(rb_ary_new4(argc, argv)));

    user_data = grn_obj_user_data(context, table);
    if (user_data && user_data->ptr) {
	rb_table = RB_GRN_OBJECT(user_data->ptr)->self;
    } else {
	if (klass == rb_cGrnTable) {
	    klass = GRNOBJECT2RCLASS(table);
	} else {
	    VALUE rb_class;

	    rb_class = GRNOBJECT2RCLASS(table);
	    if (rb_class != klass) {
		rb_raise(rb_eTypeError,
			 "unexpected existing table type: %s: expected %s",
			 rb_grn_inspect(rb_class),
			 rb_grn_inspect(klass));
	    }
	}

	rb_table = rb_grn_object_alloc(klass);
	rb_grn_object_assign(klass, rb_table, rb_context, context, table);
    }

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

static VALUE
rb_grn_table_inspect_content (VALUE self, VALUE inspected)
{
    grn_ctx *context = NULL;
    grn_obj *table;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    if (!table)
	return inspected;
    if (!context)
	return inspected;

    if (table->header.type != GRN_TABLE_NO_KEY) {
	grn_obj value;
	grn_encoding encoding;

	rb_str_cat2(inspected, ", ");
	rb_str_cat2(inspected, "encoding: <");
	GRN_OBJ_INIT(&value, GRN_BULK, 0, GRN_ID_NIL);
	grn_obj_get_info(context, table, GRN_INFO_ENCODING, &value);
	encoding = *((grn_encoding *)GRN_BULK_HEAD(&value));
	grn_obj_close(context, &value);

	if (context->rc == GRN_SUCCESS)
	    rb_str_concat(inspected, rb_inspect(GRNENCODING2RVAL(encoding)));
	else
	    rb_str_cat2(inspected, "invalid");

	rb_str_cat2(inspected, ">");
    }

    rb_str_cat2(inspected, ", ");
    rb_str_cat2(inspected, "size: <");
    {
	char buf[21]; /* ceil(log10(2 ** 64)) + 1('\0') == 21 */
	snprintf(buf, sizeof(buf), "%u", grn_table_size(context, table));
	rb_str_cat2(inspected, buf);
    }
    rb_str_cat2(inspected, ">");

    return inspected;
}

/*
 * call-seq:
 *   _table_.inspect -> String
 *
 * テーブルの中身を人に見やすい文字列で返す。
 */
static VALUE
rb_grn_table_inspect (VALUE self)
{
    VALUE inspected;

    inspected = rb_str_new2("");
    rb_grn_object_inspect_header(self, inspected);
    rb_grn_object_inspect_content(self, inspected);
    rb_grn_table_inspect_content(self, inspected);
    rb_grn_object_inspect_footer(self, inspected);

    return inspected;
}

/*
 * call-seq:
 *   table.define_column(name, value_type, options={}) ->
 *     Groonga::FixSizeColumnかGroonga::VariableSizeColumn
 *
 * テーブルに名前が_name_で型が_value_type_のカラムを定義
 * し、新しく定義されたカラムを返す。
 *
 * _options_に指定可能な値は以下の通り。
 *
 * [+:path+]
 *   カラムを保存するパス。
 *
 * [+:persistent+]
 *   +true+を指定すると永続カラムとなる。+:path+を省略した
 *   場合は自動的にパスが付加される。
 *
 * [+:type+]
 *   カラムの値の格納方法について指定する。省略した場合は、
 *   +:scalar+になる。
 *
 *   [+:scalar+]
 *     スカラ値(単独の値)を格納する。
 *
 *   [+:vector+]
 *     値の配列を格納する。
 *
 * [+:compress+]
 *   値の圧縮方法を指定する。省略した場合は、圧縮しない。
 *
 *   [+:zlib+]
 *     値をzlib圧縮して格納する。
 *
 *   [+:lzo+]
 *     値をlzo圧縮して格納する。
 */
static VALUE
rb_grn_table_define_column (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *value_type, *column;
    char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_obj_flags flags = 0;
    VALUE rb_name, rb_value_type;
    VALUE options, rb_path, rb_persistent, rb_compress, rb_type;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rb_scan_args(argc, argv, "21", &rb_name, &rb_value_type, &options);

    name = StringValuePtr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    rb_grn_scan_options(options,
			"path", &rb_path,
			"persistent", &rb_persistent,
			"type", &rb_type,
			"compress", &rb_compress,
			NULL);

    value_type = RVAL2GRNOBJECT(rb_value_type, &context);

    if (!NIL_P(rb_path)) {
	path = StringValueCStr(rb_path);
	flags |= GRN_OBJ_PERSISTENT;
    }

    if (RVAL2CBOOL(rb_persistent))
	flags |= GRN_OBJ_PERSISTENT;

    if (NIL_P(rb_type) ||
	(rb_grn_equal_option(rb_type, "scalar"))) {
	flags |= GRN_OBJ_COLUMN_SCALAR;
    } else if (rb_grn_equal_option(rb_type, "vector")) {
	flags |= GRN_OBJ_COLUMN_VECTOR;
    } else {
	rb_raise(rb_eArgError,
		 "invalid column type: %s: "
		 "available types: [:scalar, :vector, nil]",
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

    column = grn_column_create(context, table, name, name_size,
			       path, flags, value_type);
    rb_grn_context_check(context, self);

    return GRNCOLUMN2RVAL(Qnil, context, column, RB_GRN_TRUE);
}

/*
 * call-seq:
 *   table.define_index_column(name, value_type, options={}) -> Groonga::IndexColumn
 *
 * テーブルに名前が_name_で型が_value_type_のインデックスカ
 * ラムを定義し、新しく定義されたカラムを返す。
 *
 * _options_に指定可能な値は以下の通り。
 *
 * [+:path+]
 *   カラムを保存するパス。
 *
 * [+:persistent+]
 *   +true+を指定すると永続カラムとなる。+:path+を省略した
 *   場合は自動的にパスが付加される。
 *
 * [+:with_section+]
 *   転置索引にsection(段落情報)を合わせて格納する。
 *
 * [+:with_weight+]
 *   転置索引にweight情報を合わせて格納する。
 *
 * [+:with_position+]
 *   転置索引に出現位置情報を合わせて格納する。
 *
 * [+:source+]
 *  インデックス対象となるカラムを指定する。+:sources+との併用はできない。
 *
 * [+:sources+]
 *  インデックス対象となる複数のカラムを指定する。+:source+との併用はできない。
 */
static VALUE
rb_grn_table_define_index_column (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *value_type, *column;
    char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_obj_flags flags = GRN_OBJ_COLUMN_INDEX;
    VALUE rb_name, rb_value_type;
    VALUE options, rb_path, rb_persistent;
    VALUE rb_with_section, rb_with_weight, rb_with_position;
    VALUE rb_column, rb_source, rb_sources;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rb_scan_args(argc, argv, "21", &rb_name, &rb_value_type, &options);

    name = StringValuePtr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    rb_grn_scan_options(options,
			"path", &rb_path,
			"persistent", &rb_persistent,
			"with_section", &rb_with_section,
			"with_weight", &rb_with_weight,
			"with_position", &rb_with_position,
			"source", &rb_source,
			"sources", &rb_sources,
			NULL);

    value_type = RVAL2GRNOBJECT(rb_value_type, &context);

    if (!NIL_P(rb_path)) {
	path = StringValueCStr(rb_path);
	flags |= GRN_OBJ_PERSISTENT;
    }

    if (RVAL2CBOOL(rb_persistent))
	flags |= GRN_OBJ_PERSISTENT;

    if (RVAL2CBOOL(rb_with_section))
	flags |= GRN_OBJ_WITH_SECTION;

    if (RVAL2CBOOL(rb_with_weight))
	flags |= GRN_OBJ_WITH_WEIGHT;

    if (NIL_P(rb_with_position) &&
	(table->header.type == GRN_TABLE_HASH_KEY ||
	 table->header.type == GRN_TABLE_PAT_KEY)) {
	grn_id tokenizer_id;
	grn_obj *tokenizer;

	tokenizer = grn_obj_get_info(context, table,
				     GRN_INFO_DEFAULT_TOKENIZER,
				     NULL);
	tokenizer_id = grn_obj_id(context, tokenizer);
	if ((tokenizer_id == GRN_DB_UNIGRAM) ||
	    (tokenizer_id == GRN_DB_BIGRAM) ||
	    (tokenizer_id == GRN_DB_TRIGRAM)) {
	    rb_with_position = Qtrue;
	}
    }
    if (RVAL2CBOOL(rb_with_position))
	flags |= GRN_OBJ_WITH_POSITION;

    if (!NIL_P(rb_source) && !NIL_P(rb_sources))
	rb_raise(rb_eArgError, "should not pass both of :source and :sources.");

    column = grn_column_create(context, table, name, name_size,
			       path, flags, value_type);
    rb_grn_context_check(context, self);

    rb_column = GRNCOLUMN2RVAL(Qnil, context, column, RB_GRN_TRUE);
    if (!NIL_P(rb_source))
	rb_funcall(rb_column, rb_intern("source="), 1, rb_source);
    if (!NIL_P(rb_sources))
	rb_funcall(rb_column, rb_intern("sources="), 1, rb_sources);

    return rb_column;
}

/*
 * call-seq:
 *  table.add_column(name, value_type, path)
 *
 * _value_type_を値の型として、_path_に保存されている永続的
 * なカラムを、テーブルの_name_に対応するカラムとして開く。
 */
static VALUE
rb_grn_table_add_column (VALUE self, VALUE rb_name, VALUE rb_value_type,
			 VALUE rb_path)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *value_type, *column;
    char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    VALUE rb_column;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    name = StringValuePtr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    value_type = RVAL2GRNOBJECT(rb_value_type, &context);

    path = StringValueCStr(rb_path);

    column = grn_column_open(context, table, name, name_size,
			     path, value_type);
    rb_grn_context_check(context, self);

    rb_column = GRNCOLUMN2RVAL(Qnil, context, column, RB_GRN_TRUE);
    rb_iv_set(rb_column, "table", self);
    return rb_column;
}

static VALUE
rb_grn_table_get_column (VALUE self, VALUE rb_name)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *column;
    char *name = NULL;
    unsigned name_size = 0;
    rb_grn_boolean owner;
    VALUE rb_column;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    switch (TYPE(rb_name)) {
      case T_SYMBOL:
	rb_name = rb_str_new2(rb_id2name(SYM2ID(rb_name)));
	break;
      case T_STRING:
	break;
      default:
	rb_raise(rb_eArgError,
		 "column name should be String or Symbol: %s",
		 rb_grn_inspect(rb_name));
	break;
    }
    name = StringValuePtr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    column = grn_obj_column(context, table, name, name_size);
    rb_grn_context_check(context, self);

    owner = (column && column->header.type == GRN_ACCESSOR);
    rb_column = GRNCOLUMN2RVAL(Qnil, context, column, owner);
    if (owner)
	rb_iv_set(rb_column, "table", self);
    return rb_column;
}

static VALUE
rb_grn_table_get_columns (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *columns;
    grn_rc rc;
    int n;
    grn_table_cursor *cursor;
    VALUE rb_name, rb_columns;
    char *name = NULL;
    unsigned name_size = 0;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rb_scan_args(argc, argv, "01", &rb_name);

    if (!NIL_P(rb_name)) {
	name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
    }

    columns = grn_table_create(context, NULL, 0, NULL, GRN_TABLE_HASH_KEY,
			       NULL, 0);
    n = grn_table_columns(context, table, name, name_size, columns);
    rb_grn_context_check(context, self);

    rb_columns = rb_ary_new2(n);
    if (n == 0)
	return rb_columns;

    cursor = grn_table_cursor_open(context, columns, NULL, 0, NULL, 0,
				   0, -1, GRN_CURSOR_ASCENDING);
    rb_grn_context_check(context, self);
    while (grn_table_cursor_next(context, cursor) != GRN_ID_NIL) {
	void *key;
	grn_id *column_id;
	grn_obj *column;
	VALUE rb_column;

	grn_table_cursor_get_key(context, cursor, &key);
	column_id = key;
	column = grn_ctx_at(context, *column_id);
	rb_column = GRNOBJECT2RVAL(Qnil, context, column, RB_GRN_FALSE);
	rb_ary_push(rb_columns, rb_column);
    }
    rc = grn_table_cursor_close(context, cursor);
    if (rc != GRN_SUCCESS) {
	rb_grn_context_check(context, self);
	rb_grn_rc_check(rc, self);
    }

    return rb_columns;
}

static grn_table_cursor *
rb_grn_table_open_grn_cursor (int argc, VALUE *argv, VALUE self,
			      grn_ctx **context)
{
    grn_obj *table;
    grn_table_cursor *cursor;
    void *min_key = NULL, *max_key = NULL;
    unsigned min_key_size = 0, max_key_size = 0;
    int offset = 0, limit = -1;
    int flags = 0;
    VALUE options, rb_min, rb_max, rb_order, rb_greater_than, rb_less_than;
    VALUE rb_offset, rb_limit;

    rb_grn_table_deconstruct(SELF(self), &table, context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"min", &rb_min,
                        "max", &rb_max,
                        "offset", &rb_offset,
                        "limit", &rb_limit,
			"order", &rb_order,
			"greater_than", &rb_greater_than,
			"less_than", &rb_less_than,
			NULL);

    if (!NIL_P(rb_min)) {
	min_key = StringValuePtr(rb_min);
	min_key_size = RSTRING_LEN(rb_min);
    }
    if (!NIL_P(rb_max)) {
	max_key = StringValuePtr(rb_max);
	max_key_size = RSTRING_LEN(rb_max);
    }
    if (!NIL_P(rb_offset))
	offset = NUM2INT(rb_offset);
    if (!NIL_P(rb_limit))
	limit = NUM2INT(rb_limit);

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

    cursor = grn_table_cursor_open(*context, table,
				   min_key, min_key_size,
				   max_key, max_key_size,
				   offset, limit, flags);
    rb_grn_context_check(*context, self);

    return cursor;
}

/*
 * call-seq:
 *   table.open_cursor(options={}) -> Groonga::TableCursor
 *   table.open_cursor(options={}) {|cursor| ... }
 *
 * カーソルを生成して返す。ブロックを指定すると、そのブロッ
 * クに生成したカーソルが渡され、ブロックを抜けると自動的に
 * カーソルが破棄される。 
 *
 * _options_に指定可能な値は以下の通り。
 *
 * [+:min+]
 *  キーの下限
 *
 * [+:max+]
 *  キーの上限
 *
 * [+:offset+]
 *   該当する範囲のレコードのうち、(0ベースで)_:offset_番目
 *   からレコードを取り出す。
 *
 * [+:limit+]
 *   該当する範囲のレコードのうち、_:limit_件のみを取り出す。
 *
 * [+:order+]
 *   +:asc+または+:ascending+を指定すると昇順にレコードを取
 *   り出す。
 *   +:desc+または+:descending+を指定すると降順にレコードを
 *   取り出す。
 *
 * [+:greater_than+]
 *   +true+を指定すると+:min+で指定した値に一致した[+key+]を
 *   範囲に含まない。
 *
 * [+:less_than+]
 *   +true+を指定すると+:max+で指定した値に一致した[+key+]を
 *   範囲に含まない。
 */
static VALUE
rb_grn_table_open_cursor (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_table_cursor *cursor;
    VALUE rb_cursor;

    cursor = rb_grn_table_open_grn_cursor(argc, argv, self, &context);
    rb_cursor = GRNTABLECURSOR2RVAL(Qnil, context, cursor);
    rb_iv_set(rb_cursor, "@table", self); /* FIXME: cursor should mark table */
    if (rb_block_given_p())
	return rb_ensure(rb_yield, rb_cursor, rb_grn_object_close, rb_cursor);
    else
	return rb_cursor;
}

/*
 * call-seq:
 *   table.records -> Groonga::Recordの配列
 *
 * テーブルに登録されている全てのレコードが入っている配列を
 * 返す。
 */
static VALUE
rb_grn_table_get_records (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_table_cursor *cursor;
    grn_id record_id;
    VALUE records;

    cursor = rb_grn_table_open_grn_cursor(argc, argv, self, &context);
    records = rb_ary_new();
    while ((record_id = grn_table_cursor_next(context, cursor))) {
	rb_ary_push(records, rb_grn_record_new(self, record_id, Qnil));
    }
    grn_table_cursor_close(context, cursor);

    return records;
}

/*
 * call-seq:
 *   table.size -> レコード数
 *
 * テーブルに登録されているレコード数を返す。
 */
static VALUE
rb_grn_table_get_size (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    unsigned int size;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);
    size = grn_table_size(context, table);
    return UINT2NUM(size);
}

/*
 * call-seq:
 *   table.truncate
 *
 * テーブルの全レコードを一括して削除する。
 */
static VALUE
rb_grn_table_truncate (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_rc rc;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);
    rc = grn_table_truncate(context, table);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   table.each {|record| ...}
 *
 * テーブルに登録されているレコードを順番にブロックに渡す。
 */
static VALUE
rb_grn_table_each (VALUE self)
{
    RbGrnTable *rb_table;
    RbGrnObject *rb_grn_object;
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_table_cursor *cursor;
    VALUE rb_cursor;
    grn_id id;

    rb_table = SELF(self);
    rb_grn_table_deconstruct(rb_table, &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);
    cursor = grn_table_cursor_open(context, table, NULL, 0, NULL, 0,
				   0, -1, GRN_CURSOR_ASCENDING);
    rb_cursor = GRNTABLECURSOR2RVAL(Qnil, context, cursor);
    rb_grn_object = RB_GRN_OBJECT(rb_table);
    while (rb_grn_object->object &&
	   (id = grn_table_cursor_next(context, cursor)) != GRN_ID_NIL) {
	rb_yield(rb_grn_record_new(self, id, Qnil));
    }
    rb_grn_object_close(rb_cursor);

    return Qnil;
}

VALUE
rb_grn_table_delete (VALUE self, VALUE rb_id)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_id id;
    grn_rc rc;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    id = NUM2UINT(rb_id);
    rc = grn_table_delete_by_id(context, table, id);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   table.sort(keys, options={}) -> Groonga::Recordの配列
 *
 * テーブルに登録されているレコードを_keys_で指定されたルー
 * ルに従ってソートしたレコードの配列を返す。
 *
 *   [
 *    {:key => "カラム名", :order => :asc, :ascending,
 *                                   :desc, :descendingのいずれか},
 *    {:key => "カラム名", :order => :asc, :ascending,
 *                                   :desc, :descendingのいずれか},
 *    ...,
 *   ]
 *
 * _options_に指定可能な値は以下の通り。
 *
 * [+:offset+]
 *   ソートされたレコードのうち、(0ベースで)_:offset_番目
 *   からレコードを取り出す。
 *
 * [+:limit+]
 *   ソートされたレコードのうち、_:limit_件のみを取り出す。
 */
static VALUE
rb_grn_table_sort (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *result;
    grn_table_sort_key *keys;
    int i, n_keys;
    int n_records, offset = 0, limit = -1;
    VALUE rb_keys, options;
    VALUE rb_offset, rb_limit;
    VALUE *rb_sort_keys;
    grn_table_cursor *cursor;
    VALUE rb_result;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rb_scan_args(argc, argv, "11", &rb_keys, &options);

    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_keys, rb_cArray)))
	rb_raise(rb_eArgError, "keys should be an array of key: <%s>",
		 rb_grn_inspect(rb_keys));

    n_keys = RARRAY_LEN(rb_keys);
    rb_sort_keys = RARRAY_PTR(rb_keys);
    keys = ALLOCA_N(grn_table_sort_key, n_keys);
    for (i = 0; i < n_keys; i++) {
	VALUE rb_sort_options, rb_key, rb_order;

	if (RVAL2CBOOL(rb_obj_is_kind_of(rb_sort_keys[i], rb_cHash))) {
	    rb_sort_options = rb_sort_keys[i];
	} else if (RVAL2CBOOL(rb_obj_is_kind_of(rb_sort_keys[i], rb_cArray))) {
	    rb_sort_options = rb_hash_new();
	    rb_hash_aset(rb_sort_options,
			 RB_GRN_INTERN("key"),
			 rb_ary_entry(rb_sort_keys[i], 0));
	    rb_hash_aset(rb_sort_options,
			 RB_GRN_INTERN("order"),
			 rb_ary_entry(rb_sort_keys[i], 1));
	} else {
	    rb_sort_options = rb_hash_new();
	    rb_hash_aset(rb_sort_options,
			 RB_GRN_INTERN("key"),
			 rb_sort_keys[i]);
	}
	rb_grn_scan_options(rb_sort_options,
			    "key", &rb_key,
			    "order", &rb_order,
			    NULL);
	if (RVAL2CBOOL(rb_obj_is_kind_of(rb_key, rb_cString)))
	    rb_key = rb_grn_table_get_column(self, rb_key);
	keys[i].key = RVAL2GRNOBJECT(rb_key, &context);
	if (NIL_P(rb_order)) {
	    keys[i].flags = 0;
	} else if (rb_grn_equal_option(rb_order, "desc") ||
		   rb_grn_equal_option(rb_order, "descending")) {
	    keys[i].flags = GRN_TABLE_SORT_DESC;
	} else if (rb_grn_equal_option(rb_order, "asc") ||
		   rb_grn_equal_option(rb_order, "ascending")) {
	    keys[i].flags = GRN_TABLE_SORT_ASC;
	} else {
	    rb_raise(rb_eArgError,
		     "order should be one of "
		     "[nil, :desc, :descending, :asc, :ascending]: %s",
		     rb_grn_inspect(rb_order));
	}
    }

    rb_grn_scan_options(options,
			"offset", &rb_offset,
			"limit", &rb_limit,
			NULL);

    if (!NIL_P(rb_offset))
	offset = NUM2INT(rb_offset);
    if (!NIL_P(rb_limit))
	limit = NUM2INT(rb_limit);

    result = grn_table_create(context, NULL, 0, NULL, GRN_TABLE_NO_KEY,
			      NULL, table);
    n_records = grn_table_sort(context, table, offset, limit,
			       result, keys, n_keys);

    rb_result = rb_ary_new();
    cursor = grn_table_cursor_open(context, result, NULL, 0, NULL, 0,
				   0, -1, GRN_CURSOR_ASCENDING);
    while (grn_table_cursor_next(context, cursor) != GRN_ID_NIL) {
	void *value;
	grn_id *id;

	grn_table_cursor_get_value(context, cursor, &value);
	id = value;
	rb_ary_push(rb_result, rb_grn_record_new(self, *id, Qnil));
    }
    grn_table_cursor_close(context, cursor);
    grn_obj_close(context, result);

    rb_grn_context_check(context, self); /* FIXME: here is too late */

    return rb_result;
}

static VALUE
rb_grn_table_group (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_table_sort_key *keys;
    grn_table_group_result *results;
    int i, n_keys, n_results;
    grn_rc rc;
    VALUE rb_keys;
    VALUE *rb_sort_keys;
    VALUE rb_results;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rb_scan_args(argc, argv, "10", &rb_keys);

    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_keys, rb_cArray)))
	rb_raise(rb_eArgError, "keys should be an array of key: <%s>",
		 rb_grn_inspect(rb_keys));

    n_keys = RARRAY_LEN(rb_keys);
    rb_sort_keys = RARRAY_PTR(rb_keys);
    keys = ALLOCA_N(grn_table_sort_key, n_keys);
    for (i = 0; i < n_keys; i++) {
	VALUE rb_sort_options, rb_key;

	if (RVAL2CBOOL(rb_obj_is_kind_of(rb_sort_keys[i], rb_cHash))) {
	    rb_sort_options = rb_sort_keys[i];
	} else {
	    rb_sort_options = rb_hash_new();
	    rb_hash_aset(rb_sort_options,
			 RB_GRN_INTERN("key"),
			 rb_sort_keys[i]);
	}
	rb_grn_scan_options(rb_sort_options,
			    "key", &rb_key,
			    NULL);
	if (RVAL2CBOOL(rb_obj_is_kind_of(rb_key, rb_cString)))
	    rb_key = rb_grn_table_get_column(self, rb_key);
	keys[i].key = RVAL2GRNOBJECT(rb_key, &context);
	keys[i].flags = 0;
    }

    n_results = n_keys;
    results = ALLOCA_N(grn_table_group_result, n_results);
    rb_results = rb_ary_new();
    for (i = 0; i < n_results; i++) {
	grn_obj *result;
	grn_id range_id;
	VALUE rb_result;

	range_id = grn_obj_get_range(context, keys[i].key);
	result = grn_table_create(context, NULL, 0, NULL,
				  GRN_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
				  grn_ctx_at(context, range_id), 0);
	results[i].table = result;
	results[i].key_begin = 0;
	results[i].key_end = 0;
	results[i].limit = 0;
	results[i].flags = 0;
	results[i].op = GRN_OP_OR;

	rb_result = GRNOBJECT2RVAL(Qnil, context, result, RB_GRN_TRUE);
	rb_ary_push(rb_results, rb_result);
    }

    rc = grn_table_group(context, table, keys, n_keys, results, n_results);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    if (n_results == 1)
	return rb_ary_pop(rb_results);
    else
	return rb_results;
}

/*
 * Document-method: []
 *
 * call-seq:
 *   table[id] -> 値
 *
 * _table_の_id_に対応する値を返す。
 */
VALUE
rb_grn_table_array_reference (VALUE self, VALUE rb_id)
{
    grn_id id;
    grn_ctx *context;
    grn_obj *table;
    grn_obj *range;
    grn_obj *value;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     &value, NULL, &range);

    id = NUM2UINT(rb_id);
    GRN_BULK_REWIND(value);
    grn_obj_get_value(context, table, id, value);
    rb_grn_context_check(context, self);

    if (GRN_BULK_EMPTYP(value))
	return Qnil;
    else
	return rb_str_new(GRN_BULK_HEAD(value), GRN_BULK_VSIZE(value));
}

/*
 * Document-method: []=
 *
 * call-seq:
 *   table[id] = 値
 *
 * _table_の_id_に対応する値を設定する。既存の値は上書きさ
 * れる。
 */
VALUE
rb_grn_table_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    grn_id id;
    grn_ctx *context;
    grn_obj *table;
    grn_obj *range;
    grn_obj *value;
    grn_rc rc;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     &value, NULL, &range);

    id = NUM2UINT(rb_id);
    GRN_BULK_REWIND(value);
    RVAL2GRNBULK(rb_value, context, value);
    rc = grn_obj_set_value(context, table, id, value, GRN_OBJ_SET);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * Document-method: unlock
 *
 * call-seq:
 *   table.unlock(options={})
 *
 * _table_のロックを解除する。
 *
 * 利用可能なオプションは以下の通り。
 *
 * [_:id_]
 *   _:id_で指定したレコードのロックを解除する。（注:
 *   groonga側が未実装のため、現在は無視される）
 */
static VALUE
rb_grn_table_unlock (int argc, VALUE *argv, VALUE self)
{
    grn_id id = GRN_ID_NIL;
    grn_ctx *context;
    grn_obj *table;
    grn_rc rc;
    VALUE options, rb_id;

    rb_scan_args(argc, argv, "01",  &options);

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rb_grn_scan_options(options,
			"id", &rb_id,
			NULL);

    if (!NIL_P(rb_id))
	id = NUM2UINT(rb_id);

    rc = grn_obj_unlock(context, table, id);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

static VALUE
rb_grn_table_unlock_ensure (VALUE self)
{
    return rb_grn_table_unlock(0, NULL, self);
}

/*
 * Document-method: lock
 *
 * call-seq:
 *   table.lock(options={})
 *   table.lock(options={}) {...}
 *
 * _table_をロックする。ロックに失敗した場合は
 * Groonga::ResourceDeadlockAvoided例外が発生する。
 *
 * ブロックを指定した場合はブロックを抜けたときにunlockする。
 *
 * 利用可能なオプションは以下の通り。
 *
 * [_:timeout_]
 *   ロックを獲得できなかった場合は_:timeout_秒間ロックの獲
 *   得を試みる。_:timeout_秒以内にロックを獲得できなかった
 *   場合は例外が発生する。
 * [_:id_]
 *   _:id_で指定したレコードをロックする。（注: groonga側が
 *   未実装のため、現在は無視される）
 */
static VALUE
rb_grn_table_lock (int argc, VALUE *argv, VALUE self)
{
    grn_id id = GRN_ID_NIL;
    grn_ctx *context;
    grn_obj *table;
    int timeout = 0;
    grn_rc rc;
    VALUE options, rb_timeout, rb_id;

    rb_scan_args(argc, argv, "01",  &options);

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rb_grn_scan_options(options,
			"timeout", &rb_timeout,
			"id", &rb_id,
			NULL);

    if (!NIL_P(rb_timeout))
	timeout = NUM2UINT(rb_timeout);

    if (!NIL_P(rb_id))
	id = NUM2UINT(rb_id);

    rc = grn_obj_lock(context, table, id, timeout);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    if (rb_block_given_p()) {
	return rb_ensure(rb_yield, Qnil, rb_grn_table_unlock_ensure, self);
    } else {
	return Qnil;
    }
}

/*
 * Document-method: clear_lock
 *
 * call-seq:
 *   table.clear_lock(options={})
 *
 * _table_のロックを強制的に解除する。
 *
 * 利用可能なオプションは以下の通り。
 *
 * [_:id_]
 *   _:id_で指定したレコードのロックを強制的に解除する。
 *   （注: groonga側が未実装のため、現在は無視される。実装さ
 *   れるのではないかと思っているが、実装されないかもしれな
 *   い。）
 */
static VALUE
rb_grn_table_clear_lock (int argc, VALUE *argv, VALUE self)
{
    grn_id id = GRN_ID_NIL;
    grn_ctx *context;
    grn_obj *table;
    VALUE options, rb_id;

    rb_scan_args(argc, argv, "01",  &options);

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rb_grn_scan_options(options,
			"id", &rb_id,
			NULL);

    if (!NIL_P(rb_id))
	id = NUM2UINT(rb_id);

    grn_obj_clear_lock(context, table);

    return Qnil;
}

/*
 * Document-method: locked?
 *
 * call-seq:
 *   table.locked?(options={})
 *
 * _table_がロックされていれば+true+を返す。
 *
 * 利用可能なオプションは以下の通り。
 *
 * [_:id_]
 *   _:id_で指定したレコードがロックされていれば+true+を返す。
 *   （注: groonga側が未実装のため、現在は無視される。実装さ
 *   れるのではないかと思っているが、実装されないかもしれな
 *   い。）
 */
static VALUE
rb_grn_table_is_locked (int argc, VALUE *argv, VALUE self)
{
    grn_id id = GRN_ID_NIL;
    grn_ctx *context;
    grn_obj *table;
    VALUE options, rb_id;

    rb_scan_args(argc, argv, "01",  &options);

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rb_grn_scan_options(options,
			"id", &rb_id,
			NULL);

    if (!NIL_P(rb_id))
	id = NUM2UINT(rb_id);

    return CBOOL2RVAL(grn_obj_is_locked(context, table));
}

/*
 * call-seq:
 *   table.select(options) {|record| ...} -> Groonga::Hash
 *   table.select(query, options) -> Groonga::Hash
 *   table.select(expression, options) -> Groonga::Hash
 *
 * _table_からブロックまたは文字列で指定した条件にマッチする
 * レコードを返す。返されたテーブルには+expression+という特
 * 異メソッドがあり、指定した条件を表している
 * Groonga::Expressionを取得できる。
 * Groonga::Expression#snippetを使うことにより、指定した条件
 * 用のスニペットを簡単に生成できる。
 *
 *   results = table.select do |record|
 *     record["description"] =~ "groonga"
 *   end
 *   snippet = results.expression.snippet([["<em>", "</em>"]])
 *   results.each do |record|
 *     puts "#{record['name']}の説明文の中で「groonga」が含まれる部分"
 *     snippet.execute(record["description"].each do |snippet|
 *       puts "---"
 *       puts "#{snippet}..."
 *       puts "---"
 *     end
 *   end
 *
 * 出力例
 *   Ruby/groongaの説明文の中で「groonga」が含まれる部分
 *   ---
 *   Ruby/<em>groonga</em>は<em>groonga</em>のいわゆるDB-APIの層の...
 *   ---
 *
 * _query_には「[カラム名]:[演算子][値]」という書式で条件を
 * 指定する。演算子は以下の通り。
 *
 * [なし]
 *   [カラム値] == [値]
 * [<tt>!</tt>]
 *   [カラム値] != [値]
 * [<tt><</tt>]
 *   [カラム値] < [値]
 * [<tt>></tt>]
 *   [カラム値] > [値]
 * [<tt><=</tt>]
 *   [カラム値] <= [値]
 * [<tt>>=</tt>]
 *   [カラム値] >= [値]
 * [<tt>@</tt>]
 *   [カラム値]が[値]を含んでいるかどうか
 *
 * 例:
 *   "name:daijiro" # "name"カラムの値が"daijiro"のレコードにマッチ
 *   "description:@groonga" # "description"カラムが
 *                          # "groonga"を含んでいるレコードにマッチ
 *
 * _expression_には既に作成済みのGroonga::Expressionを渡す
 *
 * ブロックで条件を指定する場合は
 * Groonga::RecordExpressionBuilderを参照。
 *
 * _options_に指定可能な値は以下の通り。
 *
 * [+:operator+]
 *   マッチしたレコードをどのように扱うか。指定可能な値は以
 *   下の通り。省略した場合はGroonga::Operation::OR。
 *
 *   [Groonga::Operation::OR]
 *     マッチしたレコードを追加。すでにレコードが追加され
 *     ている場合は何もしない。
 *   [Groonga::Operation::AND]
 *     マッチしたレコードのスコアを増加。マッチしなかった
 *     レコードを削除。
 *   [Groonga::Operation::BUT]
 *     マッチしたレコードを削除。
 *   [Groonga::Operation::ADJUST]
 *     マッチしたレコードのスコアを増加。
 *
 * [+:result+]
 *   検索結果を格納するテーブル。マッチしたレコードが追加さ
 *   れていく。省略した場合は新しくテーブルを作成して返す。
 *
 * [+:name+]
 *   条件の名前。省略した場合は名前を付けない。
 *
 * [+:parser+]
 *   _query_をパースする時に使用するパーサー。省略した場合は
 *   +:query+。
 */
static VALUE
rb_grn_table_select (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *table, *result, *expression;
    grn_operator operator = GRN_OP_OR;
    VALUE rb_query = Qnil, condition_or_options, options;
    VALUE rb_name, rb_operator, rb_result, rb_parser = Qnil;
    VALUE rb_expression = Qnil, builder;

    rb_scan_args(argc, argv, "02", &condition_or_options, &options);

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);

    if (RVAL2CBOOL(rb_obj_is_kind_of(condition_or_options, rb_cString))) {
	rb_query = condition_or_options;
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(condition_or_options,
					    rb_cGrnExpression))) {
	rb_expression = condition_or_options;
    } else {
	if (!NIL_P(options))
	    rb_raise(rb_eArgError,
		     "should be [query_string, option_hash], "
		     "[expression, opion_hash] "
		     "or [option_hash]: %s",
		     rb_grn_inspect(rb_ary_new4(argc, argv)));
	options = condition_or_options;
    }

    rb_grn_scan_options(options,
			"operator", &rb_operator,
			"result", &rb_result,
			"name", &rb_name,
			"parser", &rb_parser,
			NULL);

    if (!NIL_P(rb_operator))
	operator = NUM2INT(rb_operator);

    if (NIL_P(rb_result)) {
	result = grn_table_create(context, NULL, 0, NULL,
				  GRN_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
				  table,
				  0);
	rb_result = GRNTABLE2RVAL(context, result, RB_GRN_TRUE);
    } else {
	result = RVAL2GRNTABLE(rb_result, &context);
    }

    if (NIL_P(rb_expression)) {
      builder = rb_grn_record_expression_builder_new(self, rb_name);
      rb_funcall(builder, rb_intern("query="), 1, rb_query);
      rb_funcall(builder, rb_intern("parser="), 1, rb_parser);
      rb_expression = rb_grn_record_expression_builder_build(builder);
    }
    rb_grn_object_deconstruct(RB_GRN_OBJECT(DATA_PTR(rb_expression)),
                              &expression, NULL,
			      NULL, NULL, NULL, NULL);

    grn_table_select(context, table, expression, result, operator);
    rb_grn_context_check(context, self);

    rb_attr(rb_singleton_class(rb_result),
	    rb_intern("expression"),
	    RB_GRN_TRUE, RB_GRN_FALSE, RB_GRN_FALSE);
    rb_iv_set(rb_result, "@expression", rb_expression);

    return rb_result;
}

static VALUE
rb_grn_table_set_operation_bang (VALUE self, VALUE rb_other,
				 grn_operator operator)
{
    grn_ctx *context;
    grn_obj *table, *other;
    grn_rc rc;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL);
    rb_grn_table_deconstruct(SELF(rb_other), &other, NULL,
			     NULL, NULL,
			     NULL, NULL, NULL);

    rc = grn_table_setoperation(context, table, other, table, operator);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return self;
}

static VALUE
rb_grn_table_union_bang (VALUE self, VALUE rb_other)
{
    return rb_grn_table_set_operation_bang(self ,rb_other, GRN_OP_OR);
}

static VALUE
rb_grn_table_intersection_bang (VALUE self, VALUE rb_other)
{
    return rb_grn_table_set_operation_bang(self ,rb_other, GRN_OP_AND);
}

static VALUE
rb_grn_table_difference_bang (VALUE self, VALUE rb_other)
{
    return rb_grn_table_set_operation_bang(self ,rb_other, GRN_OP_BUT);
}

static VALUE
rb_grn_table_merge_bang (VALUE self, VALUE rb_other)
{
    return rb_grn_table_set_operation_bang(self ,rb_other, GRN_OP_ADJUST);
}

void
rb_grn_init_table (VALUE mGrn)
{
    rb_cGrnTable = rb_define_class_under(mGrn, "Table", rb_cGrnObject);
    rb_define_alloc_func(rb_cGrnTable, rb_grn_table_alloc);

    rb_include_module(rb_cGrnTable, rb_mEnumerable);

    rb_define_singleton_method(rb_cGrnTable, "open",
			       rb_grn_table_s_open, -1);

    rb_define_method(rb_cGrnTable, "initialize", rb_grn_table_initialize, -1);

    rb_define_method(rb_cGrnTable, "inspect", rb_grn_table_inspect, 0);

    rb_define_method(rb_cGrnTable, "define_column",
		     rb_grn_table_define_column, -1);
    rb_define_method(rb_cGrnTable, "define_index_column",
		     rb_grn_table_define_index_column, -1);
    rb_define_method(rb_cGrnTable, "add_column",
		     rb_grn_table_add_column, 3);
    rb_define_method(rb_cGrnTable, "column",
		     rb_grn_table_get_column, 1);
    rb_define_method(rb_cGrnTable, "columns",
		     rb_grn_table_get_columns, -1);

    rb_define_method(rb_cGrnTable, "open_cursor", rb_grn_table_open_cursor, -1);
    rb_define_method(rb_cGrnTable, "records", rb_grn_table_get_records, -1);

    rb_define_method(rb_cGrnTable, "size", rb_grn_table_get_size, 0);
    rb_define_method(rb_cGrnTable, "truncate", rb_grn_table_truncate, 0);

    rb_define_method(rb_cGrnTable, "each", rb_grn_table_each, 0);

    rb_define_method(rb_cGrnTable, "delete", rb_grn_table_delete, 1);

    rb_define_method(rb_cGrnTable, "sort", rb_grn_table_sort, -1);
    rb_define_method(rb_cGrnTable, "group", rb_grn_table_group, -1);

    rb_define_method(rb_cGrnTable, "[]", rb_grn_table_array_reference, 1);
    rb_define_method(rb_cGrnTable, "[]=", rb_grn_table_array_set, 2);

    rb_define_method(rb_cGrnTable, "lock", rb_grn_table_lock, -1);
    rb_define_method(rb_cGrnTable, "unlock", rb_grn_table_unlock, -1);
    rb_define_method(rb_cGrnTable, "clear_lock", rb_grn_table_clear_lock, -1);
    rb_define_method(rb_cGrnTable, "locked?", rb_grn_table_is_locked, -1);

    rb_define_method(rb_cGrnTable, "select", rb_grn_table_select, -1);

    rb_define_method(rb_cGrnTable, "union!", rb_grn_table_union_bang, 1);
    rb_define_method(rb_cGrnTable, "intersection!",
		     rb_grn_table_intersection_bang, 1);
    rb_define_method(rb_cGrnTable, "difference!",
		     rb_grn_table_difference_bang, 1);
    rb_define_method(rb_cGrnTable, "merge!",
		     rb_grn_table_merge_bang, 1);

    rb_grn_init_table_key_support(mGrn);
    rb_grn_init_array(mGrn);
    rb_grn_init_hash(mGrn);
    rb_grn_init_patricia_trie(mGrn);
}
