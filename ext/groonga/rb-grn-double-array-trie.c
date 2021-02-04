/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sts=4 sw=4 ts=8 noet: */
/*
  Copyright (C) 2011-2021  Sutou Kouhei <kou@clear-code.com>
  Copyright (C) 2014  Masafumi Yokoyama <myokoym@gmail.com>

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

#define SELF(object) ((RbGrnDoubleArrayTrie *)RTYPEDDATA_DATA(object))

VALUE rb_cGrnDoubleArrayTrie;

/*
 * Document-class: Groonga::DoubleArrayTrie < Groonga::Table
 *
 * It's a table that manages records by double array
 * trie. It can change key without ID change. This feature
 * is supported by only Groonga::DoubleArrayTrie. But it
 * requires large spaces rather than other tables. It is
 * used by {Groonga::Database} for key management
 * internally. It's reasonable choice because number of
 * tables and columns in {Groonga::Database} (number of their
 * names equals to number of keys to be managed by
 * Groonga::DoubleArrayTrie) will be less than number of
 * records of user defined tables.
 *
 * Groonga::DoubleArrayTrie supports exact match search,
 * predictive search and common prefix search like
 * {Groonga::PatriciaTrie} . It also supports cursor API.
 */

static void
rb_grn_double_array_trie_deconstruct (RbGrnDoubleArrayTrie *rb_grn_double_array_trie,
                                      grn_obj **double_array_trie,
                                      grn_ctx **context,
                                      grn_obj **key,
                                      grn_obj **new_key,
                                      grn_id *domain_id,
                                      grn_obj **domain,
                                      grn_obj **value,
                                      grn_id *range_id,
                                      grn_obj **range,
                                      VALUE *columns)
{
    RbGrnTableKeySupport *rb_grn_table_key_support;

    rb_grn_table_key_support =
        RB_GRN_TABLE_KEY_SUPPORT(rb_grn_double_array_trie);

    rb_grn_table_key_support_deconstruct(rb_grn_table_key_support,
                                         double_array_trie, context,
                                         key, domain_id, domain,
                                         value, range_id, range,
                                         columns);

    if (new_key)
        *new_key = rb_grn_double_array_trie->new_key;
}

void
rb_grn_double_array_trie_finalizer (grn_ctx *context,
                                    grn_obj *grn_object,
                                    RbGrnDoubleArrayTrie *rb_grn_double_array_trie)
{
    RbGrnTableKeySupport *rb_grn_table_key_support;

    if (!context)
        return;

    if (rb_grn_double_array_trie->new_key)
        grn_obj_unlink(context, rb_grn_double_array_trie->new_key);
    rb_grn_double_array_trie->new_key = NULL;

    rb_grn_table_key_support =
        RB_GRN_TABLE_KEY_SUPPORT(rb_grn_double_array_trie);
    rb_grn_table_key_support_finalizer(context, grn_object,
                                       rb_grn_table_key_support);
}

void
rb_grn_double_array_trie_bind (RbGrnDoubleArrayTrie *rb_grn_double_array_trie,
                               grn_ctx *context,
                               grn_obj *double_array_trie)
{
    RbGrnObject *rb_grn_object;
    RbGrnTableKeySupport *rb_grn_table_key_support;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_double_array_trie);

    rb_grn_table_key_support =
        RB_GRN_TABLE_KEY_SUPPORT(rb_grn_double_array_trie);
    rb_grn_table_key_support_bind(rb_grn_table_key_support,
                                  context,
                                  double_array_trie);

    rb_grn_double_array_trie->new_key =
        grn_obj_open(context, GRN_BULK, 0, rb_grn_object->domain_id);
}

/*
 * It creates a table that manages records by double array trie.
 * ブロックを指定すると、そのブロックに生成したテーブルが渡さ
 * れ、ブロックを抜けると自動的にテーブルが破棄される。
 *
 * @example
 *   #無名一時テーブルを生成する。
 *   Groonga::DoubleArrayTrie.create
 *
 *   #無名永続テーブルを生成する。
 *   Groonga::DoubleArrayTrie.create(:path => "/tmp/hash.grn")
 *
 *   #名前付き永続テーブルを生成する。ただし、ファイル名は気に
 *   #しない。
 *   Groonga::DoubleArrayTrie.create(:name => "Bookmarks",
 *                                   :persistent => true)
 *
 *   #それぞれのレコードに512バイトの値を格納できる無名一時テー
 *   #ブルを生成する。
 *   Groonga::DoubleArrayTrie.create(:value => 512)
 *
 *   #キーとして文字列を使用する無名一時テーブルを生成する。
 *   Groonga::DoubleArrayTrie.create(:key_type => Groonga::Type::SHORT_TEXT)
 *
 *   #キーとして文字列を使用する無名一時テーブルを生成する。
 *   #（キーの種類を表すオブジェクトは文字列で指定。）
 *   Groonga::DoubleArrayTrie.create(:key_type => "ShortText")
 *
 *   #キーとしてBookmarksテーブルのレコードを使用す
 *   #る無名一時テーブルを生成する。
 *   bookmarks = Groonga::DoubleArrayTrie.create(:name => "Bookmarks")
 *   Groonga::DoubleArrayTrie.create(:key_type => bookmarks)
 *
 *   #キーとしてBookmarksテーブルのレコードを使用す
 *   #る無名一時テーブルを生成する。
 *   #（テーブルは文字列で指定。）
 *   Groonga::DoubleArrayTrie.create(:name => "Bookmarks")
 *   Groonga::DoubleArrayTrie.create(:key_type => "Bookmarks")
 *
 *   #全文検索用のトークンをバイグラムで切り出す無名一時テーブ
 *   #ルを生成する。
 *   bookmarks = Groonga::DoubleArrayTrie.create(:name => "Bookmarks")
 *   bookmarks.define_column("comment", "Text")
 *   terms = Groonga::DoubleArrayTrie.create(:name => "Terms",
 *                                           :default_tokenizer => "TokenBigram")
 *   terms.define_index_column("content", bookmarks,
 *                             :source => "Bookmarks.comment")
 *
 * @overload create(options={})
 *   @return [Groonga::DoubleArrayTrie]
 *   @!macro [new] double-array-trie.create.options
 *     @param [::Hash] options The name and value
 *       pairs. Omitted names are initialized as the default value.
 *     @option options [Groonga::Context] :context (Groonga::Context.default)
 *       テーブルが利用する {Groonga::Context} 。
 *     @option options :name
 *       テーブルの名前。名前をつけると、 {Groonga::Context#[]} に名
 *       前を指定してテーブルを取得することができる。省略すると
 *       無名テーブルになり、テーブルIDでのみ取得できる。
 *     @option options :path
 *       テーブルを保存するパス。パスを指定すると永続テーブルとな
 *       り、プロセス終了後もレコードは保持される。次回起動時に
 *       {Groonga::Context#[]} で保存されたレコードを利用する
 *       ことができる。省略すると一時テーブルになり、プロセスが終
 *       了するとレコードは破棄される。
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
 *     @option options :key_with_sis
 *       +true+ を指定するとキーの文字列の全suffixが自動的に登
 *       録される。
 *     @option options :key_type
 *       キーの種類を示すオブジェクトを指定する。キーの種類には型
 *       名（"Int32"や"ShortText"など）または {Groonga::Type} または
 *       テーブル（ {Groonga::Array} 、{Groonga::Hash} 、
 *       {Groonga::DoubleArrayTrie} のどれか）を指定する。
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
 *
 *     @option options :default_tokenizer
 *       {Groonga::IndexColumn} で使用するトークナイザを指定する。
 *       デフォルトでは何も設定されていないので、テーブルに
 *       {Groonga::IndexColumn} を定義する場合は
 *       @"TokenBigram"@ などを指定する必要がある。
 *
 *     @option options [::Array<String, Groonga::Procedure>, nil]
 *       :token_filters (nil) The token filters to be used in the
 *       table.
 *
 *     @option options :sub_records
 *       +true+ を指定すると {#group} でグループ化したときに、
 *       {Groonga::Record#n_sub_records} でグループに含まれるレコー
 *       ドの件数を取得できる。
 *
 *     @option options [String, Groonga::Procedure, nil] :normalizer
 *       The normalizer that is used by {Groonga::IndexColumn}. You
 *       can specify this by normalizer name as String such as
 *       @"NormalizerAuto"@ or normalizer object.
 *
 *   @!macro double-array-trie.create.options
 * @overload create(options={})
 *   @yield [table]
 *   @!macro double-array-trie.create.options
 */
static VALUE
rb_grn_double_array_trie_s_create (int argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context;
    grn_obj *key_type = NULL, *value_type = NULL, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_table_flags flags = GRN_OBJ_TABLE_DAT_KEY;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_key_normalize, rb_key_with_sis, rb_key_type;
    VALUE rb_value_type;
    VALUE rb_default_tokenizer;
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
                        "key_with_sis", &rb_key_with_sis,
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

    if (RVAL2CBOOL(rb_key_with_sis))
        flags |= GRN_OBJ_KEY_WITH_SIS;

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
 * {fGroonga::Hash} が返る。
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
 *   @return [Groonga::Hash]
 *   @param key [String] レコードにマッチさせる値
 *   @param options [::Hash] The name and value
 *     pairs. Omitted names are initialized as the default value.
 *   @option options :result The result
 *     結果を格納するテーブル。
 *   @option options :operator (Groonga::Operator::OR)
 *     マッチしたレコードをどのように扱うか。指定可能な値は以
 *     下の通り。
 *
 *     - Groonga::Operator::OR :=
 *       マッチしたレコードを追加。すでにレコードが追加され
 *       ている場合は何もしない。 =:
 *     - Groonga::Operator::AND :=
 *       マッチしたレコードのスコアを増加。マッチしなかった
 *       レコードを削除。 =:
 *     - Groonga::Operator::AND_NOT :=
 *       マッチしたレコードを削除。 =:
 *     - Groonga::Operator::ADJUST :=
 *       マッチしたレコードのスコアを増加。 =:
 *     - +:type+ :=
 *       ????? =:
 *
 */
static VALUE
rb_grn_double_array_trie_search (int argc, VALUE *argv, VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *table;
    grn_id domain_id;
    grn_obj *key, *domain, *result;
    grn_operator operator;
    grn_search_optarg search_options;
    grn_bool search_options_is_set = GRN_FALSE;
    VALUE rb_key, options, rb_result, rb_operator, rb_type;

    memset(&search_options, 0, sizeof(grn_search_optarg));

    rb_grn_double_array_trie_deconstruct(SELF(self), &table, &context,
                                         &key, NULL, &domain_id, &domain,
                                         NULL, NULL, NULL,
                                         NULL);

    rb_scan_args(argc, argv, "11", &rb_key, &options);

    RVAL2GRNKEY(rb_key, context, key, domain_id, domain, self);

    rb_grn_scan_options(options,
                        "result", &rb_result,
                        "operator", &rb_operator,
                        "type", &rb_type,
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

    operator = RVAL2GRNOPERATOR(rb_operator);

    rc = grn_obj_search(context, table, key,
                        result, operator,
                        search_options_is_set ? &search_options : NULL);
    rb_grn_rc_check(rc, self);

    return rb_result;
}

static grn_table_cursor *
rb_grn_double_array_trie_open_grn_prefix_cursor (int argc, VALUE *argv,
                                                 VALUE self, grn_ctx **context)
{
    grn_obj *table;
    grn_table_cursor *cursor;
    void *prefix = NULL;
    unsigned prefix_size = 0;
    int offset = 0, limit = -1;
    int flags = GRN_CURSOR_PREFIX;
    VALUE options, rb_prefix, rb_key_bytes, rb_key_bits;
    VALUE rb_order, rb_order_by;
    VALUE rb_greater_than, rb_less_than, rb_offset, rb_limit;

    rb_grn_table_deconstruct((RbGrnTable *)SELF(self), &table, context,
                             NULL, NULL,
                             NULL, NULL, NULL,
                             NULL);

    rb_scan_args(argc, argv, "11", &rb_prefix, &options);

    rb_grn_scan_options(options,
                        "key_bytes", &rb_key_bytes,
                        "key_bites", &rb_key_bits,
                        "offset", &rb_offset,
                        "limit", &rb_limit,
                        "order", &rb_order,
                        "order_by", &rb_order_by,
                        "greater_than", &rb_greater_than,
                        "less_than", &rb_less_than,
                        NULL);

    prefix = StringValuePtr(rb_prefix);
    if (!NIL_P(rb_key_bytes) && !NIL_P(rb_key_bits)) {
        rb_raise(rb_eArgError,
                 "should not specify both :key_bytes and :key_bits once: %s",
                 rb_grn_inspect(rb_ary_new_from_values(argc, argv)));
    } else if (!NIL_P(rb_key_bytes)) {
        prefix_size = NUM2UINT(rb_key_bytes);
    } else if (!NIL_P(rb_key_bits)) {
        prefix_size = NUM2UINT(rb_key_bits);
        flags |= GRN_CURSOR_SIZE_BY_BIT;
    } else {
        prefix_size = RSTRING_LEN(rb_prefix);
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
    if (NIL_P(rb_order_by)) {
    } else if (rb_grn_equal_option(rb_order_by, "id")) {
        flags |= GRN_CURSOR_BY_ID;
    } else if (rb_grn_equal_option(rb_order_by, "key")) {
        flags |= GRN_CURSOR_BY_KEY;
    } else {
        rb_raise(rb_eArgError,
                 "order_by should be one of [:id, :key]: %s",
                 rb_grn_inspect(rb_order_by));
    }

    if (RVAL2CBOOL(rb_greater_than))
        flags |= GRN_CURSOR_GT;
    if (RVAL2CBOOL(rb_less_than))
        flags |= GRN_CURSOR_LT;

    cursor = grn_table_cursor_open(*context, table,
                                   prefix, prefix_size,
                                   NULL, 0,
                                   offset, limit, flags);
    rb_grn_context_check(*context, self);

    return cursor;
}

/*
 * _prefix_ に前方一致検索をするカーソルを生成して返す。ブロッ
 * クを指定すると、そのブロックに生成したカーソルが渡され、ブ
 * ロックを抜けると自動的にカーソルが破棄される。
 *
 * @overload open_prefix_cursor(prefix, options={})
 *   @return [Groonga::DoubleArrayTrieCursor]
 *   @!macro [new] double_array_trie.open_prefix_cursor.arguments
 *     @param [String] prefix 前方一致させる値
 *     @param options [::Hash] The name and value
 *       pairs. Omitted names are initialized as the default value.
 *     @option options :key_bytes
 *      _prefix_ のサイズ（byte）
 *     @option options :key_bits
 *      _prefix_ のサイズ（bit）
 *     @option options :offset
 *       該当する範囲のレコードのうち、(0ベースで) _:offset_ 番目
 *       からレコードを取り出す。
 *     @option options :limit
 *       該当する範囲のレコードのうち、 _:limit_ 件のみを取り出す。
 *       省略された場合または-1が指定された場合は、全件が指定され
 *       たものとみなす。
 *     @option options :order
 *       +:asc+ または +:ascending+ を指定すると昇順にレコードを取
 *       り出す。
 *       +:desc+ または +:descending+ を指定すると降順にレコードを
 *       取り出す。
 *     @option options :order_by (:id)
 *       +:id+ を指定するとID順にレコードを取り出す。（デフォルト）
 *       +:key+指定するとキー順にレコードを取り出す。
 *     @option options :greater_than
 *       +true+ を指定すると _prefix_ で指定した値に一致した [ +key+ ] を
 *       範囲に含まない。
 *     @option options :less_than
 *       +true+ を指定すると _prefix_ で指定した値に一致した [ +key+ ] を
 *       範囲に含まない。
 *   @!macro double_array_trie.open_prefix_cursor.arguments
 * @overload open_prefix_cursor(prefix, options={})
 *   @yield [cursor]
 *   @!macro double_array_trie.open_prefix_cursor.arguments
 */
static VALUE
rb_grn_double_array_trie_open_prefix_cursor (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_table_cursor *cursor;
    VALUE rb_cursor;

    cursor = rb_grn_double_array_trie_open_grn_prefix_cursor(argc, argv,
                                                         self, &context);
    rb_cursor = GRNTABLECURSOR2RVAL(Qnil, context, cursor);
    rb_iv_set(rb_cursor, "@table", self); /* FIXME: cursor should mark table */
    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_cursor, rb_grn_object_close, rb_cursor);
    else
        return rb_cursor;
}

static VALUE
rb_grn_double_array_trie_update_by_id (VALUE self, VALUE rb_id, VALUE rb_new_key)
{
    grn_ctx *context;
    grn_obj *table;
    grn_id id, domain_id;
    grn_obj *new_key, *domain;
    grn_rc rc;

    rb_grn_double_array_trie_deconstruct(SELF(self), &table, &context,
                                         NULL, &new_key, &domain_id, &domain,
                                         NULL, NULL, NULL,
                                         NULL);

    id = NUM2UINT(rb_id);
    RVAL2GRNKEY(rb_new_key, context, new_key, domain_id, domain, self);
    rc = grn_table_update_by_id(context, table, id,
                                GRN_BULK_HEAD(new_key), GRN_BULK_VSIZE(new_key));
    rb_grn_rc_check(rc, self);

    return Qnil;
}

static VALUE
rb_grn_double_array_trie_update_by_key (VALUE self,
                                        VALUE rb_current_key,
                                        VALUE rb_new_key)
{
    grn_ctx *context;
    grn_obj *table;
    grn_id domain_id;
    grn_obj *current_key, *new_key, *domain;
    grn_rc rc;

    rb_grn_double_array_trie_deconstruct(SELF(self), &table, &context,
                                         &current_key, &new_key,
                                         &domain_id, &domain,
                                         NULL, NULL, NULL,
                                         NULL);

    RVAL2GRNKEY(rb_current_key, context, current_key, domain_id, domain, self);
    RVAL2GRNKEY(rb_new_key, context, new_key, domain_id, domain, self);
    rc = grn_table_update(context, table,
                          GRN_BULK_HEAD(current_key),
                          GRN_BULK_VSIZE(current_key),
                          GRN_BULK_HEAD(new_key),
                          GRN_BULK_VSIZE(new_key));
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * Renames key of a record.
 *
 * @overload update(current_key, new_key)
 * @overload update(id, new_key, :id=>true)
 *
 * @since 4.0.5
 */
static VALUE
rb_grn_double_array_trie_update (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_current_key_or_id, rb_new_key, rb_options;
    VALUE rb_option_id;

    rb_scan_args(argc, argv, "21",
                 &rb_current_key_or_id, &rb_new_key, &rb_options);
    rb_grn_scan_options(rb_options,
                        "id", &rb_option_id,
                        NULL);
    if (RVAL2CBOOL(rb_option_id)) {
        VALUE rb_id;
        rb_id = rb_current_key_or_id;
        return rb_grn_double_array_trie_update_by_id(self, rb_id, rb_new_key);
    } else {
        VALUE rb_current_key;
        rb_current_key = rb_current_key_or_id;
        return rb_grn_double_array_trie_update_by_key(self,
                                                      rb_current_key,
                                                      rb_new_key);
    }
}

void
rb_grn_init_double_array_trie (VALUE mGrn)
{
    rb_cGrnDoubleArrayTrie =
        rb_define_class_under(mGrn, "DoubleArrayTrie", rb_cGrnTable);

    rb_include_module(rb_cGrnDoubleArrayTrie, rb_mGrnTableKeySupport);
    rb_define_singleton_method(rb_cGrnDoubleArrayTrie, "create",
                               rb_grn_double_array_trie_s_create, -1);

    rb_define_method(rb_cGrnDoubleArrayTrie, "search",
                     rb_grn_double_array_trie_search, -1);

    rb_define_method(rb_cGrnDoubleArrayTrie, "open_prefix_cursor",
                     rb_grn_double_array_trie_open_prefix_cursor, -1);

    rb_define_method(rb_cGrnDoubleArrayTrie, "update",
                     rb_grn_double_array_trie_update, -1);
}
