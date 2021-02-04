/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2021  Sutou Kouhei <kou@clear-code.com>
  Copyright (C) 2016  Masafumi Yokoyama <yokoyama@clear-code.com>

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

/*
 * Document-class: Groonga::Database
 *
 * テーブルの集合を管理するためのオブジェクト。
 *
 * コンテキストに結びつけて使用する。通常、アプリケーション
 * 毎に1つのコンテキストを利用するので、データベースも1つだ
 * け利用する。コンテキストと違い、データベースは暗黙のうち
 * に作成されないので明示的に作成する必要がある。
 */

#include "rb-grn.h"

#define SELF(object) ((RbGrnObject *)RTYPEDDATA_DATA(object))

VALUE rb_cGrnDatabase;

static void
rb_grn_database_mark_existing_ruby_object (grn_ctx *context, grn_obj *database)
{
    grn_table_cursor *cursor;
    grn_id id;

    cursor = grn_table_cursor_open(context, database, NULL, 0, NULL, 0,
                                   0, -1, GRN_CURSOR_ASCENDING);
    if (!cursor)
        return;

    while ((id = grn_table_cursor_next(context, cursor)) != GRN_ID_NIL) {
        rb_grn_context_mark_grn_id(context, id);
    }
    grn_table_cursor_close(context, cursor);
}

static void
rb_grn_database_mark (void *data)
{
    RbGrnObject *rb_grn_database = data;
    grn_ctx *context;
    grn_obj *database;

    if (!rb_grn_database)
        return;

    context = rb_grn_database->context;
    database = rb_grn_database->object;
    if (!context || !database)
        return;

    rb_grn_database_mark_existing_ruby_object(context, database);
}

static void
rb_grn_database_free (void *pointer)
{
    rb_grn_object_free(pointer);
}

static rb_data_type_t data_type = {
    "Groonga::Database",
    {
        rb_grn_database_mark,
        rb_grn_database_free,
        NULL,
    },
    &rb_grn_object_data_type,
    NULL,
    RUBY_TYPED_FREE_IMMEDIATELY
};

grn_obj *
rb_grn_database_from_ruby_object (VALUE object)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnDatabase))) {
        rb_raise(rb_eTypeError, "not a Rroonga database");
    }

    return RVAL2GRNOBJECT(object, NULL);
}

VALUE
rb_grn_database_to_ruby_object (grn_ctx *context, grn_obj *database,
                                grn_bool owner)
{
    return GRNOBJECT2RVAL(rb_cGrnDatabase, context, database, owner);
}

static VALUE
rb_grn_database_alloc (VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &data_type, NULL);
}

static void
rb_grn_database_deconstruct (RbGrnObject *rb_grn_database,
                             grn_obj **database,
                             grn_ctx **context,
                             grn_id *domain_id,
                             grn_obj **domain,
                             grn_id *range_id,
                             grn_obj **range)
{
    rb_grn_object_deconstruct(rb_grn_database, database, context,
                              domain_id, domain,
                              range_id, range);
}

void
rb_grn_database_finalizer (grn_ctx *context,
                           RbGrnContext *rb_grn_context,
                           grn_obj *column,
                           RbGrnObject *rb_grn_database)
{
    if (rb_grn_context) {
        rb_grn_context_close_floating_objects(rb_grn_context);
    }

    if (!(context->flags & GRN_CTX_PER_DB)) {
        grn_ctx_use(context, NULL);
    }
}

/*
 * _database_ が使用しているリソースを開放する。これ以降 _database_ を
 * 使うことはできない。
 *
 * @overload close
 */
static VALUE
rb_grn_database_close (VALUE self)
{
    VALUE rb_context;

    rb_context = rb_iv_get(self, "@context");
    if (!NIL_P(rb_context))
        rb_iv_set(rb_context, "database", Qnil);

    return rb_grn_object_close(self);
}

static void
reset_floating_objects (VALUE rb_context)
{
    RbGrnContext *rb_grn_context = rb_grn_context_get_struct(rb_context);
    rb_grn_context_reset_floating_objects(rb_grn_context);
}

/*
 * 新しくデータベースを作成する。
 * _options_ にはハッシュでオプションを指定する。
 *
 * @example
 *   # 一時データベースを作成:
 *   Groonga::Database.create
 *
 *   # 永続データベースを作成:
 *   Groonga::Database.create(:path => "/tmp/db.groonga")
 *
 * @overload create(options=nil)
 *   @return [Groonga::Database] 作成されたデータベースを返す。
 *   @param [::Hash] options The name and value
 *     pairs. Omitted names are initialized as the default value.
 *   @option options :path
 *     データベースを保存するパス。省略すると一時データベース
 *     となる。
 *   @option options :context (Groonga::Context.default)
 *     データベースを結びつけるコンテキスト。省略すると
 *     {Groonga::Context.default} を利用する。
 */
static VALUE
rb_grn_database_s_create (int argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context;
    grn_obj *old_database, *database;
    grn_db_create_optarg create_args;
    const char *path = NULL;
    VALUE rb_database;
    VALUE rb_path, options, rb_context, builtin_type_names;
    grn_bool owner;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
                        "path", &rb_path,
                        "context", &rb_context,
                        "builtin_type_names", &builtin_type_names,
                        NULL);

    if (!NIL_P(rb_path)) {
        FilePathValue(rb_path);
        path = StringValuePtr(rb_path);
    }
    context = rb_grn_context_ensure(&rb_context);

    create_args.builtin_type_names = NULL;
    create_args.n_builtin_type_names = 0;

    old_database = grn_ctx_db(context);
    if (old_database)
        grn_obj_unlink(context, old_database);
    reset_floating_objects(rb_context);
    database = grn_db_create(context, path, &create_args);
    rb_grn_context_check(context, rb_ary_new_from_values(argc, argv));
    owner = (context->flags & GRN_CTX_PER_DB) ? GRN_FALSE : GRN_TRUE;
    rb_database = GRNOBJECT2RVAL(klass, context, database, owner);
    rb_iv_set(rb_database, "@context", rb_context);
    if (!NIL_P(rb_context))
        rb_iv_set(rb_context, "database", rb_database);
    rb_grn_context_check(context, rb_ary_new_from_values(argc, argv));

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_database,
                         rb_grn_database_close, rb_database);
    else
        return rb_database;
}

/*
 * 既存のデータベースを開く。ブロックを指定した場合はブロッ
 * クに開いたデータベースを渡し、ブロックを抜けるときに閉じ
 * る。
 *
 * @overload new(path, options=nil)
 *   @!macro [new] database.new.arguments
 *     @param options [::Hash] The name and value
 *       pairs. Omitted names are initialized as the default value.
 *     @option options :context (Groonga::Context.default)
 *       データベースを結びつけるコンテキスト。省略すると
 *       {Groonga::Context.default} を利用する。
 *   @!macro database.new.arguments
 *   @return [Groonga::Database]
 * @overload new(path, options=nil)
 *   @!macro database.new.arguments
 *   @yield [database]
 *   @yieldparam [Groonga::Database] database 開いたデータベース
 */
static VALUE
rb_grn_database_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *old_database, *database;
    const char *path;
    VALUE rb_path, options, rb_context;

    rb_scan_args(argc, argv, "11", &rb_path, &options);

    FilePathValue(rb_path);
    path = StringValuePtr(rb_path);
    rb_grn_scan_options(options,
                        "context", &rb_context,
                        NULL);

    context = rb_grn_context_ensure(&rb_context);

    old_database = grn_ctx_db(context);
    if (old_database)
        grn_obj_unlink(context, old_database);
    reset_floating_objects(rb_context);
    database = grn_db_open(context, path);
    rb_grn_object_assign(Qnil, self, rb_context, context, database);
    rb_grn_context_check(context, self);

    rb_iv_set(self, "@context", rb_context);
    if (!NIL_P(rb_context))
        rb_iv_set(rb_context, "database", self);

    return Qnil;
}

/*
 * 既存のデータベースを開く。ブロックを指定した場合はブロッ
 * クに開いたデータベースを渡し、ブロックを抜けるときに閉じ
 * る。 _options_ にはハッシュでオプションを指定する。
 *
 * @overload open(path, options=nil)
 *   @return [Groonga::Database]
 *   @param options [::Hash] The name and value
 *     pairs. Omitted names are initialized as the default value.
 *   @!macro [new] database.open.context
 *     @option options :context (Groonga::Context.default) The context
 *       データベースを結びつけるコンテキスト。
 *   @!macro database.open.context
 * @overload open(path, options=nil)
 *   @param options [::Hash] The name and value
 *     pairs. Omitted names are initialized as the default value.
 *   @!macro database.open.context
 *   @yield [database]
 *   @yieldparam [Groonga::Database] database 開いたデータベース
 */
static VALUE
rb_grn_database_s_open (int argc, VALUE *argv, VALUE klass)
{
    VALUE database;

    database = rb_obj_alloc(klass);
    rb_grn_database_initialize(argc, argv, database);
    if (rb_block_given_p())
        return rb_ensure(rb_yield, database, rb_grn_database_close, database);
    else
        return database;
}

/*
 * データベース内のオブジェクトを順番にブロックに渡す。
 *
 * @example すべてのオブジェクトの名前を表示する:
 *   database.each do |object|
 *     p object.name
 *   end
 *
 * @example すべてのオブジェクトの名前をID順で表示する:
 *   database.each(:order_by => :id) do |object|
 *     p object.name
 *   end
 *
 * @example すべてのオブジェクトの名前をキー名の降順で表示する:
 *   database.each(:order_by => :key, :order => :desc) do |object|
 *     p object.name
 *   end
 *
 * @overload each(options=nil)
 *   @macro [new] database.each.options
 *     @param options [::Hash]
 *     @yield [object]
 *     @option options :order
 *       +:asc+ または +:ascending+ を指定すると昇順にレコードを取
 *       り出す。（デフォルト）
 *       +:desc+ または +:descending+ を指定すると降順にレコードを
 *       取り出す。
 *     @option options :order_by (:key)
 *       +:id+ を指定するとID順にレコードを取り出す。
 *       +:key+ 指定するとキー順にレコードを取り出す。（デフォル
 *       ト）
 *   @macro database.each.options
 *
 * @overload each(options=nil)
 *   @macro database.each.options
 *   @option options :ignore_missing_object (false)
 *     Specify +true+ to ignore missing object. Otherwise, an exception is
 *     raised for missing object.
 *
 *   @since 2.0.5
 */
static VALUE
rb_grn_database_each (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *database;
    grn_table_cursor *cursor;
    VALUE rb_cursor, rb_options, rb_order, rb_order_by;
    VALUE rb_ignore_missing_object;
    int flags = 0;
    grn_id id;
    VALUE exception;

    RETURN_ENUMERATOR(self, argc, argv);

    rb_grn_database_deconstruct(SELF(self), &database, &context,
                                NULL, NULL, NULL, NULL);

    rb_scan_args(argc, argv, "01", &rb_options);

    rb_grn_scan_options(rb_options,
                        "order", &rb_order,
                        "order_by", &rb_order_by,
                        "ignore_missing_object", &rb_ignore_missing_object,
                        NULL);

    flags |= rb_grn_table_cursor_order_to_flag(rb_order);
    flags |= rb_grn_table_cursor_order_by_to_flag(GRN_TABLE_PAT_KEY,
                                                  self,
                                                  rb_order_by);

    cursor = grn_table_cursor_open(context, database, NULL, 0, NULL, 0,
                                   0, -1,
                                   flags);
    rb_cursor = GRNTABLECURSOR2RVAL(Qnil, context, cursor);
    rb_iv_set(self, "cursor", rb_cursor);
    while ((id = grn_table_cursor_next(context, cursor)) != GRN_ID_NIL) {
        grn_obj *object;

        object = grn_ctx_at(context, id);
        if (!object && RTEST(rb_ignore_missing_object)) {
            context->rc = GRN_SUCCESS;
            continue;
        }

        exception = rb_grn_context_to_exception(context, self);
        if (!NIL_P(exception)) {
            rb_grn_object_close(rb_cursor);
            rb_iv_set(self, "cursor", Qnil);
            rb_exc_raise(exception);
        }

        if (object) {
            rb_yield(GRNOBJECT2RVAL(Qnil, context, object, GRN_FALSE));
        }
    }
    rb_grn_object_close(rb_cursor);
    rb_iv_set(self, "cursor", Qnil);

    return Qnil;
}

/*
 * _database_ のロックを解除する。
 *
 * @overload unlock
 */
static VALUE
rb_grn_database_unlock (VALUE self)
{
    grn_ctx *context;
    grn_obj *database;
    grn_rc rc;

    rb_grn_database_deconstruct(SELF(self), &database, &context,
                                NULL, NULL, NULL, NULL);

    rc = grn_obj_unlock(context, database, GRN_ID_NIL);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * _database_ をロックする。ロックに失敗した場合は
 * {Groonga::ResourceDeadlockAvoided} 例外が発生する。
 *
 * @overload lock(options={})
 *   @!macro [new] database.lock.arguments
 *     @param [::Hash] options 利用可能なオプションは以下の通り。
 *     @option options :timeout
 *       ロックを獲得できなかった場合は _:timeout_ 秒間ロックの獲
 *       得を試みる。 _:timeout_ 秒以内にロックを獲得できなかった
 *       場合は例外が発生する。
 *   @!macro database.lock.arguments
 * @overload lock(options={})
 *   @yield ブロックを指定した場合はブロックを抜けたときにunlockする。
 *   @!macro database.lock.arguments
 */
static VALUE
rb_grn_database_lock (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *database;
    int timeout = 0;
    grn_rc rc;
    VALUE options, rb_timeout;

    rb_scan_args(argc, argv, "01",  &options);

    rb_grn_database_deconstruct(SELF(self), &database, &context,
                                NULL, NULL, NULL, NULL);

    rb_grn_scan_options(options,
                        "timeout", &rb_timeout,
                        NULL);

    if (!NIL_P(rb_timeout))
        timeout = NUM2UINT(rb_timeout);

    rc = grn_obj_lock(context, database, GRN_ID_NIL, timeout);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    if (rb_block_given_p()) {
        return rb_ensure(rb_yield, Qnil, rb_grn_database_unlock, self);
    } else {
        return Qnil;
    }
}

/*
 * _database_ のロックを強制的に解除する。
 *
 * @overload clear_lock
 */
static VALUE
rb_grn_database_clear_lock (VALUE self)
{
    grn_ctx *context;
    grn_obj *database;

    rb_grn_database_deconstruct(SELF(self), &database, &context,
                                NULL, NULL, NULL, NULL);

    grn_obj_clear_lock(context, database);

    return Qnil;
}

/*
 * _database_ がロックされていれば +true+ を返す。
 *
 * @overload locked?
 */
static VALUE
rb_grn_database_is_locked (VALUE self)
{
    grn_ctx *context;
    grn_obj *database;

    rb_grn_database_deconstruct(SELF(self), &database, &context,
                                NULL, NULL, NULL, NULL);

    return CBOOL2RVAL(grn_obj_is_locked(context, database));
}

/*
 * _database_ の最終更新時刻を現在時刻にする。
 *
 * @overload touch
 */
static VALUE
rb_grn_database_touch (VALUE self)
{
    grn_ctx *context;
    grn_obj *database;

    rb_grn_database_deconstruct(SELF(self), &database, &context,
                                NULL, NULL, NULL, NULL);

    grn_db_touch(context, database);
    return Qnil;
}

/*
 * Defrags all variable size columns in the database.
 *
 * @return [Integer] the number of defraged segments
 * @overload defrag(options={})
 *   @param [::Hash] options option for defrag
 *   @option options [Integer] :threshold (0) the threshold to
 *     determine whether a segment is defraged. Available
 *     values are -4..22. -4 means all segments are defraged.
 *     22 means no segment is defraged.
 * @since 1.2.6
 */
static VALUE
rb_grn_database_defrag (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *database;
    int n_segments;
    VALUE options, rb_threshold;
    int threshold = 0;

    rb_scan_args(argc, argv, "01", &options);
    rb_grn_scan_options(options,
                        "threshold", &rb_threshold,
                        NULL);
    if (!NIL_P(rb_threshold)) {
        threshold = NUM2INT(rb_threshold);
    }

    rb_grn_database_deconstruct(SELF(self), &database, &context,
                                NULL, NULL, NULL, NULL);
    n_segments = grn_obj_defrag(context, database, threshold);
    rb_grn_context_check(context, self);

    return INT2NUM(n_segments);
}

/*
 * Recovers database.
 *
 * @overload recover()
 *
 *   If the database is broken, try to recover the database. If the
 *   database can't be recovered, an {Groonga::Error} family exception
 *   is raised.
 *
 *   If the database isn't broken, it does nothing.
 *
 *   @return [void]
 *
 * @since 4.0.8
 */
static VALUE
rb_grn_database_recover (VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *database;

    rb_grn_database_deconstruct(SELF(self), &database, &context,
                                NULL, NULL, NULL, NULL);
    rc = grn_db_recover(context, database);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * @overload unmap()
 *
 *   Unmaps all mapped tables and columns in database.
 *
 *   It frees resources for them.
 *
 *   Normally, you don't need to unmap explicitly. Because OS manages
 *   resourced for mapped tables and columns cleverly.
 *
 *   @return [void]
 *
 * @since 5.0.5
 */
static VALUE
rb_grn_database_unmap (VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *database;

    rb_grn_database_deconstruct(SELF(self), &database, &context,
                                NULL, NULL, NULL, NULL);
    rc = grn_db_unmap(context, database);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * Recreates all index columns in the database.
 *
 * This method is useful when you have any broken index columns in
 * the database. You don't need to specify each index column. But
 * this method spends more time rather than you specify only reindex
 * needed index columns.
 *
 * You can use {Groonga::TableKeySupport#reindex} to specify reindex
 * target index columns in a table.
 *
 * You can use {Groonga::FixSizeColumn#reindex} or
 * {Groonga::VariableSizeColumn#reindex} to specify reindex target
 * index columns. They use index columns of the data column as
 * reindex target index columns.
 *
 * You can use {Groonga::IndexColumn#reindex} to specify the reindex
 * target index column.
 *
 * @example How to recreate all index columns in the database
 *   database = Groonga::Database.create(:path => "/tmp/db")
 *
 *   Groonga::Schema.define do |schema|
 *     schema.create_table("Memos") do |table|
 *       table.short_text("title")
 *       table.text("content")
 *     end
 *
 *     schema.create_table("BigramTerms",
 *                         :type => :patricia_trie,
 *                         :key_type => :short_text,
 *                         :normalizer => "NormalizerAuto",
 *                         :default_tokenizer => "TokenBigram") do |table|
 *       table.index("Memos.title")
 *       table.index("Memos.content")
 *     end
 *
 *     schema.create_table("MeCabTerms",
 *                         :type => :patricia_trie,
 *                         :key_type => :short_text,
 *                         :normalizer => "NormalizerAuto",
 *                         :default_tokenizer => "TokenMecab") do |table|
 *       table.index("Memos.title")
 *       table.index("Memos.content")
 *     end
 *   end
 *
 *   database.reindex
 *   # They are called:
 *   #   Groonga["BigramTerms.Memos_title"].reindex
 *   #   Groonga["BigramTerms.Memos_content"].reindex
 *   #   Groonga["MeCabTerms.Memos_title"].reindex
 *   #   Groonga["MeCabTerms.Memos_content"].reindex
 *
 * @overload reindex
 *   @return [void]
 *
 * @see Groonga::TableKeySupport#reindex
 * @see Groonga::FixSizeColumn#reindex
 * @see Groonga::VariableSizeColumn#reindex
 * @see Groonga::IndexColumn#reindex
 *
 * @since 5.1.1
 */
static VALUE
rb_grn_database_reindex (VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *database;

    rb_grn_database_deconstruct(SELF(self), &database, &context,
                                NULL, NULL, NULL, NULL);

    rc = grn_obj_reindex(context, database);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * Removes an object forcibly.
 *
 * Normally, you should use {Groonga::Object#remove}.
 *
 * @overload remove_force(name)
 *
 *   @param [String] name The target object name.
 *
 * @since 6.0.9
 */
static VALUE
rb_grn_database_remove_force (VALUE self, VALUE rb_name)
{
    grn_rc rc;
    grn_ctx *context;
    char *name;
    int name_size;

    rb_grn_database_deconstruct(SELF(self), NULL, &context,
                                NULL, NULL, NULL, NULL);

    name = StringValueCStr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    rc = grn_obj_remove_force(context, name, name_size);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

void
rb_grn_init_database (VALUE mGrn)
{
    rb_cGrnDatabase = rb_define_class_under(mGrn, "Database", rb_cGrnObject);
    rb_define_alloc_func(rb_cGrnDatabase, rb_grn_database_alloc);

    rb_include_module(rb_cGrnDatabase, rb_mEnumerable);
    rb_include_module(rb_cGrnDatabase, rb_mGrnEncodingSupport);
    rb_include_module(rb_cGrnDatabase, rb_mGrnFlushable);

    rb_define_singleton_method(rb_cGrnDatabase, "create",
                               rb_grn_database_s_create, -1);
    rb_define_singleton_method(rb_cGrnDatabase, "open",
                               rb_grn_database_s_open, -1);

    rb_define_method(rb_cGrnDatabase, "initialize",
                     rb_grn_database_initialize, -1);

    rb_define_method(rb_cGrnDatabase, "each",
                     rb_grn_database_each, -1);

    rb_define_method(rb_cGrnDatabase, "close",
                     rb_grn_database_close, 0);

    rb_define_method(rb_cGrnDatabase, "lock", rb_grn_database_lock, -1);
    rb_define_method(rb_cGrnDatabase, "unlock", rb_grn_database_unlock, 0);
    rb_define_method(rb_cGrnDatabase, "clear_lock",
                     rb_grn_database_clear_lock, 0);
    rb_define_method(rb_cGrnDatabase, "locked?", rb_grn_database_is_locked, 0);

    rb_define_method(rb_cGrnDatabase, "touch", rb_grn_database_touch, 0);
    rb_define_method(rb_cGrnDatabase, "defrag", rb_grn_database_defrag, -1);
    rb_define_method(rb_cGrnDatabase, "recover", rb_grn_database_recover, 0);
    rb_define_method(rb_cGrnDatabase, "unmap", rb_grn_database_unmap, 0);
    rb_define_method(rb_cGrnDatabase, "reindex", rb_grn_database_reindex, 0);
    rb_define_method(rb_cGrnDatabase, "remove_force", rb_grn_database_remove_force, 1);
}
