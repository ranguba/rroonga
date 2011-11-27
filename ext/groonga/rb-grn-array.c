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

#define SELF(object, context) (RVAL2GRNTABLE(object, context))

VALUE rb_cGrnArray;

/*
 * Document-class: Groonga::Array < Groonga::Table
 *
 * 各レコードがキーに関連付けられていないテーブル。レコードは
 * IDで識別する。
 */

/*
 * call-seq:
 *   Groonga::Array.create(options={})                -> Groonga::Array
 *   Groonga::Array.create(options={}) {|table| ... }
 *
 * キーのないテーブルを生成する。ブロックを指定すると、そのブ
 * ロックに生成したテーブルが渡され、ブロックを抜けると自動的
 * にテーブルが破棄される。
 *
 * @example
 *   #無名一時テーブルを生成する。
 *   Groonga::Array.create
 *
 *   #無名永続テーブルを生成する。
 *   Groonga::Array.create(:path => "/tmp/array.grn")
 *
 *   #名前付き永続テーブルを生成する。ただし、ファイル名は気にしない。
 *   Groonga::Array.create(:name => "Bookmarks",
 *                         :persistent => true)
 *
 *   #それぞれのレコードに512バイトの値を格納できる無名一時テーブルを生成する。
 *   Groonga::Array.create(:value => 512)
 *
 * @param [::Hash] options The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options [Grrnga::Context] :context (Groonga::Context.default) The context
 *   テーブルが利用するGrrnga::Context
 * @option options :name The name
 *   テーブルの名前。名前をつけると、Groonga::Context#[]に名
 *   前を指定してテーブルを取得することができる。省略すると
 *   無名テーブルになり、テーブルIDでのみ取得できる。
 * @option options :path The path
 *   テーブルを保存するパス。パスを指定すると永続テーブルとな
 *   り、プロセス終了後もレコードは保持される。次回起動時に
 *   Groonga::Context#[]で保存されたレコードを利用することが
 *   できる。省略すると一時テーブルになり、プロセスが終了する
 *   とレコードは破棄される。
 * @option options :persistent The persistent
 *   +true+ を指定すると永続テーブルとなる。 +path+ を省略した
 *   場合は自動的にパスが付加される。 +:context+ で指定した
 *   Groonga::Contextに結びついているデータベースが一時デー
 *   タベースの場合は例外が発生する。
 * @option options :value_type (nil) The value_type
 *   値の型を指定する。省略すると値のための領域を確保しない。
 *   値を保存したい場合は必ず指定すること。
 *   参考: Groonga::Type.new
 * @option options [Groonga::Record#n_sub_records] :sub_records The sub_records
 *   +true+ を指定すると#groupでグループ化したときに、
 *   Groonga::Record#n_sub_recordsでグループに含まれるレコー
 *   ドの件数を取得できる。
 */
static VALUE
rb_grn_array_s_create (int argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context = NULL;
    grn_obj *value_type = NULL, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_obj_flags flags = GRN_OBJ_TABLE_NO_KEY;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_value_type, rb_sub_records;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"name", &rb_name,
                        "path", &rb_path,
			"persistent", &rb_persistent,
			"value_type", &rb_value_type,
			"sub_records", &rb_sub_records,
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

    if (!NIL_P(rb_value_type))
	value_type = RVAL2GRNOBJECT(rb_value_type, &context);

    if (RVAL2CBOOL(rb_sub_records))
	flags |= GRN_OBJ_WITH_SUBREC;

    table = grn_table_create(context, name, name_size, path,
			     flags, NULL, value_type);
    if (!table)
	rb_grn_context_check(context, rb_ary_new4(argc, argv));
    rb_table = GRNOBJECT2RVAL(klass, context, table, GRN_TRUE);
    rb_grn_context_check(context, rb_table);
    rb_iv_set(rb_table, "@context", rb_context);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

/*
 * call-seq:
 *   array.add(values=nil) -> Groonga::Recordまたはnil
 *
 * レコード追加し、追加したレコードを返す。レコードの追加に失
 * 敗した場合は +nil+ を返す。
 *
 * _values_ にはレコードのカラムに設定する値を指定する。省略
 * した場合または +nil+ を指定した場合はカラムは設定しない。カ
 * ラムの値は<tt>{:カラム名1 => 値1, :カラム名2 => 値2,
 * ...}</tt>と指定する。
 *
 * @example
 *   #以下のようなユーザを格納するGroonga::Arrayが
 *   #定義されているものとする。
 *   users = Groonga::Array.create(:name => "Users")
 *   users.define_column("name", "ShortText")
 *   users.define_column("uri", "ShortText")
 *   #ユーザを追加する。
 *   user = users.add
 *
 *   #daijiroユーザを追加する。
 *   daijiro = users.add(:name => "daijiro")
 *
 *   #gunyara-kunユーザを追加する。
 *   gunyara_kun = users.add(:name => "gunyara-kun",
 *                           :uri => "http://d.hatena.ne.jp/tasukuchan/")
 */
static VALUE
rb_grn_array_add (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    grn_id id;
    VALUE values;

    rb_scan_args(argc, argv, "01", &values);

    table = SELF(self, &context);

    id = grn_table_add(context, table, NULL, 0, NULL);
    rb_grn_context_check(context, self);

    if (GRN_ID_NIL == id) {
	return Qnil;
    } else {
	return rb_grn_record_new_added(self, id, values);
    }
}

void
rb_grn_init_array (VALUE mGrn)
{
    rb_cGrnArray = rb_define_class_under(mGrn, "Array", rb_cGrnTable);

    rb_define_singleton_method(rb_cGrnArray, "create",
			       rb_grn_array_s_create, -1);

    rb_define_method(rb_cGrnArray, "add", rb_grn_array_add, -1);
}
