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

#include <string.h>

#define SELF(object) ((RbGrnIndexColumn *)DATA_PTR(object))

VALUE rb_cGrnIndexColumn;

/*
 * Document-class: Groonga::IndexColumn < Groonga::Column
 *
 * 転置索引エントリを格納するカラム。このカラムを利用するこ
 * とにより高速な全文検索を実現できる。
 *
 * テーブルにGroonga::IndexColumnを定義する方法は
 * {Groonga::Table#define_index_column} を参照。
 */

void
rb_grn_index_column_finalizer (grn_ctx *context, grn_obj *object,
			       RbGrnIndexColumn *rb_grn_index_column)
{
    if (!context)
	return;

    grn_obj_unlink(context, rb_grn_index_column->id_query);
    grn_obj_unlink(context, rb_grn_index_column->string_query);
    grn_obj_unlink(context, rb_grn_index_column->old_value);

    rb_grn_column_finalizer(context, object, RB_GRN_COLUMN(rb_grn_index_column));
}

void
rb_grn_index_column_bind (RbGrnIndexColumn *rb_grn_index_column,
			  grn_ctx *context, grn_obj *column)
{
    RbGrnObject *rb_grn_object;

    rb_grn_column_bind(RB_GRN_COLUMN(rb_grn_index_column), context, column);
    rb_grn_object = RB_GRN_OBJECT(rb_grn_index_column);

    rb_grn_index_column->old_value = grn_obj_open(context, GRN_BULK, 0,
						  rb_grn_object->range_id);

    rb_grn_index_column->id_query = grn_obj_open(context, GRN_BULK, 0,
						 rb_grn_object->domain_id);
    rb_grn_index_column->string_query = grn_obj_open(context, GRN_BULK,
						     GRN_OBJ_DO_SHALLOW_COPY,
						     GRN_ID_NIL);
}

void
rb_grn_index_column_deconstruct (RbGrnIndexColumn *rb_grn_index_column,
				 grn_obj **column,
				 grn_ctx **context,
				 grn_id *domain_id,
				 grn_obj **domain,
				 grn_obj **value,
				 grn_obj **old_value,
				 grn_id *range_id,
				 grn_obj **range,
				 grn_obj **id_query,
				 grn_obj **string_query)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_index_column);
    rb_grn_column_deconstruct(RB_GRN_COLUMN(rb_grn_object), column, context,
			      domain_id, domain, value,
			      range_id, range);

    if (old_value)
	*old_value = rb_grn_index_column->old_value;
    if (id_query)
	*id_query = rb_grn_index_column->id_query;
    if (string_query)
	*string_query = rb_grn_index_column->string_query;
}

/*
 * call-seq:
 *   column[id] = value
 *   column[id] = options
 *
 * IDが _id_ であるレコードを高速に全文検索するため転置索引を作
 * 成する。多くの場合、Groonga::Table#define_index_columnで
 * +:source+ オプションを指定することにより、自動的に全文検索
 * 用の索引は更新されるので、明示的にこのメソッドを使うこと
 * は少ない。
 *
 * _value_ には文字列を指定する。
 *
 * _options_ を指定することにより、より索引の作成を制御できる。
 * @param [::Hash] options  The name and value
 *   pairs. Omitted names are initialized as the default value
 * @option options :section The section
 *
 *   段落番号を指定する。省略した場合は1を指定したとみなされ
 *   る。
 *   Groonga::Table#define_index_columnで
 *   <tt>{:with_section => true}</tt>を指定していなければい
 *   けない。
 *
 * @option options :old_value The old_value
 *
 *   以前の値を指定する。省略した場合は現在の値が用いられる。
 *   通常は指定する必要はない。
 *
 * @option options :value The value
 *   新しい値を指定する。 _value_ を指定した場合と _options_ で
 *   <tt>{:value => value}</tt>を指定した場合は同じ動作とな
 *   る。
 *
 * @example 記事の段落毎に索引を作成する。
 *   articles = Groonga::Array.create(:name => "<articles>")
 *   articles.define_column("title", "ShortText")
 *   articles.define_column("content", "Text")
 *
 *   terms = Groonga::Hash.create(:name => "<terms>",
 *                                :default_tokenizer => "TokenBigram")
 *   content_index = terms.define_index_column("content", articles,
 *                                             :with_section => true)
 *
 *   content = <<-EOC
 *   groonga は組み込み型の全文検索エンジンライブラリです。
 *   DBMSやスクリプト言語処理系等に組み込むことによって、その
 *   全文検索機能を強化することができます。また、リレーショナ
 *   ルモデルに基づくデータストア機能を内包しており、groonga
 *   単体でも高速なデータストアサーバとして使用することができ
 *   ます。
 *
 *   ■全文検索方式
 *   転置索引型の全文検索エンジンです。転置索引は圧縮されてファ
 *   イルに格納され、検索時のディスク読み出し量を小さく、かつ
 *   局所的に抑えるように設計されています。用途に応じて以下の
 *   索引タイプを選択できます。
 *   EOC
 *
 *   groonga = articles.add(:title => "groonga", :content => content)
 *
 *   content.split(/\n{2,}/).each_with_index do |sentence, i|
 *     content_index[groonga] = {:value => sentence, :section => i + 1}
 *   end
 *
 *   content_index.search("エンジン").collect do |record|
 *     p record.key["title"] # -> "groonga"
 *   end
 */
static VALUE
rb_grn_index_column_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    grn_ctx *context = NULL;
    grn_obj *column, *range;
    grn_rc rc;
    grn_id id;
    unsigned int section;
    grn_obj *old_value, *new_value;
    VALUE original_rb_value, rb_section, rb_old_value, rb_new_value;

    original_rb_value = rb_value;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
				    NULL, NULL,
				    &new_value, &old_value,
				    NULL, &range,
				    NULL, NULL);

    id = RVAL2GRNID(rb_id, context, range, self);

    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_value, rb_cHash))) {
	VALUE hash_value;
	hash_value = rb_hash_new();
	rb_hash_aset(hash_value, RB_GRN_INTERN("value"), rb_value);
	rb_value = hash_value;
    }

    rb_grn_scan_options(rb_value,
			"section", &rb_section,
			"old_value", &rb_old_value,
			"value", &rb_new_value,
			NULL);

    if (NIL_P(rb_section))
	section = 1;
    else
	section = NUM2UINT(rb_section);

    if (NIL_P(rb_old_value)) {
	old_value = NULL;
    } else {
	GRN_BULK_REWIND(old_value);
	RVAL2GRNBULK(rb_old_value, context, old_value);
    }

    if (NIL_P(rb_new_value)) {
	new_value = NULL;
    } else {
	GRN_BULK_REWIND(new_value);
	RVAL2GRNBULK(rb_new_value, context, new_value);
    }

    rc = grn_column_index_update(context, column,
				 id, section, old_value, new_value);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return original_rb_value;
}

/*
 * call-seq:
 *   column.sources -> Groonga::Columnの配列
 *
 * インデックス対象となっているカラムの配列を返す。
 */
static VALUE
rb_grn_index_column_get_sources (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj sources;
    grn_id *source_ids;
    VALUE rb_sources;
    int i, n;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
				    NULL, NULL,
				    NULL, NULL, NULL, NULL,
				    NULL, NULL);

    GRN_OBJ_INIT(&sources, GRN_BULK, 0, GRN_ID_NIL);
    grn_obj_get_info(context, column, GRN_INFO_SOURCE, &sources);
    rb_grn_context_check(context, self);

    n = GRN_BULK_VSIZE(&sources) / sizeof(grn_id);
    source_ids = (grn_id *)GRN_BULK_HEAD(&sources);
    rb_sources = rb_ary_new2(n);
    for (i = 0; i < n; i++) {
	grn_obj *source;
	VALUE rb_source;

	source = grn_ctx_at(context, *source_ids);
	rb_source = GRNOBJECT2RVAL(Qnil, context, source, GRN_FALSE);
	rb_ary_push(rb_sources, rb_source);
	source_ids++;
    }
    grn_obj_unlink(context, &sources);

    return rb_sources;
}

static grn_id
resolve_source_id (grn_ctx *context, grn_obj *column, grn_id range_id,
		   VALUE rb_source)
{
    grn_id source_id;

    if (CBOOL2RVAL(rb_obj_is_kind_of(rb_source, rb_cInteger))) {
	source_id = NUM2UINT(rb_source);
    } else {
	grn_obj *source;
	grn_bool need_source_unlink = GRN_FALSE;

	if (TYPE(rb_source) == T_STRING) {
	    grn_obj *table;
	    const char *name;
	    const char *dot_point;
	    int length;

	    table = grn_ctx_at(context, grn_obj_get_range(context, column));
	    name = StringValueCStr(rb_source);
	    length = RSTRING_LEN(rb_source);
	    dot_point = strstr(name, ".");
	    if (dot_point) {
		char table_name[4096];
		int table_name_length;

		table_name_length = grn_obj_name(context, table,
						 table_name, sizeof(table_name));
		table_name[table_name_length] = '\0';
		if (strncmp(table_name, name, dot_point - name) != 0) {
		    rb_raise(rb_eArgError,
			     "wrong table's column: <%s>: "
			     "expected table: <%s>",
			     name, table_name);
		}
		length -= (dot_point - name) + 1;
		name = dot_point + 1;
	    }
	    source = grn_obj_column(context, table, name, length);
	    need_source_unlink = GRN_TRUE;
	} else {
	    source = RVAL2GRNOBJECT(rb_source, &context);
	}
	rb_grn_context_check(context, rb_source);
	if (source->header.type == GRN_ACCESSOR) {
	    char name[256];
	    int length;
	    length = grn_column_name(context, source, name, sizeof(name));
	    name[length] = '\0';
	    if (strcmp(name, "_key") != 0) {
		rb_raise(rb_eArgError,
			 "source accessor must be '_key': <%s>", name);
	    }
	    source_id = range_id;
	} else {
	    source_id = grn_obj_id(context, source);
	}
	if (need_source_unlink) {
	    grn_obj_unlink(context, source);
	}
    }

    return source_id;
}

/*
 * call-seq:
 *   column.sources = Groonga::Columnの配列
 *
 * インデックス対象となる複数のカラムを配列で設定する。
 */
static VALUE
rb_grn_index_column_set_sources (VALUE self, VALUE rb_sources)
{
    VALUE exception;
    grn_ctx *context = NULL;
    grn_obj *column;
    int i, n;
    VALUE *rb_source_values;
    grn_id range_id;
    grn_id *sources;
    grn_rc rc;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
				    NULL, NULL,
				    NULL, NULL,
				    &range_id, NULL,
				    NULL, NULL);

    n = RARRAY_LEN(rb_sources);
    rb_source_values = RARRAY_PTR(rb_sources);
    sources = ALLOCA_N(grn_id, n);
    for (i = 0; i < n; i++) {
	sources[i] = resolve_source_id(context, column, range_id,
				       rb_source_values[i]);
    }

    {
	grn_obj bulk_sources;
	GRN_OBJ_INIT(&bulk_sources, GRN_BULK, 0, GRN_ID_NIL);
	GRN_TEXT_SET(context, &bulk_sources, sources, n * sizeof(grn_id));
	rc = grn_obj_set_info(context, column, GRN_INFO_SOURCE, &bulk_sources);
	exception = rb_grn_context_to_exception(context, self);
	grn_obj_unlink(context, &bulk_sources);
    }

    if (!NIL_P(exception))
	rb_exc_raise(exception);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   column.source = Groonga::Column
 *
 * インデックス対象となるカラムを設定する。
 */
static VALUE
rb_grn_index_column_set_source (VALUE self, VALUE rb_source)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_source, rb_cArray)))
	rb_source = rb_ary_new3(1, rb_source);

    return rb_grn_index_column_set_sources(self, rb_source);
}

/*
 * Document-method: search
 *
 * call-seq:
 *   column.search(query, options={}) -> Groonga::Hash
 *
 * _object_ から _query_ に対応するオブジェクトを検索し、見つかっ
 * たオブジェクトのIDがキーになっているGroonga::Hashを返す。
 *
 * 利用可能なオプションは以下の通り。
 * @param [::Hash] options The name and value
 *   pairs. Omitted names are initialized as the default value
 * @option options [Groonga::Hash] :result The result
 *   結果を格納するGroonga::Hash。指定しない場合は新しく
 *   Groonga::Hashを生成し、それに結果を格納して返す。
 * @option options :operator The operator
 *
 *   以下のどれかの値を指定する。 +nil+ , <tt>"or"</tt>, <tt>"||"</tt>,
 *   <tt>"and"</tt>, <tt>"+"</tt>, <tt>"&&"</tt>, <tt>"but"</tt>,
 *   <tt>"not"</tt>, <tt>"-"</tt>, <tt>"adjust"</tt>, <tt>">"</tt>。
 *   それぞれ以下のようになる。（FIXME: 「以下」）
 * @option options :exact The exact
 *
 *   +true+ を指定すると完全一致で検索する
 * @option options :longest_common_prefix The longest_common_prefix
 *   +true+ を指定すると _query_ と同じ接頭辞をもつエントリのう
 *   ち、もっとも長いエントリを検索する
 * @option options :suffix The suffix
 *   +true+ を指定すると _query_ が後方一致するエントリを検索す
 *   る
 * @option options :prefix The prefix
 *   +true+ を指定すると _query_ が前方一致するレコードを検索す
 *   る
 * @option options :near The near
 *   +true+ を指定すると _query_ に指定した複数の語が近傍に含ま
 *   れるレコードを検索する
 * [...]
 *   ...
 */
static VALUE
rb_grn_index_column_search (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *column;
    grn_obj *range;
    grn_obj *query = NULL, *id_query = NULL, *string_query = NULL;
    grn_obj *result;
    grn_operator operator;
    grn_rc rc;
    VALUE rb_query, options, rb_result, rb_operator;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
				    NULL, NULL,
				    NULL, NULL, NULL, &range,
				    &id_query, &string_query);

    rb_scan_args(argc, argv, "11", &rb_query, &options);

    if (CBOOL2RVAL(rb_obj_is_kind_of(rb_query, rb_cInteger))) {
	grn_id id;
	id = NUM2UINT(rb_query);
	GRN_TEXT_SET(context, id_query, &id, sizeof(grn_id));
	query = id_query;
    } else {
	const char *_query;
	_query = StringValuePtr(rb_query);
	GRN_TEXT_SET(context, string_query, _query, RSTRING_LEN(rb_query));
	query = string_query;
    }

    rb_grn_scan_options(options,
			"result", &rb_result,
			"operator", &rb_operator,
			NULL);

    if (NIL_P(rb_result)) {
	result = grn_table_create(context, NULL, 0, NULL,
				  GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
				  range, 0);
	rb_grn_context_check(context, self);
	rb_result = GRNOBJECT2RVAL(Qnil, context, result, GRN_TRUE);
    } else {
	result = RVAL2GRNOBJECT(rb_result, &context);
    }

    operator = RVAL2GRNOPERATOR(rb_operator);

    rc = grn_obj_search(context, column, query, result, operator, NULL);
    rb_grn_rc_check(rc, self);

    return rb_result;
}

/*
 * Document-method: with_section?
 *
 * call-seq:
 *   column.with_section? -> true/false
 *
 * _column_ が段落情報も格納する場合は +true+ を返します。
 */
static VALUE
rb_grn_index_column_with_section_p (VALUE self)
{
    grn_obj *column;

    rb_grn_index_column_deconstruct(SELF(self), &column, NULL,
				    NULL, NULL,
				    NULL, NULL, NULL, NULL,
				    NULL, NULL);

    return CBOOL2RVAL(column->header.flags & GRN_OBJ_WITH_SECTION);
}

/*
 * Document-method: with_weight?
 *
 * call-seq:
 *   column.with_weight? -> true/false
 *
 * _column_ がウェイト情報も格納する場合は +true+ を返します。
 */
static VALUE
rb_grn_index_column_with_weight_p (VALUE self)
{
    grn_obj *column;

    rb_grn_index_column_deconstruct(SELF(self), &column, NULL,
				    NULL, NULL,
				    NULL, NULL, NULL, NULL,
				    NULL, NULL);

    return CBOOL2RVAL(column->header.flags & GRN_OBJ_WITH_WEIGHT);
}

/*
 * Document-method: with_position?
 *
 * call-seq:
 *   column.with_position? -> true/false
 *
 * _column_ が位置情報も格納する場合は +true+ を返します。
 */
static VALUE
rb_grn_index_column_with_position_p (VALUE self)
{
    grn_obj *column;

    rb_grn_index_column_deconstruct(SELF(self), &column, NULL,
				    NULL, NULL,
				    NULL, NULL, NULL, NULL,
				    NULL, NULL);

    return CBOOL2RVAL(column->header.flags & GRN_OBJ_WITH_POSITION);
}

static VALUE
rb_grn_index_column_open_cursor (VALUE self, VALUE rb_table_cursor)
{
    grn_ctx *context;
    grn_obj *column;
    grn_table_cursor *table_cursor;
    grn_id rid_min = GRN_ID_NIL;
    grn_id rid_max = GRN_ID_MAX;
    int flags = 0;
    grn_obj *index_cursor;
    VALUE rb_cursor;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
				    NULL, NULL,
				    NULL, NULL, NULL, NULL,
				    NULL, NULL);
    table_cursor = RVAL2GRNTABLECURSOR(rb_table_cursor, NULL);

    index_cursor = grn_index_cursor_open(context, table_cursor,
					 column, rid_min, rid_max, flags);

    rb_cursor = GRNINDEXCURSOR2RVAL(context, index_cursor);

    if (rb_block_given_p())
	return rb_ensure(rb_yield, rb_cursor, rb_grn_object_close, rb_cursor);
    else
	return rb_cursor;
}

void
rb_grn_init_index_column (VALUE mGrn)
{
    rb_cGrnIndexColumn =
	rb_define_class_under(mGrn, "IndexColumn", rb_cGrnColumn);

    rb_define_method(rb_cGrnIndexColumn, "[]=",
		     rb_grn_index_column_array_set, 2);

    rb_define_method(rb_cGrnIndexColumn, "sources",
		     rb_grn_index_column_get_sources, 0);
    rb_define_method(rb_cGrnIndexColumn, "sources=",
		     rb_grn_index_column_set_sources, 1);
    rb_define_method(rb_cGrnIndexColumn, "source=",
		     rb_grn_index_column_set_source, 1);

    rb_define_method(rb_cGrnIndexColumn, "search",
		     rb_grn_index_column_search, -1);

    rb_define_method(rb_cGrnIndexColumn, "with_section?",
		     rb_grn_index_column_with_section_p, 0);
    rb_define_method(rb_cGrnIndexColumn, "with_weight?",
		     rb_grn_index_column_with_weight_p, 0);
    rb_define_method(rb_cGrnIndexColumn, "with_position?",
		     rb_grn_index_column_with_position_p, 0);
    rb_define_method(rb_cGrnIndexColumn, "open_cursor",
		     rb_grn_index_column_open_cursor, 1);
}
