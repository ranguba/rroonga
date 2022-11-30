/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sts=4 sw=4 ts=8 noet: */
/*
  Copyright (C) 2009-2022  Sutou Kouhei <kou@clear-code.com>

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

VALUE rb_cGrnPatriciaTrie;

/*
 * Document-class: Groonga::PatriciaTrie < Groonga::Table
 *
 * 各レコードをパトリシアトライで管理するテーブル。ハッシュ
 * テーブルに比べて完全一致検索の速度がやや遅いが、前方一致
 * 検索・共通接頭辞探索などの検索ができる。またカーソルを用
 * いてキーの昇降順にレコードを取り出すことができる。
 */

/*
 * 各レコードをパトリシアトライで管理するテーブルを生成する。
 * ブロックを指定すると、そのブロックに生成したテーブルが渡さ
 * れ、ブロックを抜けると自動的にテーブルが破棄される。
 *
 * @example
 *   #無名一時テーブルを生成する。
 *   Groonga::PatriciaTrie.create
 *
 *   #無名永続テーブルを生成する。
 *   Groonga::PatriciaTrie.create(:path => "/tmp/hash.grn")
 *
 *   #名前付き永続テーブルを生成する。ただし、ファイル名は気に
 *   #しない。
 *   Groonga::PatriciaTrie.create(:name => "Bookmarks",
 *                                :persistent => true)
 *
 *   #それぞれのレコードに512バイトの値を格納できる無名一時テー
 *   #ブルを生成する。
 *   Groonga::PatriciaTrie.create(:value => 512)
 *
 *   #キーとして文字列を使用する無名一時テーブルを生成する。
 *   Groonga::PatriciaTrie.create(:key_type => Groonga::Type::SHORT_TEXT)
 *
 *   #キーとして文字列を使用する無名一時テーブルを生成する。
 *   #（キーの種類を表すオブジェクトは文字列で指定。）
 *   Groonga::PatriciaTrie.create(:key_type => "ShortText")
 *
 *   #キーとしてBookmarksテーブルのレコードを使用す
 *   #る無名一時テーブルを生成する。
 *   bookmarks = Groonga::PatriciaTrie.create(:name => "Bookmarks")
 *   Groonga::PatriciaTrie.create(:key_type => bookmarks)
 *
 *   #キーとしてBookmarksテーブルのレコードを使用す
 *   #る無名一時テーブルを生成する。
 *   #（テーブルは文字列で指定。）
 *   Groonga::PatriciaTrie.create(:name => "Bookmarks")
 *   Groonga::PatriciaTrie.create(:key_type => "Bookmarks")
 *
 *   #全文検索用のトークンをバイグラムで切り出す無名一時テーブ
 *   #ルを生成する。
 *   bookmarks = Groonga::PatriciaTrie.create(:name => "Bookmarks")
 *   bookmarks.define_column("comment", "Text")
 *   terms = Groonga::PatriciaTrie.create(:name => "Terms",
 *                                        :default_tokenizer => "TokenBigram")
 *   terms.define_index_column("content", bookmarks,
 *                             :source => "Bookmarks.comment")
 *
 * @overload create(options={})
 *   @return [Groonga::PatriciaTrie]
 *   @!macro [new] patricia_trie.create.options
 *     @param options [::Hash] The name and value
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
 *
 *     @option options :key_size (nil) The key size in bytes or the
 *       max key size in bytes. If `key_variable_size: true` is also
 *       specified, the size is the max key size not the key size.
 *
 *       This is used only when the `:context` isn't associated with a
 *       database. If `:context` is associated with a database, this
 *       value is ignored.
 *
 *       @see :key_type
 *       @see :key_variable_size
 *       @since 12.0.8
 *
 *     @option options :key_variable_size (false) Whether the key is
 *       variable size or not.
 *
 *       This is used only when the `:context` isn't associated with a
 *       database. If `:context` is associated with a database, this
 *       value is ignored.
 *
 *       @see :key_type
 *       @see :key_size
 *       @since 12.0.8
 *
 *     @option options :value_type
 *       値の型を指定する。省略すると値のための領域を確保しない。
 *       値を保存したい場合は必ず指定すること。
 *
 *       参考: {Groonga::Type.new}
 *
 *     @option options :value_size (nil) The value size in bytes.
 *
 *       This is used only when the `:context` isn't associated with a
 *       database. If `:context` is associated with a database, this
 *       value is ignored.
 *
 *       @see :value_type
 *       @since 12.0.8
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
 *   @!macro patricia_trie.create.options
 * @overload create(options={})
 *   @yield [table]
 *   @!macro patricia_trie.create.options
 */
static VALUE
rb_grn_patricia_trie_s_create (int argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context;
    grn_obj *key_type = NULL, *value_type = NULL, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_table_flags flags = GRN_OBJ_TABLE_PAT_KEY;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_key_normalize, rb_key_with_sis, rb_key_type;
    VALUE rb_key_size;
    VALUE rb_key_variable_size;
    VALUE rb_value_type;
    VALUE rb_value_size;
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
                        "key_size", &rb_key_size,
                        "key_variable_size", &rb_key_variable_size,
                        "value_type", &rb_value_type,
                        "value_size", &rb_value_size,
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

    if (grn_ctx_db(context)) {
        table = grn_table_create(context, name, name_size, path,
                                 flags, key_type, value_type);
        if (!table)
            rb_grn_context_check(context, rb_ary_new_from_values(argc, argv));
    } else {
        unsigned int key_size = NUM2UINT(rb_key_size);
        unsigned int value_size =
            RB_NIL_P(rb_value_size) ? 0 : NUM2UINT(rb_value_size);
        if (RVAL2CBOOL(rb_key_variable_size)) {
            flags |= GRN_OBJ_KEY_VAR_SIZE;
        }
        table = (grn_obj *)grn_pat_create(context,
                                          path,
                                          key_size,
                                          value_size,
                                          flags);
        if (!table)
            rb_grn_context_check(context, rb_ary_new_from_values(argc, argv));
    }
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
 * _key_ にマッチするレコードのIDがキーに入っている {Groonga::Hash} を返す。
 * マッチするレコードがない場合は空の {Groonga::Hash} が返る。
 *
 * _options_ で +:result+ を指定することにより、そのテーブルにマッチした
 * レコードIDがキーのレコードを追加することができる。
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
 *   @param options [::Hash] The name and value
 *     pairs. Omitted names are initialized as the default value.
 *   @option options :result
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
 *   @return [Groonga::Hash]
 */
static VALUE
rb_grn_patricia_trie_search (int argc, VALUE *argv, VALUE self)
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

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
                                         &key, &domain_id, &domain,
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

/*
 * _string_ を走査し、 _patricia_trie_ 内に格納されているキーに
 * マッチした部分文字列の情報をブロックに渡す。複数のキーが
 * マッチする場合は最長一致するキーを優先する。
 *
 * ブロックを指定しない場合は、マッチした部分文字列の情報を
 * まとめて配列として返す。
 *
 * @example
 *   words = Groonga::PatriciaTrie.create(:key_type => "ShortText",
 *                                        :normalizer => "NormalizerAuto")
 *   words.add("リンク")
 *   adventure_of_link = words.add('リンクの冒険')
 *   words.add('冒険')
 *   gaxtu = words.add('ｶﾞｯ')
 *   muteki = words.add('ＭＵＴＥＫＩ')
 *
 *   text = 'muTEki リンクの冒険 ミリバール ガッ'
 *   words.scan(text).each do |record, word, start, length|
 *     p [record.key, word, start, length]
 *       # -> ["ＭＵＴＥＫＩ", "muTEki", 0, 6]
 *       # -> ["リンクの冒険", "リンクの冒険", 7, 18]
 *       # -> ["ｶﾞｯ", "ガッ", 42, 6]
 *   end
 *
 *   words.scan(text)
 *     # -> [[muteki, "muTEki", 0, 6],
 *     #     [adventure_of_link, "リンクの冒険", 7, 18],
 *     #     [gaxtu, "ガッ", 42, 6]]
 *
 * @overload scan(string)
 *   @return [Array]
 * @overload scan(string)
 *   @yield [record, word, start, length]
 *   @yieldparam record マッチしたキーの {Groonga::Record} 。
 *   @yieldparam word マッチした部分文字列。
 *   @yieldparam start _string_ 内での _word_ の出現位置。（バイト単位）
 *   @yieldparam length _word_ の長さ。（バイト単位）
 */
static VALUE
rb_grn_patricia_trie_scan (VALUE self, VALUE rb_string)
{
    grn_ctx *context;
    grn_obj *table;
    VALUE rb_result = Qnil;
    grn_pat_scan_hit hits[1024];
    const char *string;
    long string_length;
    const char *original_string;
    long rest_offset;
    grn_bool block_given;

    string = StringValuePtr(rb_string);
    string_length = RSTRING_LEN(rb_string);
    original_string = string;
    rest_offset = 0;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
                                         NULL, NULL, NULL,
                                         NULL, NULL, NULL,
                                         NULL);

    block_given = rb_block_given_p();
    if (!block_given)
        rb_result = rb_ary_new();

    while (string_length > 0) {
        const char *rest;
        int i, n_hits;
        unsigned int previous_offset = 0;

        n_hits = grn_pat_scan(context, (grn_pat *)table,
                              string, string_length,
                              hits, sizeof(hits) / sizeof(*hits),
                              &rest);
        for (i = 0; i < n_hits; i++) {
            VALUE record, term, matched_info;

            if (hits[i].offset < previous_offset)
                continue;

            record = rb_grn_record_new(self, hits[i].id, Qnil);
            term = rb_grn_context_rb_string_new(context,
                                                string + hits[i].offset,
                                                hits[i].length);
            matched_info = rb_ary_new_from_args(4,
                                                record,
                                                term,
                                                UINT2NUM(hits[i].offset + rest_offset),
                                                UINT2NUM(hits[i].length));
            if (block_given) {
                rb_yield(matched_info);
            } else {
                rb_ary_push(rb_result, matched_info);
            }
            previous_offset = hits[i].offset;
        }
        rest_offset = rest - original_string;
        string_length -= rest - string;
        string = rest;
    }

    return rb_result;
}

/*
 * キーが _prefix_ に前方一致するレコードのIDがキーに入っている
 * {Groonga::Hash} を返す。マッチするレコードがない場合は空の
 * {Groonga::Hash} が返る。
 *
 * @overload prefix_search(prefix)
 *   @return [Groonga::Hash]
 */
static VALUE
rb_grn_patricia_trie_prefix_search (VALUE self, VALUE rb_prefix)
{
    grn_ctx *context;
    grn_obj *table, *key, *domain, *result;
    grn_id domain_id;
    VALUE rb_result;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
                                         &key, &domain_id, &domain,
                                         NULL, NULL, NULL,
                                         NULL);

    result = grn_table_create(context, NULL, 0, NULL,
                              GRN_OBJ_TABLE_HASH_KEY,
                              table, 0);
    rb_grn_context_check(context, self);
    rb_result = GRNOBJECT2RVAL(Qnil, context, result, GRN_TRUE);

    GRN_BULK_REWIND(key);
    RVAL2GRNKEY(rb_prefix, context, key, domain_id, domain, self);
    grn_pat_prefix_search(context, (grn_pat *)table,
                          GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key),
                          (grn_hash *)result);
    rb_grn_context_check(context, self);

    return rb_result;
}

/*
 * キーを登録するときに文字列の全suffixも一緒に登録する場合
 * は +true+ 、登録しない場合は +false+ を返す。
 *
 * @overload register_key_with_sis?
 */
static VALUE
rb_grn_patricia_trie_register_key_with_sis_p (VALUE self)
{
    grn_obj *table;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, NULL,
                                         NULL, NULL, NULL,
                                         NULL, NULL, NULL,
                                         NULL);
    return CBOOL2RVAL(table->header.flags & GRN_OBJ_KEY_WITH_SIS);
}

static grn_table_cursor *
rb_grn_patricia_trie_open_grn_prefix_cursor (int argc, VALUE *argv, VALUE self,
                                             grn_ctx **context)
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
 *   @return [Groonga::PatriciaTrieCursor]
 *   @!macro [new] patricia-trie.open_prefix_cursor.options
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
 *   @!macro patricia-trie.open_prefix_cursor.options
 * @overload open_prefix_cursor(prefix, options={})
 *   @yield [cursor]
 *   @!macro patricia-trie.open_prefix_cursor.options
 */
static VALUE
rb_grn_patricia_trie_open_prefix_cursor (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_table_cursor *cursor;
    VALUE rb_cursor;

    cursor = rb_grn_patricia_trie_open_grn_prefix_cursor(argc, argv,
                                                         self, &context);
    rb_cursor = GRNTABLECURSOR2RVAL(Qnil, context, cursor);
    rb_iv_set(rb_cursor, "@table", self); /* FIXME: cursor should mark table */
    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_cursor, rb_grn_object_close, rb_cursor);
    else
        return rb_cursor;
}

static grn_table_cursor *
rb_grn_patricia_trie_open_grn_rk_cursor (int argc, VALUE *argv, VALUE self,
                                             grn_ctx **context)
{
    grn_obj *table;
    grn_table_cursor *cursor;
    void *prefix = NULL;
    unsigned prefix_size = 0;
    int offset = 0, limit = -1;
    int flags = GRN_CURSOR_PREFIX | GRN_CURSOR_RK;
    VALUE options, rb_prefix, rb_key_bytes, rb_key_bits;
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
 * _table_ のキーはカタカナである必要がある。また、エンコーディ
 * ングはUTF-8である必要がある。ローマ字やひらがなで _key_ を指
 * 定しても、 _key_ に対応するカタカナのキーを検索するカーソル
 * を生成して返す。ブロックを指定すると、そのブロックに生成し
 * たカーソルが渡され、ブロックを抜けると自動的にカーソルが破
 * 棄される。
 *
 * @overload open_rk_cursor(key, options={})
 *   @return [Groonga::PatriciaTrieCursor]
 *   @!macro [new] patricia-trie.open_rk_cursor.options
 *     @param options [::Hash]
 *       pairs. Omitted names are initialized as the default value.
 *     @option options :key_bytes
 *      _key_のサイズ（byte）
 *     @option options :key_bits
 *      _key_のサイズ（bit）（現在は未サポート）
 *     @option options :offset
 *       該当する範囲のレコードのうち、(0ベースで) _:offset_ 番目から
 *       レコードを取り出す。
 *     @option options :limit
 *       該当する範囲のレコードのうち、 _:limit_ 件のみを取り出す。
 *       省略された場合または-1が指定された場合は、全件が指定されたものとみなす。
 *     @option options :greater_than
 *       +true+ を指定すると _key_ で指定した値に一致した [ +key+ ] を
 *       範囲に含まない。
 *     @option options :less_than
 *       +true+ を指定すると _key_ で指定した値に一致した [ +key+ ] を
 *       範囲に含まない。
 *   @!macro patricia-trie.open_rk_cursor.options
 * @overload open_rk_cursor(key, options={})
 *   @yield [cursor]
 *   @!macro patricia-trie.open_rk_cursor.options
 */
static VALUE
rb_grn_patricia_trie_open_rk_cursor (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_table_cursor *cursor;
    VALUE rb_cursor;

    cursor = rb_grn_patricia_trie_open_grn_rk_cursor(argc, argv,
                                                                self, &context);
    rb_cursor = GRNTABLECURSOR2RVAL(Qnil, context, cursor);
    rb_iv_set(rb_cursor, "@table", self); /* FIXME: cursor should mark table */
    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_cursor, rb_grn_object_close, rb_cursor);
    else
        return rb_cursor;
}


static grn_table_cursor *
rb_grn_patricia_trie_open_grn_near_cursor (int argc, VALUE *argv, VALUE self,
                                           grn_ctx **context, int flags)
{
    grn_obj *table;
    grn_id domain_id;
    grn_obj *key, *domain;
    grn_table_cursor *cursor;
    unsigned min_size = 0;
    int offset = 0, limit = -1;
    VALUE options, rb_key, rb_min_size;
    VALUE rb_greater_than, rb_less_than, rb_offset, rb_limit;

    flags |= GRN_CURSOR_PREFIX;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, context,
                                         &key, &domain_id, &domain,
                                         NULL, NULL, NULL,
                                         NULL);

    rb_scan_args(argc, argv, "11", &rb_key, &options);

    rb_grn_scan_options(options,
                        "size", &rb_min_size,
                        "offset", &rb_offset,
                        "limit", &rb_limit,
                        "greater_than", &rb_greater_than,
                        "less_than", &rb_less_than,
                        NULL);

    RVAL2GRNBULK_WITH_TYPE(rb_key, *context, key, domain_id, domain);

    if (!NIL_P(rb_min_size))
        min_size = NUM2UINT(rb_min_size);
    if (!NIL_P(rb_offset))
        offset = NUM2INT(rb_offset);
    if (!NIL_P(rb_limit))
        limit = NUM2INT(rb_limit);

    if (RVAL2CBOOL(rb_greater_than))
        flags |= GRN_CURSOR_GT;
    if (RVAL2CBOOL(rb_less_than))
        flags |= GRN_CURSOR_LT;

    cursor = grn_table_cursor_open(*context, table,
                                   NULL, min_size,
                                   GRN_BULK_HEAD(key), GRN_BULK_VSIZE(key),
                                   offset, limit, flags);
    rb_grn_context_check(*context, self);

    return cursor;
}

/*
 * _key_ に近い順にレコードを取り出すカーソルを生成して返す。
 * ブロックを指定すると、そのブロックに生成したカーソルが渡され、
 * ブロックを抜けると自動的にカーソルが破棄される。
 *
 * @overload open_near_cursor(key, options={})
 *   @return [Groonga::PatriciaTrieCursor]
 *   @!macro [new] patricia-trie.open_near_cursor.options
 *     @param options [::Hash] The name and value
 *       pairs. Omitted names are initialized as the default value.
 *     @option options :size
 *       _size_ バイト以降のデータが同じキーのレコードに限定する。
 *     @option options :offset
 *       該当する範囲のレコードのうち、(0ベースで) _:offset_ 番目
 *       からレコードを取り出す。
 *     @option options :limit
 *       該当する範囲のレコードのうち、 _:limit_ 件のみを取り出す。
 *       省略された場合または-1が指定された場合は、全件が指定され
 *       たものとみなす。
 *     @option options :greater_than
 *       +true+ を指定すると _key_ で指定した値に一致した [ +key+ ] を
 *       範囲に含まない。
 *     @option options :less_than
 *       +true+ を指定すると _key_ で指定した値に一致した [ +key+ ] を
 *       範囲に含まない。
 *   @!macro patricia-trie.open_near_cursor.options
 * @overload open_near_cursor(key, options={})
 *   @yield [cursor]
 *   @!macro patricia-trie.open_near_cursor.options
 */
static VALUE
rb_grn_patricia_trie_open_near_cursor (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_table_cursor *cursor;
    VALUE rb_cursor;

    cursor = rb_grn_patricia_trie_open_grn_near_cursor(argc, argv,
                                                       self, &context, GRN_CURSOR_RK);
    rb_cursor = GRNTABLECURSOR2RVAL(Qnil, context, cursor);
    rb_iv_set(rb_cursor, "@table", self); /* FIXME: cursor should mark table */
    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_cursor, rb_grn_object_close, rb_cursor);
    else
        return rb_cursor;
}

void
rb_grn_init_patricia_trie (VALUE mGrn)
{
    rb_cGrnPatriciaTrie =
        rb_define_class_under(mGrn, "PatriciaTrie", rb_cGrnTable);

    rb_include_module(rb_cGrnPatriciaTrie, rb_mGrnTableKeySupport);
    rb_define_singleton_method(rb_cGrnPatriciaTrie, "create",
                               rb_grn_patricia_trie_s_create, -1);

    rb_define_method(rb_cGrnPatriciaTrie, "search",
                     rb_grn_patricia_trie_search, -1);
    rb_define_method(rb_cGrnPatriciaTrie, "scan",
                     rb_grn_patricia_trie_scan, 1);
    rb_define_method(rb_cGrnPatriciaTrie, "prefix_search",
                     rb_grn_patricia_trie_prefix_search, 1);

    rb_define_method(rb_cGrnPatriciaTrie, "register_key_with_sis?",
                     rb_grn_patricia_trie_register_key_with_sis_p, 0);

    rb_define_method(rb_cGrnPatriciaTrie, "open_prefix_cursor",
                     rb_grn_patricia_trie_open_prefix_cursor,
                     -1);
    rb_define_method(rb_cGrnPatriciaTrie, "open_rk_cursor",
                     rb_grn_patricia_trie_open_rk_cursor,
                     -1);
    rb_define_method(rb_cGrnPatriciaTrie, "open_near_cursor",
                     rb_grn_patricia_trie_open_near_cursor,
                     -1);
}
