/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>

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
 * _options_に指定可能な値は以下の通り。
 *
 * [+:context+]
 *   ビューが利用するGroonga::Context。省略すると
 *   Groonga::Context.defaultを用いる。
 *
 * [+:name+]
 *   ビューの名前。名前をつけると、Groonga::Context#[]に名
 *   前を指定してビューを取得することができる。省略すると
 *   無名ビューになり、ビューIDでのみ取得できる。
 *
 * [+:path+]
 *   ビューを保存するパス。パスを指定すると永続ビューとな
 *   り、プロセス終了後もレコードは保持される。次回起動時に
 *   Groonga::View.openで保存されたビューを利用することが
 *   できる。省略すると一時ビューになり、プロセスが終了する
 *   とビューは破棄される。
 *
 * [+:persistent+]
 *   +true+を指定すると永続ビューとなる。+path+を省略した
 *   場合は自動的にパスが付加される。+:context+で指定した
 *   Groonga::Contextに結びついているデータベースが一時デー
 *   タベースの場合は例外が発生する。
 *
 * 使用例:
 *
 * 無名一時ビューを生成する。
 *   Groonga::View.create
 *
 * 無名永続ブーを生成する。
 *   Groonga::View.create(:path => "/tmp/view.grn")
 *
 * 名前付き永続ビューを生成する。ただし、ファイル名は気に
 * しない。
 *   Groonga::View.create(:name => "Entries",
 *                        :persistent => true)
 *
 * +Users+テーブルと+Dogs+テーブルを横断検索するための
 * るビューを生成する。
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
    rb_table = GRNOBJECT2RVAL(klass, context, table, RB_GRN_TRUE);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

/*
 * call-seq:
 *   view.add_table(table)
 *
 * _table_をビューからアクセスできるようにする。
 */
static VALUE
rb_grn_view_add_table (VALUE self, VALUE rb_table)
{
    grn_ctx *context = NULL;
    grn_obj *view, *table;

    rb_grn_table_deconstruct(SELF(self), &view, &context,
			     NULL, NULL,
			     NULL, NULL, NULL,
			     NULL);
    table = RVAL2GRNOBJECT(rb_table, &context);
    grn_view_add(context, view, table);
    rb_grn_context_check(context, self);

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

    return Qnil;
}

/*
 * Document-method: column_value
 *
 * call-seq:
 *   view.column_value(id, name) -> 値
 *
 * _view_の_id_に対応するカラム_name_の値を返す。
 */
static VALUE
rb_grn_view_get_column_value (VALUE self, VALUE rb_id, VALUE rb_name)
{
    RbGrnTable *rb_view;
    grn_ctx *context = NULL;
    grn_obj *view, *value, *accessor;
    VALUE rb_value;
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

    return rb_value;
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
}
