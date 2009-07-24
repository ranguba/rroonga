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

VALUE rb_cGrnHash;

/*
 * Document-class: Groonga::Hash < Groonga::Table
 *
 * 各レコードをキーで管理するテーブル。キーと完全一致するレ
 * コードを非常に高速に検索できる。
 */

/*
 * call-seq:
 *   Groonga::Hash.create(options={})                -> Groonga::Hash
 *   Groonga::Hash.create(options={}) {|table| ... }
 *
 * 各レコードをキーで管理するテーブルを生成する。ブロックを指
 * 定すると、そのブロックに生成したテーブルが渡され、ブロック
 * を抜けると自動的にテーブルが破棄される。
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
 *   り、プロセス終了後もレコードは保持される。次回起動時に
 *   Groonga::Hash.openで保存されたレコードを利用することが
 *   できる。省略すると一時テーブルになり、プロセスが終了する
 *   とレコードは破棄される。
 *
 * [+:persistent+]
 *   +true+を指定すると永続テーブルとなる。+path+を省略した
 *   場合は自動的にパスが付加される。+:context+で指定した
 *   Groonga::Contextに結びついているデータベースが一時デー
 *   タベースの場合は例外が発生する。
 *
 * [+:key_type+]
 *   キーの種類を示すオブジェクトを指定する。キーの種類には
 *   Groonga::Typeまたはテーブル（Groonga::Array、
 *   Groonga::Hash、Groonga::PatriciaTrieのどれか）を指定する。
 *
 *   Groonga::Typeを指定した場合は、その型が示す範囲の値をキー
 *   として使用する。ただし、キーの最大サイズは4096バイトで
 *   あるため、Groonga::Type::TEXTやGroonga::Type::LONG_TEXT
 *   は使用できない。
 *
 *   テーブルを指定した場合はレコードIDをキーとして使用する。
 *   指定したテーブルのGroonga::Recordをキーとして使用するこ
 *   ともでき、その場合は自動的にGroonga::Recordからレコード
 *   IDを取得する。
 *
 *   省略した場合は文字列をキーとして使用する。この場合、
 *   4096バイトまで使用可能である。
 *
 * [+:value_size+]
 *   値の大きさを指定する。省略すると0になる。
 *
 * [+:default_tokenizer+]
 *   Groonga::IndexColumnで使用するトークナイザを指定する。
 *   デフォルトでは何も設定されていないので、テーブルに
 *   Groonga::IndexColumnを定義する場合は
 *   <tt>"<token:bigram>"</tt>などを指定する必要がある。
 *
 * [+:sub_records+]
 *   +true+を指定すると#groupでグループ化したときに、
 *   <tt>record[".:nsubrecs"]</tt>でグループに含まれるレコー
 *   ドの件数を取得できる。
 *
 * 使用例:
 *
 * 無名一時テーブルを生成する。
 *   Groonga::Hash.create
 *
 * 無名永続テーブルを生成する。
 *   Groonga::Hash.create(:path => "/tmp/hash.grn")
 *
 * 名前付き永続テーブルを生成する。ただし、ファイル名は気に
 * しない。
 *   Groonga::Hash.create(:name => "<bookmarks>",
 *                        :persistent => true)
 *
 * それぞれのレコードに512バイトの値を格納できる無名一時テー
 * ブルを生成する。
 *   Groonga::Hash.create(:value => 512)
 *
 * キーとして文字列を使用する無名一時テーブルを生成する。
 *   Groonga::Hash.create(:key_type => Groonga::Type::TEXT)
 *
 * キーとして文字列を使用する無名一時テーブルを生成する。
 * （キーの種類を表すオブジェクトは文字列で指定。）
 *   Groonga::Hash.create(:key_type => "<shorttext>")
 *
 * キーとして<tt><bookmarks></tt>テーブルのレコードを使用す
 * る無名一時テーブルを生成する。
 *   bookmarks = Groonga::Hash.create(:name => "<bookmarks>")
 *   Groonga::Hash.create(:key_type => bookmarks)
 *
 * キーとして<tt><bookmarks></tt>テーブルのレコードを使用す
 * る無名一時テーブルを生成する。
 * （テーブルは文字列で指定。）
 *   Groonga::Hash.create(:name => "<bookmarks>")
 *   Groonga::Hash.create(:key_type => "<bookmarks>")
 *
 * 全文検索用のトークンをバイグラムで切り出す無名一時テーブ
 * ルを生成する。
 *   bookmarks = Groonga::Hash.create(:name => "<bookmarks>")
 *   bookmarks.define_column("comment", "<text>")
 *   terms = Groonga::Hash.create(:name => "<terms>",
 *                                :default_tokenizer => "<token:bigram>")
 *   terms.define_index_column("content", bookmarks,
 *                             :source => "<bookmarks>.comment")
 */
static VALUE
rb_grn_hash_s_create (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *key_type = NULL, *value_type = NULL, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_obj_flags flags = GRN_TABLE_HASH_KEY;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_key_type, rb_value_type, rb_default_tokenizer;
    VALUE rb_sub_records;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"name", &rb_name,
                        "path", &rb_path,
			"persistent", &rb_persistent,
			"key_type", &rb_key_type,
			"value_type", &rb_value_type,
			"default_tokenizer", &rb_default_tokenizer,
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

    if (NIL_P(rb_key_type)) {
	flags |= GRN_OBJ_KEY_VAR_SIZE;
    } else {
	key_type = RVAL2GRNOBJECT(rb_key_type, &context);
    }

    if (!NIL_P(rb_value_type))
	value_type = RVAL2GRNOBJECT(rb_value_type, &context);

    if (RVAL2CBOOL(rb_sub_records))
	flags |= GRN_OBJ_WITH_SUBREC;

    table = grn_table_create(context, name, name_size, path,
			     flags, key_type, value_type);
    if (!table)
	rb_grn_context_check(context, rb_ary_new4(argc, argv));
    rb_table = rb_grn_object_alloc(self);
    rb_grn_object_assign(Qnil, rb_table, rb_context, context, table);
    rb_grn_context_check(context, rb_table);

    if (!NIL_P(rb_default_tokenizer))
	rb_funcall(rb_table, rb_intern("default_tokenizer="), 1,
		   rb_default_tokenizer);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

/*
 * call-seq:
 *   hash.search(key, options=nil) -> Groonga::Hash
 *
 * _key_にマッチするレコードのIDがキーに入っている
 * Groonga::Hashを返す。マッチするレコードがない場合は空の
 * Groonga::Hashが返。
 *
 * _options_で+:result+を指定することにより、そのテーブルにマッ
 * チしたレコードIDがキーのレコードを追加することができる。
 * +:result+にテーブルを指定した場合は、そのテーブルが返る。
 *
 * _options_に指定可能な値は以下の通り。
 *
 * [+:result+]
 *   結果を格納するテーブル。
 *
 * 複数のキーで検索し、結果を1つのテーブルに集める。
 *   result = nil
 *   keys = ["morita", "gunyara-kun", "yu"]
 *   keys.each do |key|
 *     result = users.search(key, :result => result)
 *   end
 *   result.each do |record|
 *     user = record.key
 *     p user.key # -> "morita"または"gunyara-kun"または"yu"
 *   end
 */
static VALUE
rb_grn_hash_search (int argc, VALUE *argv, VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *table;
    grn_id domain_id;
    grn_obj *key, *domain, *result;
    VALUE rb_key, options, rb_result;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 &key, &domain_id, &domain,
					 NULL, NULL, NULL);

    rb_scan_args(argc, argv, "11", &rb_key, &options);

    RVAL2GRNKEY(rb_key, context, key, domain_id, domain, self);

    rb_grn_scan_options(options,
			"result", &rb_result,
			NULL);

    if (NIL_P(rb_result)) {
	result = grn_table_create(context, NULL, 0, NULL,
				  GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
				  table, 0);
	rb_grn_context_check(context, self);
	rb_result = GRNOBJECT2RVAL(Qnil, context, result, RB_GRN_TRUE);
    } else {
	result = RVAL2GRNOBJECT(rb_result, &context);
    }

    rc = grn_obj_search(context, table, key,
			result, GRN_OP_OR, NULL);
    rb_grn_rc_check(rc, self);

    return rb_result;
}

void
rb_grn_init_hash (VALUE mGrn)
{
    rb_cGrnHash = rb_define_class_under(mGrn, "Hash", rb_cGrnTable);

    rb_include_module(rb_cGrnHash, rb_mGrnTableKeySupport);

    rb_define_singleton_method(rb_cGrnHash, "create",
			       rb_grn_hash_s_create, -1);

    rb_define_method(rb_cGrnHash, "search", rb_grn_hash_search, -1);
}
