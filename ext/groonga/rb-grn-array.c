/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2013  Kouhei Sutou <kou@clear-code.com>

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
 * @overload create(options={})
 *   @return [Groonga::Array]
 *   @!macro [new] array.create.options
 *     @param [::Hash] options The name and value
 *       pairs. Omitted names are initialized as the default value.
 *     @option options [Groonga::Context] :context (Groonga::Context.default)
 *       テーブルが利用する {Groonga::Context} 。
 *     @option options :name The name
 *       テーブルの名前。名前をつけると、 {Groonga::Context#[]} に名
 *       前を指定してテーブルを取得することができる。省略すると
 *       無名テーブルになり、テーブルIDでのみ取得できる。
 *     @option options :path
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
 *     @option options :value_type (nil)
 *       値の型を指定する。省略すると値のための領域を確保しない。
 *       値を保存したい場合は必ず指定すること。
 *       参考: {Groonga::Type.new}
 *     @option options [Groonga::Record#n_sub_records] :sub_records
 *       +true+ を指定すると {#group} でグループ化したときに、
 *       {Groonga::Record#n_sub_records} でグループに含まれるレコー
 *       ドの件数を取得できる。
 *   @!macro array.create.options
 * @overload create(options={})
 *   @yield [table] 生成されたテーブル。ブロックを抜けると破棄される。
 *   @!macro array.create.options
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
 * レコード追加し、追加したレコードを返す。レコードの追加に失
 * 敗した場合は +nil+ を返す。
 *
 * _values_ にはレコードのカラムに設定する値を指定する。省略
 * した場合または +nil+ を指定した場合はカラムは設定しない。カ
 * ラムの値は @{:カラム名1 => 値1, :カラム名2 => 値2,
 * ...}@ と指定する。
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
 * @overload add(values=nil)
 *   @return [Groonga::Recordまたはnil]
 *
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

typedef struct _YieldRecordCallbackData
{
    VALUE self;
    VALUE record;
    grn_id id;
    int status;
} YieldRecordCallbackData;

static VALUE
yield_record (VALUE user_data)
{
    YieldRecordCallbackData *data = (YieldRecordCallbackData *)user_data;
    volatile VALUE record;

    if (data->id == GRN_ID_NIL) {
        record = Qnil;
    } else {
        record = rb_grn_record_new(data->self, data->id, Qnil);
    }
    data->record = record;

    return rb_yield(record);
}

static void
yield_record_callback (grn_ctx *ctx, grn_array *array,
                       grn_id id, void *user_data)
{
    YieldRecordCallbackData *data = user_data;

    data->id = id;
    rb_protect(yield_record, (VALUE)(data), &(data->status));
}

/*
 * Pushes a record to the array. The record should be filled in the
 * given block. The pushed record can be pulled by
 * {Groonga::Array#pull}.
 *
 * @example A program that pushes a job without error handling
 *   queue = Groonga::Array.create(:name => "CrawlURLQueue")
 *   queue.define_column("url", "ShortText")
 *   urls = ["http://groonga.org/", "http://ranguba.org/"]
 *   urls.each do |url|
 *     queue.push do |record|
 *       record.url = url
 *     end
 *   end
 *
 * @example A program that pulls a job without error handling
 *   queue = Groonga::Array.open(:name => "CrawlURLQueue")
 *   loop do
 *     url = nil
 *     queue.pull do |record|
 *       url = record.url
 *       record.delete
 *     end
 *     # Crawl URL
 *   end
 *
 * The record that is passed to the given block may be nil. You need
 * to handle the case. For example, just ignoring it or reports an
 * error.
 *
 * @example A program that pushes a job with error handling
 *   queue = Groonga::Array.create(:name => "CrawlURLQueue")
 *   queue.define_column("url", "ShortText")
 *   urls = ["http://groonga.org/", "http://ranguba.org/"]
 *   urls.each do |url|
 *     queue.push do |record|
 *       record.url = url if record # check record is not nil
 *     end
 *   end
 *
 * If an error is occurred in the given block, the pushed record may
 * not be filled completely. You should handle the case in pull side.
 *
 * @example A program that has an error in push block
 *   queue = Groonga::Array.create(:name => "CrawlURLQueue")
 *   queue.define_column("url", "ShortText")
 *   urls = ["http://groonga.org/", "http://ranguba.org/"]
 *   urls.each do |url|
 *     queue.push do |record|
 *       record.url = uri # Typo! It should be ur*l* not ur*i*
 *       # record.url isn't set
 *     end
 *   end
 *
 * @example A program that pulls a job with error handling
 *   queue = Groonga::Array.open(:name => "CrawlURLQueue")
 *   loop do
 *     url = nil
 *     queue.pull do |record|
 *       url = record.url # record.url is nil!
 *       record.delete
 *     end
 *     next if url.nil? # Ignore an uncompleted added job
 *     # Crawl URL
 *   end
 *
 * @overload push
 *   @yield [record] Filles columns of a pushed record in the given block.
 *   @yieldparam record [Groonga::Record or nil]
 *     A pushed record. It is nil when pushing is failed.
 *   @return [Groonga::Record or nil] A pushed record that is yielded.
 *
 */
static VALUE
rb_grn_array_push (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    YieldRecordCallbackData data;

    if (!rb_block_given_p()) {
        rb_raise(rb_eArgError,
                 "tried to call Groonga::Array#push without a block");
    }

    table = SELF(self, &context);

    data.self = self;
    data.record = Qnil;
    data.status = 0;
    grn_array_push(context, (grn_array *)table, yield_record_callback, &data);
    if (data.status != 0) {
        rb_jump_tag(data.status);
    }
    rb_grn_context_check(context, self);

    return data.record;
}

/*
 * Pulls a record from the array. The required values should be
 * retrieved in the given block.
 *
 * If {Groonga::Array#push} failes to fill values of the pushed
 * record, the pulled record may be uncompleted. It should be handled
 * by your application.
 *
 * If you passes @:block? => true@ option, the pull operation blocks
 * until a pushed record is pushed. It is the default behavior.
 *
 * If you passes @:block? => false@ option, the pull operation returns
 * immediately, the given block isn't called and returns nil when no
 * record exist in the array.
 *
 * @example A program that pulls with non-block mode
 *   queue = Groonga::Array.open(:name => "CrawlURLQueue")
 *   loop do
 *     url = nil
 *     # The case for no pushed records in the array.
 *     pulled_record = queue.pull(:block? => false) do |record|
 *       # This block isn't called
 *       url = record.url
 *       record.delete
 *     end
 *     p pulled_record.nil? # => true
 *   end
 *
 * Note that your signal handlers can't be ran while a pull
 * operation. You need to use {Groonga::Array#unblock} from
 * another process to unblock the pull operation. If you call
 * {Groonga::Array#unblock}, signal handler can be ran.
 *
 * @example Signal handler isn't called
 *   queue = Groonga::Array.open(:name => "CrawlURLQueue")
 *   trap(:INT) do
 *     p :not_called!
 *   end
 *   queue.pull do |record|
 *     # Send SIGINT while blocking the pull operation.
 *     # The signal handler isn't called.
 *   end
 *
 * @see Groonga::Array#push Examples exist in the push documentation.
 *
 * @overload pull(options={})
 *   @param [::Hash] options The option parameters.
 *   @option options [Boolean] :block? (true)
 *     Whether the pull operation is blocked or not when no record exist
 *     in the array.
 *   @yield [record] Gets required values for a pull record in the given block.
 *   @yieldparam record [Groonga::Record or nil]
 *     A pulled record. It is nil when no records exist in the array
 *     and @block?@ parameter is not @true@.
 *   @return [Groonga::Record or nil] A pulled record that is yielded.
 *
 */
static VALUE
rb_grn_array_pull (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;
    VALUE options;
    VALUE rb_block_p;
    YieldRecordCallbackData data;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
                        "block?", &rb_block_p,
                        NULL);

    if (!rb_block_given_p()) {
        rb_raise(rb_eArgError,
                 "tried to call Groonga::Array#pull without a block");
    }

    table = SELF(self, &context);

    if (NIL_P(rb_block_p)) {
        rb_block_p = Qtrue;
    }

    data.self = self;
    data.record = Qnil;
    data.status = 0;
    grn_array_pull(context, (grn_array *)table, RVAL2CBOOL(rb_block_p),
                   yield_record_callback, &data);
    if (data.status != 0) {
        rb_jump_tag(data.status);
    }
    rb_grn_context_check(context, self);

    return data.record;
}

/*
 * Unblocks all {Groonga::Array#pull} operations for the array.
 *
 * @example Pull, unblock and signal
 *   # pull.rb
 *   queue = Groonga::Array.open(:name => "CrawlURLQueue")
 *   trap(:INT) do
 *     p :called!
 *   end
 *   queue.pull do |record|
 *     # 1. Send SIGINT while blocking the pull operation.
 *     #    The signal handler isn't called.
 *     # 2. Run unblock.rb.
 *     #    The signal handler is called!
 *   end
 *
 *   # unblock.rb
 *   queue = Groonga::Array.open(:name => "CrawlURLQueue")
 *   queue.unblock
 *
 * @see Groonga::Array#pull
 *
 * @overload unblock
 *   @return [void]
 *
 */
static VALUE
rb_grn_array_unblock (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *table;

    table = SELF(self, &context);

    grn_array_unblock(context, (grn_array *)table);

    return Qnil;
}

void
rb_grn_init_array (VALUE mGrn)
{
    rb_cGrnArray = rb_define_class_under(mGrn, "Array", rb_cGrnTable);

    rb_define_singleton_method(rb_cGrnArray, "create",
                               rb_grn_array_s_create, -1);

    rb_define_method(rb_cGrnArray, "add", rb_grn_array_add, -1);
    rb_define_method(rb_cGrnArray, "push", rb_grn_array_push, 0);
    rb_define_method(rb_cGrnArray, "pull", rb_grn_array_pull, -1);
    rb_define_method(rb_cGrnArray, "unblock", rb_grn_array_unblock, 0);
}
