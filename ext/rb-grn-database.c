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

#define SELF(object) ((RbGrnObject *)DATA_PTR(object))

VALUE rb_cGrnDatabase;

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

grn_obj *
rb_grn_database_from_ruby_object (VALUE object)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnDatabase))) {
	rb_raise(rb_eTypeError, "not a groonga database");
    }

    return RVAL2GRNOBJECT(object, NULL);
}

VALUE
rb_grn_database_to_ruby_object (grn_ctx *context, grn_obj *database,
				rb_grn_boolean owner)
{
    return GRNOBJECT2RVAL(rb_cGrnDatabase, context, database, owner);
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

/*
 * Document-method: close
 *
 * call-seq:
 *   database.close
 *
 * _database_が使用しているリソースを開放する。これ以降_database_を
 * 使うことはできない。
 */
static VALUE
rb_grn_database_close (VALUE self)
{
    VALUE rb_context;

    rb_context = rb_iv_get(self, "context");
    if (!NIL_P(rb_context))
	rb_iv_set(rb_context, "database", Qnil);

    return rb_grn_object_close(self);
}

/*
 * call-seq:
 *   Groonga::Database.create(options=nil) -> Groonga::Database
 *
 * 新しくデータベースを作成する。
 *
 * _options_にはハッシュでオプションを指定する。指定できるオ
 * プションは以下の通り。
 *
 * [+:path+]
 *   データベースを保存するパス。省略すると一時データベース
 *   となる。
 *
 * [+:context+]
 *   データベースを結びつけるコンテキスト。省略すると
 *   Groonga::Context.defaultを利用する。
 *
 * 使用例は以下の通り。
 *
 * 一時データベースを作成:
 *   Groonga::Database.create
 *
 * 永続データベースを作成:
 *   Groonga::Database.create(:path => "/tmp/db.groonga")
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

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
                        "path", &rb_path,
			"context", &rb_context,
                        "builtin_type_names", &builtin_type_names,
			NULL);

    if (!NIL_P(rb_path))
        path = StringValuePtr(rb_path);
    context = rb_grn_context_ensure(&rb_context);

    create_args.builtin_type_names = NULL;
    create_args.n_builtin_type_names = 0;

    old_database = grn_ctx_db(context);
    if (old_database)
	grn_obj_close(context, old_database);
    database = grn_db_create(context, path, &create_args);
    rb_grn_context_check(context, rb_ary_new4(argc, argv));
    rb_database = rb_grn_object_alloc(klass);
    rb_grn_object_assign(Qnil, rb_database, rb_context, context, database);
    rb_iv_set(rb_database, "context", rb_context);
    if (!NIL_P(rb_context))
	rb_iv_set(rb_context, "database", rb_database);
    rb_grn_context_check(context, rb_ary_new4(argc, argv));

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_database,
			 rb_grn_database_close, rb_database);
    else
        return rb_database;
}

/*
 * call-seq:
 *   Groonga::Database.new(path, options=nil) -> Groonga::Database
 *   Groonga::Database.new(path, options=nil) {|database| ...}
 *
 * 既存のデータベースを開く。ブロックを指定した場合はブロッ
 * クに開いたデータベースを渡し、ブロックを抜けるときに閉じ
 * る。
 *
 * _options_にはハッシュでオプションを指定する。指定できるオ
 * プションは以下の通り。
 *
 * [+:context+]
 *   データベースを結びつけるコンテキスト。省略すると
 *   Groonga::Context.defaultを利用する。
 */
static VALUE
rb_grn_database_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *old_database, *database;
    const char *path;
    VALUE rb_path, options, rb_context;

    rb_scan_args(argc, argv, "11", &rb_path, &options);

    path = StringValuePtr(rb_path);
    rb_grn_scan_options(options,
			"context", &rb_context,
			NULL);

    context = rb_grn_context_ensure(&rb_context);

    old_database = grn_ctx_db(context);
    if (old_database)
	grn_obj_close(context, old_database);
    database = grn_db_open(context, path);
    rb_grn_object_assign(Qnil, self, rb_context, context, database);
    rb_grn_context_check(context, self);

    rb_iv_set(self, "context", rb_context);
    if (!NIL_P(rb_context))
	rb_iv_set(rb_context, "database", self);

    return Qnil;
}

/*
 * call-seq:
 *   Groonga::Database.open(path, options=nil) -> Groonga::Database
 *   Groonga::Database.open(path, options=nil) {|database| ...}
 *
 * 既存のデータベースを開く。ブロックを指定した場合はブロッ
 * クに開いたデータベースを渡し、ブロックを抜けるときに閉じ
 * る。
 *
 * _options_にはハッシュでオプションを指定する。指定できるオ
 * プションは以下の通り。
 *
 * [+:context+]
 *   データベースを結びつけるコンテキスト。省略すると
 *   Groonga::Context.defaultを利用する。
 */
static VALUE
rb_grn_database_s_open (int argc, VALUE *argv, VALUE klass)
{
    VALUE database;

    database = rb_grn_object_alloc(klass);
    rb_grn_database_initialize(argc, argv, database);
    if (rb_block_given_p())
        return rb_ensure(rb_yield, database, rb_grn_database_close, database);
    else
        return database;
}

/*
 * call-seq:
 *   database.each {|object| ...}
 *
 * データベース内のオブジェクトを順番にブロックに渡す。
 *
 * すべてのオブジェクトの名前を表示する:
 *   database.each do |object|
 *     p object.name
 *   end
 */
static VALUE
rb_grn_database_each (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *database;
    grn_table_cursor *cursor;
    VALUE rb_cursor;
    grn_id id;

    rb_grn_database_deconstruct(SELF(self), &database, &context,
				NULL, NULL, NULL, NULL);
    cursor = grn_table_cursor_open(context, database, NULL, 0, NULL, 0,
				   0, 0, GRN_CURSOR_ASCENDING);
    rb_cursor = GRNTABLECURSOR2RVAL(Qnil, context, cursor);
    rb_iv_set(self, "cursor", rb_cursor);
    while ((id = grn_table_cursor_next(context, cursor)) != GRN_ID_NIL) {
	grn_obj *object;

	object = grn_ctx_at(context, id);
	if (object)
	    rb_yield(GRNOBJECT2RVAL(Qnil, context, object, RB_GRN_FALSE));
    }
    rb_grn_object_close(rb_cursor);
    rb_iv_set(self, "cursor", Qnil);

    return Qnil;
}

/*
 * Document-method: unlock
 *
 * call-seq:
 *   database.unlock
 *
 * _database_のロックを解除する。
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
 * Document-method: lock
 *
 * call-seq:
 *   database.lock(options={})
 *   database.lock(options={}) {...}
 *
 * _database_をロックする。ロックに失敗した場合は
 * Groonga::ResourceDeadlockAvoided例外が発生する。
 *
 * ブロックを指定した場合はブロックを抜けたときにunlockする。
 *
 * 利用可能なオプションは以下の通り。
 *
 * [_:timeout_]
 *   ロックを獲得できなかった場合は_:timeout_秒間ロックの獲
 *   得を試みる。_:timeout_秒以内にロックを獲得できなかった
 *   場合は例外が発生する。
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
 * Document-method: clear_lock
 *
 * call-seq:
 *   database.clear_lock
 *
 * _database_のロックを強制的に解除する。
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
 * Document-method: locked?
 *
 * call-seq:
 *   database.locked?
 *
 * _database_がロックされていれば+true+を返す。
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

void
rb_grn_init_database (VALUE mGrn)
{
    rb_cGrnDatabase = rb_define_class_under(mGrn, "Database", rb_cGrnObject);
    rb_include_module(rb_cGrnDatabase, rb_mEnumerable);
    rb_include_module(rb_cGrnDatabase, rb_mGrnEncodingSupport);

    rb_define_singleton_method(rb_cGrnDatabase, "create",
			       rb_grn_database_s_create, -1);
    rb_define_singleton_method(rb_cGrnDatabase, "open",
			       rb_grn_database_s_open, -1);

    rb_define_method(rb_cGrnDatabase, "initialize",
		     rb_grn_database_initialize, -1);

    rb_define_method(rb_cGrnDatabase, "each",
		     rb_grn_database_each, 0);

    rb_define_method(rb_cGrnDatabase, "close",
		     rb_grn_database_close, 0);

    rb_define_method(rb_cGrnDatabase, "lock", rb_grn_database_lock, -1);
    rb_define_method(rb_cGrnDatabase, "unlock", rb_grn_database_unlock, 0);
    rb_define_method(rb_cGrnDatabase, "clear_lock",
		     rb_grn_database_clear_lock, 0);
    rb_define_method(rb_cGrnDatabase, "locked?", rb_grn_database_is_locked, 0);
}
