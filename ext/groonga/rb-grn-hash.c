/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2021  Sutou Kouhei <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "rb-grn.h"

#define SELF(object) ((RbGrnTableKeySupport *)RTYPEDDATA_DATA(object))

VALUE rb_cGrnHash;

/*
 * Document-class: Groonga::Hash < Groonga::Table
 *
 * 各レコードをキーで管理するテーブル。キーと完全一致するレ
 * コードを非常に高速に検索できる。
 */

/*
 * 各レコードをキーで管理するテーブルを生成する。ブロックを指
 * 定すると、そのブロックに生成したテーブルが渡され、ブロック
 * を抜けると自動的にテーブルが破棄される。
 *
 * @example
 *   #無名一時テーブルを生成する。
 *   Groonga::Hash.create
 *
 *   #無名永続テーブルを生成する。
 *   Groonga::Hash.create(:path => "/tmp/hash.grn")
 *
 *   #名前付き永続テーブルを生成する。ただし、ファイル名は気にしない。
 *   Groonga::Hash.create(:name => "Bookmarks",
 *                        :persistent => true)
 *
 *   #それぞれのレコードに512バイトの値を格納できる無名一時テーブルを生成する。
 *   Groonga::Hash.create(:value => 512)
 *
 *   #キーとして文字列を使用する無名一時テーブルを生成する。
 *   Groonga::Hash.create(:key_type => Groonga::Type::SHORT_TEXT)
 *
 *   #キーとして文字列を使用する無名一時テーブルを生成する。
 *   （キーの種類を表すオブジェクトは文字列で指定。）
 *   Groonga::Hash.create(:key_type => "ShortText")
 *
 *   #キーとしてBookmarksテーブルのレコードを使用す
 *   る無名一時テーブルを生成する。
 *   bookmarks = Groonga::Hash.create(:name => "Bookmarks")
 *   Groonga::Hash.create(:key_type => bookmarks)
 *
 *   #キーとしてBookmarksテーブルのレコードを使用す
 *   #る無名一時テーブルを生成する。（テーブルは文字列で指定。）
 *   Groonga::Hash.create(:name => "Bookmarks")
 *   Groonga::Hash.create(:key_type => "Bookmarks")
 *
 *   #全文検索用のトークンをバイグラムで切り出す無名一時テーブ
 *   #ルを生成する。
 *   bookmarks = Groonga::Hash.create(:name => "Bookmarks")
 *   bookmarks.define_column("comment", "Text")
 *   terms = Groonga::Hash.create(:name => "Terms",
 *                                :default_tokenizer => "TokenBigram")
 *   terms.define_index_column("content", bookmarks,
 *                             :source => "Bookmarks.comment")
 *
 * @overload create(options={})
 *   @return [Groonga::Hash]
 *   @!macro hash.create.options
 *     @param [::Hash] options The name and value
 *       pairs. Omitted names are initialized as the default value
 *     @option options [Groonga::Context] :context (Groonga::Context.default)
 *       テーブルが利用する {Groonga::Context} 。省略すると
 *       {Groonga::Context.default} を用いる。
 *     @option options [Groonga::Context#[]] :name
 *       テーブルの名前。名前をつけると、 {Groonga::Context#[]} に名
 *       前を指定してテーブルを取得することができる。省略すると
 *       無名テーブルになり、テーブルIDでのみ取得できる。
 *     @option options [Groonga::Context#[]] :path
 *       テーブルを保存するパス。パスを指定すると永続テーブルとな
 *       り、プロセス終了後もレコードは保持される。次回起動時に
 *       {Groonga::Context#[]} で保存されたレコードを利用することが
 *       できる。省略すると一時テーブルになり、プロセスが終了する
 *       とレコードは破棄される。
 *     @option options :persistent
 *       +true+ を指定すると永続テーブルとなる。 +path+ を省略した
 *       場合は自動的にパスが付加される。 +:context+ で指定した
 *       {Groonga::Context} に結びついているデータベースが一時デー
 *       タベースの場合は例外が発生する。
 *
 *     @option options :key_normalize (false) Keys are normalized
 *       if this value is @true@.
 *
 *       @deprecated Use @:normalizer => "NormalizerAuto"@ instead.
 *
 *     @option options [Boolean] :key_large (false)
 *       It specifies whether total key size is large or not. The
 *       default total key size is 4GiB. Large total key size is 1TiB.
 *
 *       @since 6.0.1
 *
 *     @option options :key_type
 *       キーの種類を示すオブジェクトを指定する。キーの種類には型
 *       名（"Int32"や"ShortText"など）または {Groonga::Type} または
 *       テーブル（ {Groonga::Array} 、 {Groonga::Hash} 、
 *       {Groonga::PatriciaTrie} のどれか）を指定する。
 *
 *       {Groonga::Type} を指定した場合は、その型が示す範囲の値をキー
 *       として使用する。ただし、キーの最大サイズは4096バイトで
 *       あるため、 {Groonga::Type::TEXT} や {Groonga::Type::LONG_TEXT}
 *       は使用できない。
 *
 *       テーブルを指定した場合はレコードIDをキーとして使用する。
 *       指定したテーブルの {Groonga::Record} をキーとして使用するこ
 *       ともでき、その場合は自動的に {Groonga::Record} からレコード
 *       IDを取得する。
 *
 *       省略した場合はShortText型をキーとして使用する。この場合、
 *       4096バイトまで使用可能である。
 *     @option options :value_type
 *       値の型を指定する。省略すると値のための領域を確保しない。
 *       値を保存したい場合は必ず指定すること。
 *
 *       参考: {Groonga::Type.new}
 *     @option options [Groonga::IndexColumn] :default_tokenizer
 *       {Groonga::IndexColumn} で使用するトークナイザを指定する。
 *       デフォルトでは何も設定されていないので、テーブルに
 *       {Groonga::IndexColumn} を定義する場合は
 *       @"TokenBigram"@ などを指定する必要がある。
 *
 *     @option options [::Array<String, Groonga::Procedure>, nil]
 *       :token_filters (nil) The token filters to be used in the
 *       table.
 *
 *     @option options [Groonga::Record#n_sub_records] :sub_records
 *       +true+ を指定すると {#group} でグループ化したときに、
 *       {Groonga::Record#n_sub_records} でグループに含まれるレコー
 *       ドの件数を取得できる。
 *
 *     @option options [String, Groonga::Procedure, nil] :normalizer
 *       The normalizer that is used by {Groonga::IndexColumn}. You
 *       can specify this by normalizer name as String such as
 *       @"NormalizerAuto"@ or normalizer object.
 *
 *   @!macro hash.create.options
 * @overload create(options={})
 *   @yield [table]
 *   @!macro hash.create.options
 */
static VALUE
rb_grn_hash_s_create (int argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context;
    grn_obj *key_type = NULL, *value_type = NULL, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_table_flags flags = GRN_OBJ_TABLE_HASH_KEY;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_key_normalize;
    VALUE rb_key_large;
    VALUE rb_key_type, rb_value_type, rb_default_tokenizer;
    VALUE rb_token_filters;
    VALUE rb_sub_records;
    VALUE rb_normalizer;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
                        "context", &rb_context,
                        "name", &rb_name,
                        "path", &rb_path,
                        "persistent", &rb_persistent,
                        "key_normalize", &rb_key_normalize,
                        "key_large", &rb_key_large,
                        "key_type", &rb_key_type,
                        "value_type", &rb_value_type,
                        "default_tokenizer", &rb_default_tokenizer,
                        "token_filters", &rb_token_filters,
                        "sub_records", &rb_sub_records,
                        "normalizer", &rb_normalizer,
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

    if (RVAL2CBOOL(rb_key_large))
        flags |= GRN_OBJ_KEY_LARGE;

    if (NIL_P(rb_key_type)) {
        key_type = grn_ctx_at(context, GRN_DB_SHORT_TEXT);
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
        rb_grn_context_check(context, rb_ary_new_from_values(argc, argv));
    rb_table = GRNOBJECT2RVAL(klass, context, table, GRN_TRUE);

    if (!NIL_P(rb_default_tokenizer))
        rb_funcall(rb_table, rb_intern("default_tokenizer="), 1,
                   rb_default_tokenizer);
    if (!NIL_P(rb_token_filters))
        rb_funcall(rb_table, rb_intern("token_filters="), 1,
                   rb_token_filters);
    if (!NIL_P(rb_normalizer))
        rb_funcall(rb_table, rb_intern("normalizer="), 1,
                   rb_normalizer);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

/*
 * _key_ にマッチするレコードのIDがキーに入っている
 * {Groonga::Hash} を返す。マッチするレコードがない場合は空の
 * {Groonga::Hash} が返る。
 *
 * _options_ で +:result+ を指定することにより、そのテーブルにマッ
 * チしたレコードIDがキーのレコードを追加することができる。
 * +:result+ にテーブルを指定した場合は、そのテーブルが返る。
 *
 * @example 複数のキーで検索し、結果を1つのテーブルに集める。
 *   result = nil
 *   keys = ["morita", "gunyara-kun", "yu"]
 *   keys.each do |key|
 *     result = users.search(key, :result => result)
 *   end
 *   result.each do |record|
 *     user = record.key
 *     p user.key # -> "morita"または"gunyara-kun"または"yu"
 *   end
 *
 * @overload search(key, options=nil)
 *   @param [::Hash] options The name and value
 *     pairs. Omitted names are initialized as the default value
 *   @option options :result
 *     結果を格納するテーブル。
 * @return [Groonga::Hash]
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
                                         NULL, NULL, NULL,
                                         NULL);

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
        rb_result = GRNOBJECT2RVAL(Qnil, context, result, GRN_TRUE);
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
