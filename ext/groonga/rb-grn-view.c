/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2010-2011  Kouhei Sutou <kou@clear-code.com>

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

/* FIXME */
grn_id grn_view_add(grn_ctx *ctx, grn_obj *view, grn_obj *table);
grn_rc grn_table_cursor_next_o(grn_ctx *ctx, grn_table_cursor *tc, grn_obj *id);
grn_obj *grn_obj_get_value_o(grn_ctx *ctx, grn_obj *obj, grn_obj *id, grn_obj *value);

#define SELF(object) ((RbGrnTable *)DATA_PTR(object))

VALUE rb_cGrnView;

/*
 * Document-class: Groonga::View < Groonga::Table
 *
 * 複数のテーブルを1つのテーブルとして扱う仮想的なテーブル
 * （ビュー）。
 */

/*
 * call-seq:
 *   Groonga::View.create(options={})                -> Groonga::View
 *   Groonga::View.create(options={}) {|table| ... }
 *
 * 複数のテーブルを1つのテーブルとして扱う仮想的なテーブル
 * （ビュー）を生成する。ブロックを指定すると、そのブロック
 * に生成したテーブルが渡され、ブロックを抜けると自動的にテー
 * ブルが破棄される。
 *
 * ビューにテーブルを追加するときはGroonga::View#add_tableを
 * 使う。
 *
 * _options_ に指定可能な値は以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value
 * @option options :context (Groonga::Context.default)
 *
 *   ビューが利用するGroonga::Context。
 *
 * @option options :name The view name
 *
 *   ビューの名前。名前をつけると、Groonga::Context#[]に名
 *   前を指定してビューを取得することができる。省略すると
 *   無名ビューになり、ビューIDでのみ取得できる。
 *
 * @option options :path The path
 *
 *   ビューを保存するパス。パスを指定すると永続ビューとな
 *   り、プロセス終了後もレコードは保持される。次回起動時に
 *   Groonga::View.openで保存されたビューを利用することが
 *   できる。省略すると一時ビューになり、プロセスが終了する
 *   とビューは破棄される。
 *
 * @option options :persistent The persistent
 *
 *   +true+ を指定すると永続ビューとなる。 +path+ を省略した
 *   場合は自動的にパスが付加される。 +:context+ で指定した
 *   Groonga::Contextに結びついているデータベースが一時デー
 *   タベースの場合は例外が発生する。
 *
 * @example
 *   無名一時ビューを生成する。
 *     Groonga::View.create
 *
 * @example
 *   無名永続ビューを生成する。
 *     Groonga::View.create(:path => "/tmp/view.grn")
 *
 * @example
 *   名前付き永続ビューを生成する。ただし、ファイル名は気に
 *   しない。
 *     Groonga::View.create(:name => "Entries",
 *                          :persistent => true)
 *
 * @example
 *   +Users+ テーブルと +Dogs+ テーブルを横断検索するための
 *   るビューを生成する。
 *   entries = Groonga::View.create(:name => "Entries")
 *   entries.add_table("Users")
 *   entries.add_table("Dogs")
 */
static VALUE
rb_grn_view_s_create (int argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context;
    grn_obj *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_obj_flags flags = GRN_TABLE_VIEW;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"name", &rb_name,
                        "path", &rb_path,
			"persistent", &rb_persistent,
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

    table = grn_table_create(context, name, name_size, path, flags, NULL, NULL);
    if (!table)
	rb_grn_context_check(context, rb_ary_new4(argc, argv));
    rb_table = GRNOBJECT2RVAL(klass, context, table, GRN_TRUE);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

/*
 * call-seq:
 *   view.add_table(table)
 *
 * _table_ をビューからアクセスできるようにする。
 */
static VALUE
rb_grn_view_add_table (VALUE self, VALUE rb_table)
{
#ifdef WIN32
    rb_raise(rb_eNotImpError, "grn_view_add() isn't available on Windows.");
#else
    grn_ctx *context = NULL;
    grn_obj *view, *table;

    rb_grn_table_deconstruct(SELF(self), &view, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);
    table = RVAL2GRNOBJECT(rb_table, &context);
    grn_view_add(context, view, table);
    rb_grn_context_check(context, self);
#endif

    return Qnil;
}

/*
 * call-seq:
 *   view.each {|record| ...}
 *
 * ビューに登録されているテーブルのレコードを順番にブロック
 * に渡す。
 */
static VALUE
rb_grn_view_each (VALUE self)
{
#ifdef WIN32
    rb_raise(rb_eNotImpError,
	     "grn_table_cursor_next_o() isn't available on Windows.");
#else
    RbGrnTable *rb_grn_view;
    RbGrnObject *rb_grn_object;
    grn_ctx *context = NULL;
    grn_obj *view;
    grn_table_cursor *cursor;
    VALUE rb_cursor;
    grn_obj id;
    grn_rc rc = GRN_SUCCESS;

    rb_grn_view = SELF(self);
    rb_grn_table_deconstruct(rb_grn_view, &view, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);
    cursor = grn_table_cursor_open(context, view, NULL, 0, NULL, 0,
				   0, -1, GRN_CURSOR_ASCENDING);
    rb_cursor = GRNTABLECURSOR2RVAL(Qnil, context, cursor);
    rb_grn_object = RB_GRN_OBJECT(rb_grn_view);
    GRN_TEXT_INIT(&id, 0);
    while (rb_grn_object->object &&
	   (rc = grn_table_cursor_next_o(context, cursor, &id)) == GRN_SUCCESS) {
	rb_yield(rb_grn_view_record_new(self, &id));
    }
    GRN_OBJ_FIN(context, &id);
    rb_grn_object_close(rb_cursor);

    if (!(rc == GRN_SUCCESS || rc == GRN_END_OF_DATA)) {
	rb_grn_context_check(context, self);
    }
#endif

    return Qnil;
}

/*
 * Document-method: column_value
 *
 * call-seq:
 *   view.column_value(id, name) -> 値
 *
 * _view_ の _id_ に対応するカラム _name_ の値を返す。
 */
static VALUE
rb_grn_view_get_column_value (VALUE self, VALUE rb_id, VALUE rb_name)
{
    VALUE rb_value = Qnil;
#ifdef WIN32
    rb_raise(rb_eNotImpError,
	     "grn_obj_get_value_o() isn't available on Windows.");
#else
    RbGrnTable *rb_view;
    grn_ctx *context = NULL;
    grn_obj *view, *value, *accessor;
    grn_obj id;

    rb_view = SELF(self);
    rb_grn_table_deconstruct(rb_view, &view, &context,
			     NULL, NULL,
			     &value, NULL, NULL,
			     NULL);
    GRN_BULK_REWIND(value);
    GRN_TEXT_INIT(&id, 0);
    GRN_TEXT_PUT(context, &id, RSTRING_PTR(rb_id), RSTRING_LEN(rb_id));
    accessor = grn_obj_column(context, view,
			      RSTRING_PTR(rb_name), RSTRING_LEN(rb_name));
    grn_obj_get_value_o(context, accessor, &id, value);
    grn_obj_unlink(context, accessor);
    rb_value = GRNOBJ2RVAL(Qnil, context, value, self);
    GRN_OBJ_FIN(context, &id);
#endif

    return rb_value;
}

/*
 * call-seq:
 *   view.sort(keys, options={}) -> Groonga::ViewRecordの配列
 *
 * テーブルに登録されているレコードを _keys_ で指定されたルー
 * ルに従ってソートしたレコードの配列を返す。
 * <pre>
 *   ==[
 *    {:key => "カラム名", :order => :asc, :ascending,
 *                                   :desc, :descendingのいずれか},
 *    {:key => "カラム名", :order => :asc, :ascending,
 *                                   :desc, :descendingのいずれか},
 *    ...,
 *   ]==
 * </pre>
 * _options_ に指定可能な値は以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value
 * @option options :offset The offset
 *
 *   ソートされたレコードのうち、(0ベースで) _:offset_ 番目
 *   からレコードを取り出す。
 *
 * @option options :limit The limit
 *
 *   ソートされたレコードのうち、 _:limit_ 件のみを取り出す。
 *   省略された場合または-1が指定された場合は、全件が指定され
 *   たものとみなす。
 */
/* FIXME: DON'T WORK!!! */
static VALUE
rb_grn_view_sort (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_result = Qnil;

#ifdef WIN32
    rb_raise(rb_eNotImpError, "grn_view_add() isn't available on Windows.");
#else
    grn_ctx *context = NULL;
    grn_obj *view;
    grn_obj *result;
    grn_table_sort_key *keys;
    int i, n_keys;
    int offset = 0, limit = -1;
    VALUE rb_keys, options;
    VALUE rb_offset, rb_limit;
    VALUE *rb_sort_keys;
    grn_table_cursor *cursor;
    VALUE exception;
    grn_obj id;

    rb_grn_table_deconstruct(SELF(self), &view, &context,
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

    result = grn_table_create(context, NULL, 0, NULL, GRN_TABLE_VIEW,
			      NULL, NULL);
    /* use n_records that is return value from
       grn_table_sort() when rroonga user become specifying
       output table. */
    grn_table_sort(context, view, offset, limit, result, keys, n_keys);
    exception = rb_grn_context_to_exception(context, self);
    if (!NIL_P(exception)) {
        grn_obj_unlink(context, result);
        rb_exc_raise(exception);
    }

    rb_result = rb_ary_new();
    cursor = grn_table_cursor_open(context, result, NULL, 0, NULL, 0,
				   0, -1, GRN_CURSOR_ASCENDING);
    GRN_TEXT_INIT(&id, 0);
    while (grn_table_cursor_next_o(context, cursor, &id) == GRN_SUCCESS) {
	rb_ary_push(rb_result, rb_grn_view_record_new(self, &id));
    }
    GRN_OBJ_FIN(context, &id);
    grn_table_cursor_close(context, cursor);
    grn_obj_unlink(context, result);
#endif

    return rb_result;
}

void
rb_grn_init_view (VALUE mGrn)
{
    rb_cGrnView = rb_define_class_under(mGrn, "View", rb_cGrnTable);

    rb_define_singleton_method(rb_cGrnView, "create",
			       rb_grn_view_s_create, -1);

    rb_define_method(rb_cGrnView, "add_table", rb_grn_view_add_table, 1);
    rb_define_method(rb_cGrnView, "each", rb_grn_view_each, 0);
    rb_define_method(rb_cGrnView, "column_value",
		     rb_grn_view_get_column_value, 2);
    rb_define_method(rb_cGrnView, "sort", rb_grn_view_sort, -1);
}
