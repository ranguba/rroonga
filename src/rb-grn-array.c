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

VALUE rb_cGrnArray;

/*
 * Document-class: Groonga::Array < Groonga::Table
 *
 * 各レコードにキーが存在しないテーブル。レコードはIDで識別
 * する。
 */

/*
 * call-seq:
 *   Groonga::Array.create(options={})            -> Groonga::Array
 *   Groonga::Array.create(options={}) {|table| ... }
 *
 * キーのないテーブルを生成する。ブロックを指定すると、そのブ
 * ロックに生成したテーブルが渡され、ブロックを抜けると自動的
 * にテーブルが破棄される。
 *
 * _options_に指定可能な値は以下の通り。
 *
 * [+:context+]
 *   テーブルが利用するGroonga::Context。省略すると
 *   Groonga::Context.defaultを用いる。
 *
 * [+:name+]
 *   テーブルの名前。名前をつけると、Groonga::Context#[]に名
 *   前を指定してテーブルを取得することができる。省略すると
 *   無名テーブルになり、テーブルIDでのみ取得できる。
 *
 * [+:path+]
 *   テーブルを保存するパス。パスを指定すると永続テーブルとな
 *   り、プロセス終了後もレコードは保持され、次回起動時に
 *   Groonga::Array.openで保存されたレコードを利用することが
 *   できる。省略すると一時テーブルになり、プロセスが終了する
 *   とレコードは破棄される。
 *
 * [+:persistent+]
 *   +true+を指定すると永続テーブルとなる。+path+を省略した
 *   場合は自動的にパスが付加される。+:context+で指定した
 *   Groonga::Contextに結びついているデータベースが一時デー
 *   タベースの場合は例外が発生する。
 *
 * [+:value_size+]
 *   値の大きさを指定する。省略すると0になる。
 *
 * 無名一時テーブルを生成する。
 *   Groonga::Array.create
 *
 * 無名永続テーブルを生成する。
 *   Groonga::Array.create(:path => "/tmp/array.grn")
 *
 * 名前付き永続テーブルを生成する。ただし、ファイル名は気に
 * しない。
 *   Groonga::Array.create(:name => "<bookmarks>",
 *                         :persistent => true)
 *
 * それぞれのレコードに512バイトの値を格納できる無名一時テー
 * ブルを生成する。
 *   Groonga::Array.create(:value => 512)
 */
static VALUE
rb_grn_array_s_create (int argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context;
    grn_obj *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0, value_size = 0;
    grn_obj_flags flags = GRN_TABLE_NO_KEY;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_value_size;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"name", &rb_name,
                        "path", &rb_path,
			"persistent", &rb_persistent,
			"value_size", &rb_value_size,
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

    if (!NIL_P(rb_value_size))
	value_size = NUM2UINT(rb_value_size);

    table = grn_table_create(context, name, name_size, path,
			     flags, NULL, value_size,
			     GRN_ENC_NONE);
    rb_table = GRNOBJECT2RVAL(klass, context, table);
    rb_grn_context_check(context, rb_table);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

/*
 * call-seq:
 *  array.add -> Groonga::Record
 *
 * 新しくレコードを生成します。
 */
static VALUE
rb_grn_array_add (VALUE self)
{
    grn_ctx *context;
    grn_id id;

    context = rb_grn_object_ensure_context(self, Qnil);

    id = grn_table_add(context, SELF(self));
    rb_grn_context_check(context, self);

    if (GRN_ID_NIL == id)
	return Qnil;
    else
	return rb_grn_record_new(self, id);
}

void
rb_grn_init_array (VALUE mGrn)
{
    rb_cGrnArray = rb_define_class_under(mGrn, "Array", rb_cGrnTable);

    rb_define_singleton_method(rb_cGrnArray, "create",
			       rb_grn_array_s_create, -1);

    rb_define_method(rb_cGrnArray, "add", rb_grn_array_add, 0);
}
