/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009-2012  Kouhei Sutou <kou@clear-code.com>

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

grn_obj *grn_table_open(grn_ctx *ctx,
                        const char *name, unsigned name_size, const char *path);
grn_obj *grn_column_open(grn_ctx *ctx, grn_obj *table,
                         const char *name, unsigned name_size,
                         const char *path, grn_obj *type);

#define SELF(object) ((RbGrnTable *)DATA_PTR(object))

VALUE rb_cGrnTable;

static ID id_array_reference;
static ID id_array_set;

/*
 * Document-class: Groonga::Table < Groonga::Object
 *
 * rroongaが提供するテーブルのベースとなるクラス。このクラス
 * からGroonga::Array, Groonga::Hash, Groonga::PatriciaTrie
 * が継承されている。
 */

grn_obj *
rb_grn_table_from_ruby_object (VALUE object, grn_ctx **context)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnTable))) {
	rb_raise(rb_eTypeError,
		 "not a groonga table: <%s>",
		 rb_grn_inspect(object));
    }

    return RVAL2GRNOBJECT(object, context);
}

VALUE
rb_grn_table_to_ruby_object (grn_ctx *context, grn_obj *table,
			     grn_bool owner)
{
    return GRNOBJECT2RVAL(Qnil, context, table, owner);
}

void
rb_grn_table_finalizer (grn_ctx *context, grn_obj *object,
			RbGrnTable *rb_grn_table)
{
    if (context && rb_grn_table->value)
	grn_obj_unlink(context, rb_grn_table->value);
    rb_grn_table->value = NULL;
    rb_grn_table->columns = Qnil;
}

void
rb_grn_table_bind (RbGrnTable *rb_grn_table,
		   grn_ctx *context, grn_obj *table)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_table);
    rb_grn_table->value = grn_obj_open(context, GRN_BULK, 0,
				       rb_grn_object->range_id);
    rb_grn_table->columns = Qnil;
    rb_grn_table->columns = rb_ary_new();
}

void
rb_grn_table_deconstruct (RbGrnTable *rb_grn_table,
			  grn_obj **table,
			  grn_ctx **context,
			  grn_id *domain_id,
			  grn_obj **domain,
			  grn_obj **value,
			  grn_id *range_id,
			  grn_obj **range,
			  VALUE *columns)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_table);
    rb_grn_object_deconstruct(rb_grn_object, table, context,
			      domain_id, domain,
			      range_id, range);

    if (value)
	*value = rb_grn_table->value;
    if (columns)
	*columns = rb_grn_table->columns;
}

static void
rb_grn_table_mark (void *data)
{
    RbGrnObject *rb_grn_object = data;
    RbGrnTable *rb_grn_table = data;
    grn_ctx *context;
    grn_obj *table;

    if (!rb_grn_object)
	return;

    rb_gc_mark(rb_grn_table->columns);

    context = rb_grn_object->context;
    table = rb_grn_object->object;
    if (!context || !table)
	return;

    if (!grn_obj_path(context, table))
	return;

    if (grn_obj_name(context, table, NULL, 0) == 0)
	return;
}

static VALUE
rb_grn_table_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, rb_grn_table_mark, rb_grn_object_free, NULL);
}

static VALUE
rb_grn_table_initialize (VALUE self)
{
    rb_raise(rb_eArgError, "Use Groonga::Context#[] for get existing table.");
    return Qnil;
}

static VALUE
rb_grn_table_inspect_content (VALUE self, VALUE inspected)
{
    RbGrnTable *rb_grn_table;
    grn_ctx *context = NULL;
    grn_obj *table;
    VALUE columns;

    rb_grn_table = SELF(self);
    if (!rb_grn_table)
	return inspected;

    rb_grn_table_deconstruct(rb_grn_table, &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     &columns);

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
	grn_obj_unlink(context, &value);

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

    /*
    rb_str_cat2(inspected, ", ");
    rb_str_cat2(inspected, "columns: <");
    rb_str_concat(inspected, rb_inspect(columns));
    rb_str_cat2(inspected, ">");
    */

    return inspected;
}

/*
 * call-seq:
 *   table.inspect -> String
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
 * テーブルに名前が _name_ で型が _value_type_ のカラムを定義
 * し、新しく定義されたカラムを返す。
 *
 * _options_ に指定可能な値は以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :path The path
 *   カラムを保存するパス。
 *
 * @option options :persistent (永続カラム) The persistent
 *   +true+ を指定すると永続カラムとなる。省略した場合は永
 *   続カラムとなる。 +:path+ を省略した場合は自動的にパスが
 *   付加される。
 *
 * @option options :type (:scalar) The type
 *   カラムの値の格納方法について指定する。省略した場合は、
 *   +:scalar+ になる。
 *
 *   [+:scalar+] スカラ値(単独の値)を格納する。
 *   [+:vector+] 値の配列を格納する。
 *
 * @param options :compress The compress
 *   値の圧縮方法を指定する。省略した場合は、圧縮しない。
 *
 *   [+:zlib+] 値をzlib圧縮して格納する。
 *   [+:lzo+] 値をlzo圧縮して格納する。
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
    VALUE columns;
    VALUE rb_column;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     &columns);

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

    if ((NIL_P(rb_persistent) && grn_obj_path(context, table)) ||
	RVAL2CBOOL(rb_persistent)) {
	flags |= GRN_OBJ_PERSISTENT;
    }

    if (!NIL_P(rb_path)) {
	path = StringValueCStr(rb_path);
	if ((flags & GRN_OBJ_PERSISTENT) != GRN_OBJ_PERSISTENT) {
	    rb_raise(rb_eArgError,
		     "should not pass :path if :persistent is false: <%s>",
		     path);
	}
	flags |= GRN_OBJ_PERSISTENT;
    }

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
    if (context->rc) {
	rb_grn_context_check(context,
			     rb_ary_new3(2, self, rb_ary_new4(argc, argv)));
    }

    rb_column = GRNCOLUMN2RVAL(Qnil, context, column, GRN_TRUE);
    rb_ary_push(columns, rb_column);
    rb_grn_named_object_set_name(RB_GRN_NAMED_OBJECT(DATA_PTR(rb_column)),
				 name, name_size);

    return rb_column;
}

static grn_bool
n_gram_tokenizer_p(grn_ctx *context, grn_obj *tokenizer)
{
    char tokenizer_name[GRN_TABLE_MAX_KEY_SIZE];
    int name_size;

    name_size = grn_obj_name(context, tokenizer,
			     tokenizer_name, sizeof(tokenizer_name) - 1);
    if (name_size == 0)
	return GRN_FALSE;

    tokenizer_name[name_size] = '\0';

#define HAVE_PREFIX_P(prefix) \
    (strncmp(tokenizer_name, prefix, strlen(prefix)) == 0)
    return (HAVE_PREFIX_P("TokenUnigram") ||
	    HAVE_PREFIX_P("TokenBigram") ||
	    HAVE_PREFIX_P("TokenTrigram"));
#undef HAVE_PREFIX_P
}

/*
 * call-seq:
 *   table.define_index_column(name, value_type, options={}) -> Groonga::IndexColumn
 *
 * テーブルに名前が _name_ で型が _value_type_ のインデックスカ
 * ラムを定義し、新しく定義されたカラムを返す。
 *
 * _options_ に指定可能な値は以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :path The path
 *   カラムを保存するパス。
 *
 * @option options :persistent (永続カラム) The persistent
 *   +true+ を指定すると永続カラムとなる。省略した場合は永
 *   続カラムとなる。 +:path+ を省略した場合は自動的にパスが
 *   付加される。
 *
 * @option options :with_section The with_section
 *   転置索引にsection(段落情報)を合わせて格納する。
 *
 * @option options :with_weight  The with_weight
 *   転置索引にweight情報を合わせて格納する。
 *
 * @option options :with_position The with_position
 *   転置索引に出現位置情報を合わせて格納する。
 *
 * @option options :source The source
 *  インデックス対象となるカラムを指定する。 +:sources+ との併用はできない。
 *
 * @option options :sources The sources
 *  インデックス対象となる複数のカラムを指定する。 +:source+ との併用はできない。
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
    VALUE columns;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     &columns);

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

    if ((NIL_P(rb_persistent) && grn_obj_path(context, table)) ||
	RVAL2CBOOL(rb_persistent)) {
	flags |= GRN_OBJ_PERSISTENT;
    }

    if (!NIL_P(rb_path)) {
	path = StringValueCStr(rb_path);
	if ((flags & GRN_OBJ_PERSISTENT) != GRN_OBJ_PERSISTENT) {
	    rb_raise(rb_eArgError,
		     "should not pass :path if :persistent is false: <%s>",
		     path);
	}
	flags |= GRN_OBJ_PERSISTENT;
    }

    if (NIL_P(rb_with_section)) {
	if (TYPE(rb_sources) == T_ARRAY && RARRAY_LEN(rb_sources) > 1) {
	    flags |= GRN_OBJ_WITH_SECTION;
	}
    } else if (RVAL2CBOOL(rb_with_section)) {
	flags |= GRN_OBJ_WITH_SECTION;
    }


    if (RVAL2CBOOL(rb_with_weight))
	flags |= GRN_OBJ_WITH_WEIGHT;

    if (NIL_P(rb_with_position) &&
	(table->header.type == GRN_TABLE_HASH_KEY ||
	 table->header.type == GRN_TABLE_PAT_KEY)) {
	grn_obj *tokenizer;

	tokenizer = grn_obj_get_info(context, table,
				     GRN_INFO_DEFAULT_TOKENIZER,
				     NULL);
	if (tokenizer && n_gram_tokenizer_p(context, tokenizer)) {
	    rb_with_position = Qtrue;
	}
    }
    if (RVAL2CBOOL(rb_with_position))
	flags |= GRN_OBJ_WITH_POSITION;

    if (!NIL_P(rb_source) && !NIL_P(rb_sources))
	rb_raise(rb_eArgError, "should not pass both of :source and :sources.");

    column = grn_column_create(context, table, name, name_size,
			       path, flags, value_type);
    if (context->rc) {
	rb_grn_context_check(context,
			     rb_ary_new3(2, self, rb_ary_new4(argc, argv)));
    }

    rb_column = GRNCOLUMN2RVAL(Qnil, context, column, GRN_TRUE);
    if (!NIL_P(rb_source))
	rb_funcall(rb_column, rb_intern("source="), 1, rb_source);
    if (!NIL_P(rb_sources))
	rb_funcall(rb_column, rb_intern("sources="), 1, rb_sources);

    rb_ary_push(columns, rb_column);
    rb_grn_named_object_set_name(RB_GRN_NAMED_OBJECT(DATA_PTR(rb_column)),
				 name, name_size);

    return rb_column;
}

static void
ruby_object_to_column_name (VALUE rb_name,
			    const char **name, unsigned *name_size)
{
    switch (TYPE(rb_name)) {
      case T_SYMBOL:
	*name = rb_id2name(SYM2ID(rb_name));
	*name_size = strlen(*name);
	break;
      case T_STRING:
	*name = StringValuePtr(rb_name);
	*name_size = RSTRING_LEN(rb_name);
	break;
      default:
	rb_raise(rb_eArgError,
		 "column name should be String or Symbol: %s",
		 rb_grn_inspect(rb_name));
	break;
    }
}

/*
 * call-seq:
 *   table.column(name) -> Groonga::Column or nil
 *
 * テーブルの _name_ に対応するカラムを返す。カラムが存在しな
 * い場合は +nil+ を返す。
 */
VALUE
rb_grn_table_get_column (VALUE self, VALUE rb_name)
{
    grn_user_data *user_data;
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *column;
    const char *name = NULL;
    unsigned name_size = 0;
    grn_bool owner;
    VALUE rb_column;
    VALUE columns;
    VALUE *raw_columns;
    long i, n;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     &columns);

    ruby_object_to_column_name(rb_name, &name, &name_size);
    raw_columns = RARRAY_PTR(columns);
    n = RARRAY_LEN(columns);
    for (i = 0; i < n; i++) {
	VALUE rb_column = raw_columns[i];
	RbGrnNamedObject *rb_grn_named_object;

	rb_grn_named_object = RB_GRN_NAMED_OBJECT(DATA_PTR(rb_column));
	if (name_size == rb_grn_named_object->name_size &&
	    memcmp(name, rb_grn_named_object->name, name_size) == 0) {
	    return rb_column;
	}
    }

    column = grn_obj_column(context, table, name, name_size);
    rb_grn_context_check(context, self);
    if (!column)
	return Qnil;

    user_data = grn_obj_user_data(context, column);
    if (user_data) {
	RbGrnObject *rb_grn_object;
	rb_grn_object = user_data->ptr;
	if (rb_grn_object) {
	    rb_ary_push(columns, rb_grn_object->self);
	    return rb_grn_object->self;
	}
    }

    owner = column->header.type == GRN_ACCESSOR;
    rb_column = GRNCOLUMN2RVAL(Qnil, context, column, owner);
    if (owner) {
	rb_iv_set(rb_column, "table", self);
    }
    rb_grn_named_object_set_name(RB_GRN_NAMED_OBJECT(DATA_PTR(rb_column)),
				 name, name_size);

    return rb_column;
}

VALUE
rb_grn_table_get_column_surely (VALUE self, VALUE rb_name)
{
    VALUE rb_column;

    rb_column = rb_grn_table_get_column(self, rb_name);
    if (NIL_P(rb_column)) {
	rb_raise(rb_eGrnNoSuchColumn,
		 "no such column: <%s>: <%s>",
		 rb_grn_inspect(rb_name), rb_grn_inspect(self));
    }
    return rb_column;
}

/*
 * call-seq:
 *   table.columns(name=nil) -> Groonga::Columnの配列
 *
 * テーブルの全てのカラムを返す。 _name_ が指定された場合はカ
 * ラム名の先頭が _name_ で始まるカラムを返す。
 */
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
			     NULL, NULL, NULL,
			     NULL);

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
    if (n == 0) {
	grn_obj_unlink(context, columns);
	return rb_columns;
    }

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
	rb_column = GRNOBJECT2RVAL(Qnil, context, column, GRN_FALSE);
	rb_ary_push(rb_columns, rb_column);
    }
    rc = grn_table_cursor_close(context, cursor);
    grn_obj_unlink(context, columns);
    if (rc != GRN_SUCCESS) {
	rb_grn_context_check(context, self);
	rb_grn_rc_check(rc, self);
    }

    return rb_columns;
}

/*
 * call-seq:
 *   table.have_column?(name) -> true/false
 *
 * テーブルが _name_ カラムを持っている場合は +true+ を返す。
 */
static VALUE
rb_grn_table_have_column (VALUE self, VALUE rb_name)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *column;
    const char *name = NULL;
    unsigned name_size = 0;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);

    ruby_object_to_column_name(rb_name, &name, &name_size);
    column = grn_obj_column(context, table, name, name_size);
    if (column) {
	grn_obj_unlink(context, column);
	return Qtrue;
    } else {
	return Qfalse;
    }
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
    VALUE options, rb_min, rb_max, rb_order, rb_order_by;
    VALUE rb_greater_than, rb_less_than, rb_offset, rb_limit;

    rb_grn_table_deconstruct(SELF(self), &table, context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"min", &rb_min,
                        "max", &rb_max,
                        "offset", &rb_offset,
                        "limit", &rb_limit,
			"order", &rb_order,
			"order_by", &rb_order_by,
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

    flags |= rb_grn_table_cursor_order_to_flag(rb_order);
    flags |= rb_grn_table_cursor_order_by_to_flag(table->header.type,
						  self,
						  rb_order_by);

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
 *   table.open_cursor(options={}) {|cursor| }
 *
 * カーソルを生成して返す。ブロックを指定すると、そのブロッ
 * クに生成したカーソルが渡され、ブロックを抜けると自動的に
 * カーソルが破棄される。
 *
 * _options_に指定可能な値は以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :min The min
 *  キーの下限
 *
 * @option  options :max The max
 *  キーの上限
 *
 * @option options :offset The offset
 *   該当する範囲のレコードのうち、(0ベースで) _:offset_ 番目
 *   からレコードを取り出す。
 *
 * @option options :limit The limit
 *   該当する範囲のレコードのうち、 _:limit_ 件のみを取り出す。
 *   省略された場合または-1が指定された場合は、全件が指定され
 *   たものとみなす。
 *
 * @option options :order (asc) The order
 *   +:asc+ または +:ascending+ を指定すると昇順にレコードを取
 *   り出す。（デフォルト）
 *
 *   +:desc+ または +:descending+ を指定すると降順にレコードを
 *   取り出す。
 *
 * @option options :order_by The order_by
 *   +:id+ を指定するとID順にレコードを取り出す。（Arrayと
 *   Hashのデフォルト）
 *
 *   +:key+ 指定するとキー順にレコードを取り出す。ただし、
 *   Groonga::PatriciaTrieにしか使えない。（PatriciaTrieのデ
 *   フォルト）
 *
 * @option options :greater_than The greater_than
 *   +true+ を指定すると +:min+ で指定した値に一致した [ +key+ ] を
 *   範囲に含まない。
 *
 * @option options :less_than The less_than
 *   +true+ を指定すると +:max+ で指定した値に一致した [ +key+ ] を
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
			     NULL, NULL, NULL,
			     NULL);
    size = grn_table_size(context, table);
    return UINT2NUM(size);
}

/*
 * call-seq:
 *   table.empty? -> true/false
 *
 * テーブルにレコードが登録されていなければ +true+ を返す。
 */
static VALUE
rb_grn_table_empty_p (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    unsigned int size;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);
    size = grn_table_size(context, table);
    return CBOOL2RVAL(size == 0);
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
			     NULL, NULL, NULL,
			     NULL);
    rc = grn_table_truncate(context, table);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   table.each {|record| } -> nil
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
			     NULL, NULL, NULL,
			     NULL);
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

/*
 * call-seq:
 *   table.delete(id)
 *
 * テーブルの _id_ に対応するレコードを削除する。
 */
VALUE
rb_grn_table_delete (VALUE self, VALUE rb_id)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_id id;
    grn_rc rc;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);

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
 * _order_ には +:asc+ ,  +:ascending+ , +:desc+ , +:descending+ の
 * いずれを指定する。
 *
 * - ハッシュの配列で指定する方法 :=
 * オーソドックスな指定方法。
 *
 * <pre>
 * !!!ruby
 * [
 *  {:key => "第1ソートキー", :order => order},
 *  {:key => "第2ソートキー", :order => order},
 *  # ...,
 * ]
 * </pre>
 * =:
 *
 * - 配列の配列で指定する方法 :=
 * 少し簡単化した指定方法。
 *
 * <pre>
 * !!!ruby
 * [
 *  ["第1ソートキー", order],
 *  ["第2ソートキー", order],
 *  # ...,
 * ]
 * </pre>
 * =:
 *
 * - ソートキーの配列で指定する方法 :=
 * _order_ は常に昇順（ +:ascending+ ）になるが、最も簡単
 * に指定できる。
 *
 * <pre>
 * !!!ruby
 * ["第1ソートキー", "第2ソートキー", "..."]
 * </pre>
 * =:
 *
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :offset The offset
 *   ソートされたレコードのうち、(0ベースで) _:offset_ 番目
 *   からレコードを取り出す。
 *
 * @option options :limit The limit
 *   ソートされたレコードのうち、 _:limit_ 件のみを取り出す。
 *   省略された場合または-1が指定された場合は、全件が指定され
 *   たものとみなす。
 */
static VALUE
rb_grn_table_sort (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_obj *result;
    grn_table_sort_key *keys;
    int i, n_keys;
    int offset = 0, limit = -1;
    VALUE rb_keys, options;
    VALUE rb_offset, rb_limit;
    VALUE *rb_sort_keys;
    grn_table_cursor *cursor;
    VALUE rb_result;
    VALUE exception;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);

    rb_scan_args(argc, argv, "11", &rb_keys, &options);

    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_keys, rb_cArray)))
	rb_raise(rb_eArgError, "keys should be an array of key: <%s>",
		 rb_grn_inspect(rb_keys));

    n_keys = RARRAY_LEN(rb_keys);
    rb_sort_keys = RARRAY_PTR(rb_keys);
    keys = ALLOCA_N(grn_table_sort_key, n_keys);
    for (i = 0; i < n_keys; i++) {
	VALUE rb_sort_options, rb_key, rb_resolved_key, rb_order;

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
	if (RVAL2CBOOL(rb_obj_is_kind_of(rb_key, rb_cString))) {
	    rb_resolved_key = rb_grn_table_get_column(self, rb_key);
	} else {
	    rb_resolved_key = rb_key;
	}
	keys[i].key = RVAL2GRNOBJECT(rb_resolved_key, &context);
	if (!keys[i].key) {
	    rb_raise(rb_eGrnNoSuchColumn,
		     "no such column: <%s>: <%s>",
		     rb_grn_inspect(rb_key), rb_grn_inspect(self));
	}
	if (NIL_P(rb_order) ||
	    rb_grn_equal_option(rb_order, "asc") ||
	    rb_grn_equal_option(rb_order, "ascending")) {
	    keys[i].flags = GRN_TABLE_SORT_ASC;
	} else if (rb_grn_equal_option(rb_order, "desc") ||
		   rb_grn_equal_option(rb_order, "descending")) {
	    keys[i].flags = GRN_TABLE_SORT_DESC;
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
    /* use n_records that is return value from
       grn_table_sort() when rroonga user become specifying
       output table. */
    grn_table_sort(context, table, offset, limit, result, keys, n_keys);
    exception = rb_grn_context_to_exception(context, self);
    if (!NIL_P(exception)) {
        grn_obj_unlink(context, result);
        rb_exc_raise(exception);
    }

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
    grn_obj_unlink(context, result);

    return rb_result;
}

/*
 * call-seq:
 *   table.group([key1, key2, ...], options={}) -> [Groonga::Hash, ...]
 *   table.group(key, options={}) -> Groonga::Hash
 *
 * _table_ のレコードを _key1_ , _key2_ , _..._ で指定したキーの
 * 値でグループ化する。多くの場合、キーにはカラムを指定する。
 * カラムはカラム名（文字列）でも指定可能。
 */
static VALUE
rb_grn_table_group (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_table_sort_key *keys;
    grn_table_group_result *results;
    int i, n_keys, n_results;
    grn_rc rc;
    VALUE rb_keys, rb_options;
    VALUE *rb_group_keys;
    VALUE rb_results;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);

    rb_scan_args(argc, argv, "11", &rb_keys, &rb_options);

    if (TYPE(rb_keys) == T_ARRAY) {
	n_keys = RARRAY_LEN(rb_keys);
	rb_group_keys = RARRAY_PTR(rb_keys);
    } else {
	n_keys = 1;
	rb_group_keys = &rb_keys;
    }

    keys = ALLOCA_N(grn_table_sort_key, n_keys);
    for (i = 0; i < n_keys; i++) {
	VALUE rb_sort_options, rb_key;

	if (RVAL2CBOOL(rb_obj_is_kind_of(rb_group_keys[i], rb_cHash))) {
	    rb_sort_options = rb_group_keys[i];
	} else {
	    rb_sort_options = rb_hash_new();
	    rb_hash_aset(rb_sort_options,
			 RB_GRN_INTERN("key"),
			 rb_group_keys[i]);
	}
	rb_grn_scan_options(rb_sort_options,
			    "key", &rb_key,
			    NULL);
	if (RVAL2CBOOL(rb_obj_is_kind_of(rb_key, rb_cString))) {
	    VALUE resolved_rb_key;
	    resolved_rb_key = rb_grn_table_get_column(self, rb_key);
	    if (NIL_P(resolved_rb_key)) {
		rb_raise(rb_eArgError,
			 "unknown group key: <%s>: <%s>",
			 rb_grn_inspect(rb_key),
			 rb_grn_inspect(self));
	    }
	    rb_key = resolved_rb_key;
	}
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

	rb_result = GRNOBJECT2RVAL(Qnil, context, result, GRN_TRUE);
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
 *   table[id] -> Groonga::Record
 *
 * _table_ の _id_ に対応するGroonga::Recordを返す。
 *
 * 0.9.0から値ではなくGroonga::Recordを返すようになった。
 */
VALUE
rb_grn_table_array_reference (VALUE self, VALUE rb_id)
{
    if (FIXNUM_P(rb_id)) {
	return rb_grn_record_new_raw(self, rb_id, Qnil);
    } else {
	return Qnil;
    }
}

VALUE
rb_grn_table_get_value (VALUE self, VALUE rb_id)
{
    grn_id id;
    grn_ctx *context;
    grn_obj *table;
    grn_obj *range;
    grn_obj *value;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     &value, NULL, &range,
			     NULL);

    id = NUM2UINT(rb_id);
    GRN_BULK_REWIND(value);
    grn_obj_get_value(context, table, id, value);
    rb_grn_context_check(context, self);

    return GRNBULK2RVAL(context, value, range, self);
}

/*
 * Document-method: value
 *
 * call-seq:
 *   table.value(id) -> 値
 *   table.value(id, :id => true) -> 値
 *
 * _table_ の _id_ に対応する値を返す。
 *
 * <tt>:id => true</tt>が指定できるのは利便性のため。
 * Groonga::ArrayでもGroonga::HashやGroonga::PatriciaTrieと
 * 同じ引数で動くようになる。
 */
static VALUE
rb_grn_table_get_value_convenience (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_id, rb_options;

    rb_scan_args(argc, argv, "11", &rb_id, &rb_options);
    if (!NIL_P(rb_options)) {
	VALUE rb_option_id;
	rb_grn_scan_options(rb_options,
			    "id", &rb_option_id,
			    NULL);
	if (!(NIL_P(rb_option_id) || RVAL2CBOOL(rb_option_id))) {
	    rb_raise(rb_eArgError, ":id options must be true or nil: %s: %s",
		     rb_grn_inspect(rb_option_id),
		     rb_grn_inspect(rb_ary_new3(2,
						self, rb_ary_new4(argc, argv))));
	}
    }

    return rb_grn_table_get_value(self, rb_id);
}

VALUE
rb_grn_table_set_value (VALUE self, VALUE rb_id, VALUE rb_value)
{
    grn_id id;
    grn_ctx *context;
    grn_obj *table;
    grn_obj *range;
    grn_obj *value;
    grn_rc rc;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     &value, NULL, &range,
			     NULL);

    id = NUM2UINT(rb_id);
    GRN_BULK_REWIND(value);
    RVAL2GRNBULK(rb_value, context, value);
    rc = grn_obj_set_value(context, table, id, value, GRN_OBJ_SET);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * Document-method: set_value
 *
 * call-seq:
 *   table.set_value(id, value)
 *   table.set_value(id, value, :id => true)
 *
 * _table_ の _id_ に対応する値として _value_ 設定する。既存の値は
 * 上書きされる。
 *
 * <tt>:id => true</tt>が指定できるのは利便性のため。
 * Groonga::ArrayでもGroonga::HashやGroonga::PatriciaTrieと
 * 同じ引数で動くようになる。
 */
static VALUE
rb_grn_table_set_value_convenience (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_id, rb_value, rb_options;

    rb_scan_args(argc, argv, "21", &rb_id, &rb_value, &rb_options);
    if (!NIL_P(rb_options)) {
	VALUE rb_option_id;
	rb_grn_scan_options(rb_options,
			    "id", &rb_option_id,
			    NULL);
	if (!(NIL_P(rb_option_id) || RVAL2CBOOL(rb_option_id))) {
	    rb_raise(rb_eArgError, ":id options must be true or nil: %s: %s",
		     rb_grn_inspect(rb_option_id),
		     rb_grn_inspect(rb_ary_new3(2,
						self, rb_ary_new4(argc, argv))));
	}
    }

    return rb_grn_table_set_value(self, rb_id, rb_value);
}

VALUE
rb_grn_table_get_column_value_raw (VALUE self, grn_id id, VALUE rb_name)
{
    VALUE rb_column;

    rb_column = rb_grn_table_get_column_surely(self, rb_name);

    /* TODO: improve speed. */
    return rb_funcall(rb_column, id_array_reference, 1, INT2NUM(id));
}

VALUE
rb_grn_table_get_column_value (VALUE self, VALUE rb_id, VALUE rb_name)
{
    return rb_grn_table_get_column_value_raw(self, NUM2INT(rb_id), rb_name);
}

/*
 * Document-method: column_value
 *
 * call-seq:
 *   table.column_value(id, name) -> 値
 *   table.column_value(id, name, :id => true) -> 値
 *
 * _table_ の _id_ に対応するカラム _name_ の値を返す。
 *
 * <tt>:id => true</tt>が指定できるのは利便性のため。
 * Groonga::ArrayでもGroonga::HashやGroonga::PatriciaTrieと
 * 同じ引数で動くようになる。
 */
static VALUE
rb_grn_table_get_column_value_convenience (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_id, rb_name, rb_options;

    rb_scan_args(argc, argv, "21", &rb_id, &rb_name, &rb_options);
    if (!NIL_P(rb_options)) {
	VALUE rb_option_id;
	rb_grn_scan_options(rb_options,
			    "id", &rb_option_id,
			    NULL);
	if (!(NIL_P(rb_option_id) || RVAL2CBOOL(rb_option_id))) {
	    rb_raise(rb_eArgError, ":id options must be true or nil: %s: %s",
		     rb_grn_inspect(rb_option_id),
		     rb_grn_inspect(rb_ary_new3(2,
						self,
						rb_ary_new4(argc, argv))));
	}
    }

    return rb_grn_table_get_column_value(self, rb_id, rb_name);
}

VALUE
rb_grn_table_set_column_value_raw (VALUE self, grn_id id,
				   VALUE rb_name, VALUE rb_value)
{
    VALUE rb_column;

    rb_column = rb_grn_table_get_column_surely(self, rb_name);

    /* TODO: improve speed. */
    return rb_funcall(rb_column, id_array_set, 2, INT2NUM(id), rb_value);
}

VALUE
rb_grn_table_set_column_value (VALUE self, VALUE rb_id,
			       VALUE rb_name, VALUE rb_value)
{
    return rb_grn_table_set_column_value_raw(self, NUM2INT(rb_id),
					     rb_name, rb_value);
}

/*
 * Document-method: set_column_value
 *
 * call-seq:
 *   table.set_column_value(id, name, value)
 *   table.set_column_value(id, name, value, :id => true)
 *
 * _table_ の _id_ に対応するカラム _name_ の値として _value_ 設定す
 * る。既存の値は上書きされる。
 *
 * <tt>:id => true</tt>が指定できるのは利便性のため。
 * Groonga::ArrayでもGroonga::HashやGroonga::PatriciaTrieと
 * 同じ引数で動くようになる。
 */
static VALUE
rb_grn_table_set_column_value_convenience (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_id, rb_name, rb_value, rb_options;

    rb_scan_args(argc, argv, "31", &rb_id, &rb_name, &rb_value, &rb_options);
    if (!NIL_P(rb_options)) {
	VALUE rb_option_id;
	rb_grn_scan_options(rb_options,
			    "id", &rb_option_id,
			    NULL);
	if (!(NIL_P(rb_option_id) || RVAL2CBOOL(rb_option_id))) {
	    rb_raise(rb_eArgError, ":id options must be true or nil: %s: %s",
		     rb_grn_inspect(rb_option_id),
		     rb_grn_inspect(rb_ary_new3(2,
						self,
						rb_ary_new4(argc, argv))));
	}
    }

    return rb_grn_table_set_column_value(self, rb_id, rb_name, rb_value);
}

/*
 * Document-method: unlock
 *
 * call-seq:
 *   table.unlock(options={})
 *
 * _table_ のロックを解除する。
 *
 * 利用可能なオプションは以下の通り。
 *
 * [ _:id_ ]
 *   _:id_ で指定したレコードのロックを解除する。（注:
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
			     NULL, NULL, NULL,
			     NULL);

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
 *   table.lock(options={}) {}
 *
 * _table_ をロックする。ロックに失敗した場合は
 * Groonga::ResourceDeadlockAvoided例外が発生する。
 *
 * ブロックを指定した場合はブロックを抜けたときにunlockする。
 *
 * 利用可能なオプションは以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :timeout The timeout
 *
 *   ロックを獲得できなかった場合は _:timeout_ 秒間ロックの獲
 *   得を試みる。 _:timeout_ 秒以内にロックを獲得できなかった
 *   場合は例外が発生する。
 *
 * @option options :id The id
 *
 *   _:id_ で指定したレコードをロックする。（注: groonga側が
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
			     NULL, NULL, NULL,
			     NULL);

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
 * _table_ のロックを強制的に解除する。
 *
 * 利用可能なオプションは以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :id The id
 *
 *   _:id_ で指定したレコードのロックを強制的に解除する。
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
			     NULL, NULL, NULL,
			     NULL);

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
 * _table_ がロックされていれば +true+ を返す。
 *
 * 利用可能なオプションは以下の通り。
 * @param options [options] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :id The id
 *
 *   _:id_ で指定したレコードがロックされていれば +true+ を返す。
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
			     NULL, NULL, NULL,
			     NULL);

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
 * _table_ からブロックまたは文字列で指定した条件にマッチする
 * レコードを返す。返されたテーブルには +expression+ という特
 * 異メソッドがあり、指定した条件を表している
 * Groonga::Expressionを取得できる。
 * Groonga::Expression#snippetを使うことにより、指定した条件
 * 用のスニペットを簡単に生成できる。
 *
 * <pre>
 * !!!ruby
 * results = table.select do |record|
 *   record["description"] =~ "groonga"
 * end
 * snippet = results.expression.snippet([["<em>", "</em>"]])
 * results.each do |record|
 *   puts "#{record['name']}の説明文の中で「groonga」が含まれる部分"
 *   snippet.execute(record["description"]).each do |snippet|
 *     puts "---"
 *     puts "#{snippet}..."
 *     puts "---"
 *   end
 * end
 * </pre>
 *
 * 出力例
 *
 * <pre>
 * !!!text
 * rroongaの説明文の中で「groonga」が含まれる部分
 * ---
 * rroongaは<em>groonga</em>のいわゆるDB-APIの層の...
 * ---
 * </pre>
 *
 * _query_ には「[カラム名]:[演算子][値]」という書式で条件を
 * 指定する。演算子は以下の通り。
 *
 * - なし := [カラム値] == [値]
 * - @!@ := [カラム値] != [値]
 * - @<@ := [カラム値] < [値]
 * - @>@ := [カラム値] > [値]
 * - @<=@ := [カラム値] <= [値]
 * - @>=@ := [カラム値] >= [値]
 * - @@@ := [カラム値]が[値]を含んでいるかどうか
 *
 * 例:
 *
 * <pre>
 * !!!ruby
 * "name:daijiro" # "name"カラムの値が"daijiro"のレコードにマッチ
 * "description:@groonga" # "description"カラムが
 *                        # "groonga"を含んでいるレコードにマッチ
 * </pre>
 *
 * _expression_ には既に作成済みのGroonga::Expressionを渡す
 *
 * ブロックで条件を指定する場合は
 * Groonga::RecordExpressionBuilderを参照。
 *
 * Ruby1.9以降では、ブロックで条件を指定する際に
 * Groonga::ColumnExpressionBuilderの他に"!="も使用可能。
 *
 * 例:
 *
 * <pre>
 * !!!ruby
 * comments = Groonga::Array.create(:name => "Comments")
 * comments.define_column("content", "Text")
 * comments.add(:content => "Hello Good-bye!")
 * comments.add(:content => "Hello World")
 * comments.add(:content => "test")
 * result = comments.select do |record|
 *   record.content != "test"
 * end
 * p result.collect {|record| record.content}
 *   # => ["Hello Good-bye!", "Hello World"]
 * </pre>
 *
 * _options_ に指定可能な値は以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :default_column
 *
 *   "column_name:hoge"ではなく"hoge"のようにcolumn_nameが指
 *   定されない条件の検索対象となるカラムを指定する。
 *
 * @option options :operator (Groonga::Operator::OR) The operator
 *
 *   マッチしたレコードをどのように扱うか。指定可能な値は以
 *   下の通り。省略した場合はGroonga::Operator::OR。
 *
 *   [Groonga::Operator::OR] マッチしたレコードを追加。すで
 *                           にレコードが追加されている場合
 *                           は何もしない。
 *   [Groonga::Operator::AND] マッチしたレコードのスコアを増
 *                            加。マッチしなかったレコード
 *                            を削除。
 *   [Groonga::Operator::BUT] マッチしたレコードを削除。
 *   [Groonga::Operator::ADJUST] マッチしたレコードのスコア
 *                               を増加。
 *
 * @option options :result The result
 *
 *   検索結果を格納するテーブル。マッチしたレコードが追加さ
 *   れていく。省略した場合は新しくテーブルを作成して返す。
 *
 * @option options :name The name
 *
 *   条件の名前。省略した場合は名前を付けない。
 *
 * @option options :syntax The syntax
 *
 *   _query_ の構文。省略した場合は +:query+ 。
 *
 *   参考: Groonga::Expression#parse.
 *
 * @option options :allow_pragma The allow_pragma
 *
 *   query構文時にプラグマを利用するかどうか。省略した場合は
 *   利用する。
 *
 *   参考: Groonga::Expression#parse.
 *
 * @option options :allow_column The allow_column
 *
 *   query構文時にカラム指定を利用するかどうか。省略した場合
 *   は利用する。
 *
 *   参考: Groonga::Expression#parse.
 *
 * @option options :allow_update The allow_update
 *
 *   script構文時に更新操作を利用するかどうか。省略した場合
 *   は利用する。
 *
 *   参考: Groonga::Expression#parse.
 */
static VALUE
rb_grn_table_select (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *table, *result, *expression;
    grn_operator operator = GRN_OP_OR;
    VALUE rb_query = Qnil, condition_or_options, options;
    VALUE rb_name, rb_operator, rb_result, rb_syntax;
    VALUE rb_allow_pragma, rb_allow_column, rb_allow_update;
    VALUE rb_default_column;
    VALUE rb_expression = Qnil, builder;

    rb_scan_args(argc, argv, "02", &condition_or_options, &options);

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);

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
			"syntax", &rb_syntax,
			"allow_pragma", &rb_allow_pragma,
			"allow_column", &rb_allow_column,
			"allow_update", &rb_allow_update,
			"default_column", &rb_default_column,
			NULL);

    if (!NIL_P(rb_operator))
	operator = NUM2INT(rb_operator);

    if (NIL_P(rb_result)) {
	result = grn_table_create(context, NULL, 0, NULL,
				  GRN_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
				  table,
				  NULL);
	rb_result = GRNTABLE2RVAL(context, result, GRN_TRUE);
    } else {
	result = RVAL2GRNTABLE(rb_result, &context);
    }

    if (NIL_P(rb_expression)) {
      builder = rb_grn_record_expression_builder_new(self, rb_name);
      rb_funcall(builder, rb_intern("query="), 1, rb_query);
      rb_funcall(builder, rb_intern("syntax="), 1, rb_syntax);
      rb_funcall(builder, rb_intern("allow_pragma="), 1, rb_allow_pragma);
      rb_funcall(builder, rb_intern("allow_column="), 1, rb_allow_column);
      rb_funcall(builder, rb_intern("allow_update="), 1, rb_allow_update);
      rb_funcall(builder, rb_intern("default_column="), 1, rb_default_column);
      rb_expression = rb_grn_record_expression_builder_build(builder);
    }
    rb_grn_object_deconstruct(RB_GRN_OBJECT(DATA_PTR(rb_expression)),
                              &expression, NULL,
			      NULL, NULL, NULL, NULL);

    grn_table_select(context, table, expression, result, operator);
    rb_grn_context_check(context, self);

    rb_attr(rb_singleton_class(rb_result),
	    rb_intern("expression"),
	    GRN_TRUE, GRN_FALSE, GRN_FALSE);
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
			     NULL, NULL, NULL,
			     NULL);
    rb_grn_table_deconstruct(SELF(rb_other), &other, NULL,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);

    rc = grn_table_setoperation(context, table, other, table, operator);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return self;
}

/*
 * call-seq:
 *   table.union!(other) -> Groonga::Table
 *
 * キーを比較し、 _table_ には登録されていない _other_ のレコー
 * ドを _table_ に作成する。
 *
 */
static VALUE
rb_grn_table_union_bang (VALUE self, VALUE rb_other)
{
    return rb_grn_table_set_operation_bang(self, rb_other, GRN_OP_OR);
}


/*
 * call-seq:
 *   table.intersection!(other) -> Groonga::Table
 *
 * キーを比較し、 _other_ には登録されていないレコードを
 * _table_ から削除する。
 *
 */
static VALUE
rb_grn_table_intersection_bang (VALUE self, VALUE rb_other)
{
    return rb_grn_table_set_operation_bang(self, rb_other, GRN_OP_AND);
}

/*
 * call-seq:
 *   table.difference!(other) -> Groonga::Table
 *
 * キーを比較し、 _other_ にも登録されているレコードを _table_
 * から削除する。
 *
 */
static VALUE
rb_grn_table_difference_bang (VALUE self, VALUE rb_other)
{
    return rb_grn_table_set_operation_bang(self, rb_other, GRN_OP_BUT);
}

/*
 * call-seq:
 *   table.merge!(other) -> Groonga::Table
 *
 * キーを比較し、 _other_ にも登録されている _table_ のレコード
 * のスコアを _other_ のスコアと同値にする。
 *
 */
static VALUE
rb_grn_table_merge_bang (VALUE self, VALUE rb_other)
{
    return rb_grn_table_set_operation_bang(self, rb_other, GRN_OP_ADJUST);
}

/*
 * call-seq:
 *   table.support_key? -> true/false
 *
 * _table_ に主キーが設定されていれば +true+ 、されていなければ
 * +false+ を返す。
 */
static VALUE
rb_grn_table_support_key_p (VALUE self)
{
    return Qfalse;
}

/*
 * call-seq:
 *   table.support_sub_records? -> true/false
 *
 * グループ化したとき、テーブルにグループに含まれるレコード
 * 数を格納できる場合は +true+ 、格納できない場合は +false+ を返
 * す。
 */
static VALUE
rb_grn_table_support_sub_records_p (VALUE self)
{
    grn_obj *table;

    rb_grn_table_deconstruct(SELF(self), &table, NULL,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);
    return CBOOL2RVAL(table->header.flags & GRN_OBJ_WITH_SUBREC);
}

/*
 * call-seq:
 *   table.exist?(id) -> true/false
 *
 * _table_ に _id_ で指定したIDのレコードが存在する場合は +true+ 、
 * 存在しない場合は +false+ を返す。
 *
 * 注意: 実行には相応のコストがかかるのであまり頻繁に呼ばな
 * いようにして下さい。
 */
static VALUE
rb_grn_table_exist_p (VALUE self, VALUE id)
{
    grn_ctx *context;
    grn_obj *table;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);
    return CBOOL2RVAL(grn_table_at(context, table, NUM2UINT(id)));
}

/*
 * Document-method: defrag
 *
 * call-seq:
 *   table.defrag(options={}) -> n_segments
 *
 * Defrags all variable size columns in the table.
 *
 * @return [Integer] the number of defraged segments
 * @option options [Integer] :threshold (0) the threshold to
 *   determine whether a segment is defraged. Available
 *   values are -4..22. -4 means all segments are defraged.
 *   22 means no segment is defraged.
 * @since 1.3.0
 */
static VALUE
rb_grn_table_defrag (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *table;
    int n_segments;
    VALUE options, rb_threshold;
    int threshold = 0;

    rb_scan_args(argc, argv, "01", &options);
    rb_grn_scan_options(options,
			"threshold", &rb_threshold,
			NULL);
    if (!NIL_P(rb_threshold)) {
	threshold = NUM2INT(rb_threshold);
    }

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL, NULL,
			     NULL, NULL,
			     NULL);
    n_segments = grn_obj_defrag(context, table, threshold);
    rb_grn_context_check(context, self);

    return INT2NUM(n_segments);
}

/*
 * Document-method: rename
 *
 * call-seq:
 *   table.rename(name)
 *
 * Renames the table to name.
 *
 * @param name [String] the new name
 * @since 1.3.0
 */
static VALUE
rb_grn_table_rename (VALUE self, VALUE rb_name)
{
    int rc;
    grn_ctx *context;
    grn_obj *table;
    char *name;
    int name_size;

    rb_grn_table_deconstruct(SELF(self), &table, &context,
			     NULL, NULL, NULL,
			     NULL, NULL,
			     NULL);

    name = StringValueCStr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    rc = grn_table_rename(context, table, name, name_size);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return self;
}

void
rb_grn_init_table (VALUE mGrn)
{
    id_array_reference = rb_intern("[]");
    id_array_set = rb_intern("[]=");

    rb_cGrnTable = rb_define_class_under(mGrn, "Table", rb_cGrnObject);
    rb_define_alloc_func(rb_cGrnTable, rb_grn_table_alloc);

    rb_include_module(rb_cGrnTable, rb_mEnumerable);

    rb_define_method(rb_cGrnTable, "initialize", rb_grn_table_initialize, 0);

    rb_define_method(rb_cGrnTable, "inspect", rb_grn_table_inspect, 0);

    rb_define_method(rb_cGrnTable, "define_column",
		     rb_grn_table_define_column, -1);
    rb_define_method(rb_cGrnTable, "define_index_column",
		     rb_grn_table_define_index_column, -1);
    rb_define_method(rb_cGrnTable, "column",
		     rb_grn_table_get_column, 1);
    rb_define_method(rb_cGrnTable, "columns",
		     rb_grn_table_get_columns, -1);
    rb_define_method(rb_cGrnTable, "have_column?",
		     rb_grn_table_have_column, 1);

    rb_define_method(rb_cGrnTable, "open_cursor", rb_grn_table_open_cursor, -1);
    rb_define_method(rb_cGrnTable, "records", rb_grn_table_get_records, -1);

    rb_define_method(rb_cGrnTable, "size", rb_grn_table_get_size, 0);
    rb_define_method(rb_cGrnTable, "empty?", rb_grn_table_empty_p, 0);
    rb_define_method(rb_cGrnTable, "truncate", rb_grn_table_truncate, 0);

    rb_define_method(rb_cGrnTable, "each", rb_grn_table_each, 0);

    rb_define_method(rb_cGrnTable, "delete", rb_grn_table_delete, 1);

    rb_define_method(rb_cGrnTable, "sort", rb_grn_table_sort, -1);
    rb_define_method(rb_cGrnTable, "group", rb_grn_table_group, -1);

    rb_define_method(rb_cGrnTable, "[]", rb_grn_table_array_reference, 1);
    rb_undef_method(rb_cGrnTable, "[]=");

    rb_define_method(rb_cGrnTable, "value",
		     rb_grn_table_get_value_convenience, -1);
    rb_define_method(rb_cGrnTable, "set_value",
		     rb_grn_table_set_value_convenience, -1);
    rb_define_method(rb_cGrnTable, "column_value",
		     rb_grn_table_get_column_value_convenience, -1);
    rb_define_method(rb_cGrnTable, "set_column_value",
		     rb_grn_table_set_column_value_convenience, -1);

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

    rb_define_method(rb_cGrnTable, "support_key?",
		     rb_grn_table_support_key_p, 0);
    rb_define_method(rb_cGrnTable, "support_sub_records?",
		     rb_grn_table_support_sub_records_p, 0);

    rb_define_method(rb_cGrnTable, "exist?", rb_grn_table_exist_p, 1);

    rb_define_method(rb_cGrnTable, "defrag", rb_grn_table_defrag, -1);

    rb_define_method(rb_cGrnTable, "rename", rb_grn_table_rename, 1);

    rb_grn_init_table_key_support(mGrn);
    rb_grn_init_array(mGrn);
    rb_grn_init_hash(mGrn);
    rb_grn_init_patricia_trie(mGrn);
    rb_grn_init_double_array_trie(mGrn);
    rb_grn_init_view(mGrn);
}
