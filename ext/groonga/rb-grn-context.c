/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2010-2012  Kouhei Sutou <kou@clear-code.com>

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

#define SELF(object) (RVAL2GRNCONTEXT(object))

static VALUE cGrnContext;

/*
 * Document-class: Groonga::Context
 *
 * groonga全体に渡る情報を管理するオブジェクト。通常のアプリ
 * ケーションでは1つのコンテキストを作成し、それを利用する。
 * 複数のコンテキストを利用する必要はない。
 *
 * デフォルトで使用されるコンテキストは
 * Groonga::Context#defaultでアクセスできる。コンテキストを
 * 指定できる箇所でコンテキストの指定を省略したり +nil+ を指定
 * した場合はGroonga::Context.defaultが利用される。
 *
 * また、デフォルトのコンテキストは必要になると暗黙のうちに
 * 作成される。そのため、コンテキストを意識することは少ない。
 *
 * 暗黙のうちに作成されるコンテキストにオプションを指定する
 * 場合はGroonga::Context.default_options=を使用する。
 */

grn_ctx *
rb_grn_context_from_ruby_object (VALUE object)
{
    RbGrnContext *rb_grn_context;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, cGrnContext))) {
	rb_raise(rb_eTypeError, "not a groonga context");
    }

    Data_Get_Struct(object, RbGrnContext, rb_grn_context);
    if (!rb_grn_context)
	rb_raise(rb_eGrnError, "groonga context is NULL");
    if (!rb_grn_context->context)
	rb_raise(rb_eGrnClosed,
		 "can't access already closed groonga context");
    return rb_grn_context->context;
}

void
rb_grn_context_register_floating_object (RbGrnObject *rb_grn_object)
{
    RbGrnContext *rb_grn_context;
    grn_ctx *context;
    grn_hash *floating_objects;

    rb_grn_context = rb_grn_object->rb_grn_context;
    context = rb_grn_context->context;
    floating_objects = rb_grn_context->floating_objects;
    if (!floating_objects) {
	rb_grn_context_reset_floating_objects(rb_grn_context);
	floating_objects = rb_grn_context->floating_objects;
    }
    grn_hash_add(context, floating_objects,
		 (const void *)(&rb_grn_object), sizeof(RbGrnObject *),
		 NULL, NULL);
    rb_grn_object->floating = GRN_TRUE;
}

void
rb_grn_context_unregister_floating_object (RbGrnObject *rb_grn_object)
{
    RbGrnContext *rb_grn_context;
    grn_ctx *context;
    grn_hash *floating_objects;

    if (!rb_grn_object->floating)
	return;

    rb_grn_context = rb_grn_object->rb_grn_context;
    if (!rb_grn_context)
	return;

    context = rb_grn_context->context;
    floating_objects = rb_grn_context->floating_objects;
    grn_hash_delete(context, floating_objects,
		    (const void *)&rb_grn_object, sizeof(RbGrnObject *),
		    NULL);
    rb_grn_object->floating = GRN_FALSE;
}

void
rb_grn_context_close_floating_objects (RbGrnContext *rb_grn_context)
{
    grn_ctx *context;
    grn_hash *floating_objects;
    RbGrnObject **floating_object = NULL;

    context = rb_grn_context->context;
    floating_objects = rb_grn_context->floating_objects;
    if (!floating_objects)
	return;

    rb_grn_context->floating_objects = NULL;
    GRN_HASH_EACH(context, floating_objects, id, &floating_object, NULL, NULL, {
	    (*floating_object)->floating = GRN_FALSE;
	    grn_obj_close(context, RB_GRN_OBJECT(*floating_object)->object);
	});
    grn_hash_close(context, floating_objects);
}

void
rb_grn_context_reset_floating_objects (RbGrnContext *rb_grn_context)
{
    grn_ctx *context;

    rb_grn_context_close_floating_objects(rb_grn_context);
    context = rb_grn_context->context;
    rb_grn_context->floating_objects = grn_hash_create(context, NULL,
						       sizeof(RbGrnObject *),
						       0,
						       GRN_OBJ_TABLE_HASH_KEY);
}

static void
rb_grn_context_unlink_database (grn_ctx *context)
{
    grn_obj *database;

    database = grn_ctx_db(context);
    debug("context:database: %p:%p\n", context, database);
    if (database && database->header.type == GRN_DB) {
       debug("context:database: %p:%p: unlink\n", context, database);
       grn_obj_unlink(context, database);
    }
    debug("context:database: %p:%p: done\n", context, database);
}

static void
rb_grn_context_fin (grn_ctx *context)
{
    if (context->stat == GRN_CTX_FIN)
	return;

    if (!(context->flags & GRN_CTX_PER_DB)) {
	rb_grn_context_unlink_database(context);
    }
    grn_ctx_fin(context);
}

static void
rb_grn_context_free (void *pointer)
{
    RbGrnContext *rb_grn_context = pointer;
    grn_ctx *context;

    context = rb_grn_context->context;
    debug("context-free: %p\n", context);
    rb_grn_context_close_floating_objects(rb_grn_context);
    if (context && !rb_grn_exited)
	rb_grn_context_fin(context);
    debug("context-free: %p: done\n", context);
    xfree(rb_grn_context);
}

static VALUE
rb_grn_context_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_context_free, NULL);
}

static grn_obj *
rb_grn_context_finalizer (grn_ctx *context, int n_args, grn_obj **grn_objects,
			  grn_user_data *user_data)
{
    RbGrnContext *rb_grn_context;

    if (rb_grn_exited)
	return NULL;

    rb_grn_context = user_data->ptr;

    rb_grn_context_close_floating_objects(rb_grn_context);
    if (!(context->flags & GRN_CTX_PER_DB)) {
	rb_grn_context_unlink_database(context);
    }

    GRN_CTX_USER_DATA(context)->ptr = NULL;
    grn_ctx_set_finalizer(context, NULL);

    debug("context-finalize: %p:%p:%p\n",
	  context, rb_grn_context, rb_grn_context->context);

    rb_grn_context->context = NULL;

    debug("context-finalize: %p:%p:%p: done\n",
	  context, rb_grn_context, rb_grn_context->context);

    return NULL;
}

VALUE
rb_grn_context_to_exception (grn_ctx *context, VALUE related_object)
{
    VALUE exception, exception_class;
    const char *message;
    grn_obj bulk;

    if (context->rc == GRN_SUCCESS)
	return Qnil;

    exception_class = rb_grn_rc_to_exception(context->rc);
    message = rb_grn_rc_to_message(context->rc);

    GRN_OBJ_INIT(&bulk, GRN_BULK, 0, GRN_ID_NIL);
    GRN_TEXT_PUTS(context, &bulk, message);
    if (context->errbuf[0]) {
        GRN_TEXT_PUTS(context, &bulk, ": ");
        GRN_TEXT_PUTS(context, &bulk, context->errbuf);
    }
    if (!NIL_P(related_object)) {
	GRN_TEXT_PUTS(context, &bulk, ": ");
	GRN_TEXT_PUTS(context, &bulk, rb_grn_inspect(related_object));
    }
    if (context->errline > 0) {
        GRN_TEXT_PUTS(context, &bulk, "\n");
        GRN_TEXT_PUTS(context, &bulk, context->errfile);
        GRN_TEXT_PUTS(context, &bulk, ":");
        grn_text_itoa(context, &bulk, context->errline);
        GRN_TEXT_PUTS(context, &bulk, ": ");
        GRN_TEXT_PUTS(context, &bulk, context->errfunc);
        GRN_TEXT_PUTS(context, &bulk, "()");
    }
    exception = rb_funcall(exception_class, rb_intern("new"), 1,
			   rb_str_new(GRN_BULK_HEAD(&bulk),
				      GRN_BULK_VSIZE(&bulk)));
    grn_obj_unlink(context, &bulk);

    return exception;
}

void
rb_grn_context_check (grn_ctx *context, VALUE related_object)
{
    VALUE exception;

    exception = rb_grn_context_to_exception(context, related_object);
    if (NIL_P(exception))
	return;

    rb_exc_raise(exception);
}

grn_ctx *
rb_grn_context_ensure (VALUE *context)
{
    if (NIL_P(*context))
	*context = rb_grn_context_get_default();
    return SELF(*context);
}

VALUE
rb_grn_context_rb_string_new (grn_ctx *context, const char *string, long length)
{
    if (length < 0)
	length = strlen(string);
#ifdef HAVE_RUBY_ENCODING_H
    return rb_enc_str_new(string, length,
			  rb_grn_encoding_to_ruby_encoding(context->encoding));
#else
    return rb_str_new(string, length);
#endif
}

VALUE
rb_grn_context_rb_string_encode (grn_ctx *context, VALUE rb_string)
{
#ifdef HAVE_RUBY_ENCODING_H
    int index, to_index;
    rb_encoding *encoding, *to_encoding;
    grn_encoding context_encoding;

    context_encoding = context->encoding;
    if (context->encoding == GRN_ENC_DEFAULT)
	context->encoding = grn_get_default_encoding();
    if (context->encoding == GRN_ENC_NONE)
	return rb_string;

    if (RSTRING_LEN(rb_string) < 0)
	return rb_string;

    encoding = rb_enc_get(rb_string);
    to_encoding = rb_grn_encoding_to_ruby_encoding(context_encoding);
    index = rb_enc_to_index(encoding);
    to_index = rb_enc_to_index(to_encoding);
    if (index == to_index)
	return rb_string;

    if (rb_enc_asciicompat(to_encoding) && rb_enc_str_asciionly_p(rb_string))
	return rb_string;

    rb_string = rb_str_encode(rb_string, rb_enc_from_encoding(to_encoding),
			      0, Qnil);
#endif
    return rb_string;
}

void
rb_grn_context_text_set (grn_ctx *context, grn_obj *bulk, VALUE rb_string)
{
    rb_string = rb_grn_context_rb_string_encode(context, rb_string);
    GRN_TEXT_SET(context, bulk, RSTRING_PTR(rb_string), RSTRING_LEN(rb_string));
}

/*
 * call-seq:
 *   Groonga::Context.default -> Groonga::Context
 *
 * デフォルトのコンテキストを返す。デフォルトのコンテキスト
 * が作成されていない場合は暗黙のうちに作成し、それを返す。
 *
 * 暗黙のうちにコンテキストを作成する場合は、
 * Groonga::Context.default_optionsに設定されているオプショ
 * ンを利用する。
 */
static VALUE
rb_grn_context_s_get_default (VALUE self)
{
    VALUE context;

    context = rb_cv_get(self, "@@default");
    if (NIL_P(context)) {
	context = rb_funcall(cGrnContext, rb_intern("new"), 0);
	rb_cv_set(self, "@@default", context);
    }
    return context;
}

VALUE
rb_grn_context_get_default (void)
{
    return rb_grn_context_s_get_default(cGrnContext);
}

/*
 * call-seq:
 *   Groonga::Context.default=(context)
 *
 * デフォルトのコンテキストを設定する。 +nil+ を指定すると、
 * デフォルトのコンテキストをリセットする。リセットすると、次
 * 回Groonga::Context.defaultを呼び出したときに新しくコンテ
 * キストが作成される。
 */
static VALUE
rb_grn_context_s_set_default (VALUE self, VALUE context)
{
    rb_cv_set(self, "@@default", context);
    return Qnil;
}

/*
 * call-seq:
 *   Groonga::Context.default_options -> Hash or nil
 *
 * コンテキストを作成する時に利用するデフォルトのオプション
 * を返す。
 */
static VALUE
rb_grn_context_s_get_default_options (VALUE self)
{
    return rb_cv_get(self, "@@default_options");
}

/*
 * call-seq:
 *   Groonga::Context.default_options=(options)
 *
 * コンテキストを作成する時に利用するデフォルトのオプション
 * を設定する。利用可能なオプションは
 * Groonga::Context.newを参照。
 */
static VALUE
rb_grn_context_s_set_default_options (VALUE self, VALUE options)
{
    rb_cv_set(self, "@@default_options", options);
    return Qnil;
}

/*
 * call-seq:
 *   Groonga::Context.new(options=nil)
 *
 * コンテキストを作成する。 _options_ に指定可能な値は以下の通
 * り。
 * @param [::Hash] options The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :encoding The encoding
 *
 *   エンコーディングを指定する。エンコーディングの指定方法
 *   はGroonga::Encodingを参照。
 */
static VALUE
rb_grn_context_initialize (int argc, VALUE *argv, VALUE self)
{
    RbGrnContext *rb_grn_context;
    grn_ctx *context;
    int flags = 0; /* TODO: GRN_CTX_PER_DB */
    VALUE options, default_options;
    VALUE rb_encoding;

    rb_scan_args(argc, argv, "01", &options);
    default_options = rb_grn_context_s_get_default_options(rb_obj_class(self));
    if (NIL_P(default_options))
	default_options = rb_hash_new();

    if (NIL_P(options))
	options = rb_hash_new();
    options = rb_funcall(default_options, rb_intern("merge"), 1, options);

    rb_grn_scan_options(options,
			"encoding", &rb_encoding,
			NULL);

    rb_grn_context = ALLOC(RbGrnContext);
    DATA_PTR(self) = rb_grn_context;
    rb_grn_context->self = self;
    grn_ctx_init(&(rb_grn_context->context_entity), flags);
    context = rb_grn_context->context = &(rb_grn_context->context_entity);
    rb_grn_context_check(context, self);

    GRN_CTX_USER_DATA(context)->ptr = rb_grn_context;
    rb_grn_context->floating_objects = NULL;
    rb_grn_context_reset_floating_objects(rb_grn_context);
    grn_ctx_set_finalizer(context, rb_grn_context_finalizer);

    if (!NIL_P(rb_encoding)) {
	grn_encoding encoding;

	encoding = RVAL2GRNENCODING(rb_encoding, NULL);
	GRN_CTX_SET_ENCODING(context, encoding);
    }

    debug("context new: %p\n", context);

    return Qnil;
}

/*
 * call-seq:
 *   context.close
 *
 * Closes the _context_. Closed _context_ can't be used
 * anymore.
 */
static VALUE
rb_grn_context_close (VALUE self)
{
    int rc;
    grn_ctx *context;
    RbGrnContext *rb_grn_context;

    context = SELF(self);
    if (context) {
	rc = grn_ctx_fin(context);
	Data_Get_Struct(self, RbGrnContext, rb_grn_context);
	rb_grn_context->context = NULL;
	rb_grn_rc_check(rc, self);
    }

    return Qnil;
}

/*
 * call-seq:
 *   context.closed?
 *
 * Returns whether the _context_ is closed by #close or not.
 */
static VALUE
rb_grn_context_closed_p (VALUE self)
{
    RbGrnContext *rb_grn_context;

    Data_Get_Struct(self, RbGrnContext, rb_grn_context);

    return CBOOL2RVAL(rb_grn_context->context == NULL);
}

/*
 * call-seq:
 *   context.inspect -> String
 *
 * コンテキストの中身を人に見やすい文字列で返す。
 */
static VALUE
rb_grn_context_inspect (VALUE self)
{
    VALUE inspected;
    grn_ctx *context;
    grn_obj *database;
    VALUE rb_database;

    context = SELF(self);

    inspected = rb_str_new2("#<");
    rb_str_concat(inspected, rb_inspect(rb_obj_class(self)));
    rb_str_cat2(inspected, " ");

    if (rb_grn_exited) {
      rb_str_cat2(inspected, "(finished)");
    } else if (context) {
      rb_str_cat2(inspected, "encoding: <");
      rb_str_concat(inspected, rb_inspect(GRNENCODING2RVAL(context->encoding)));
      rb_str_cat2(inspected, ">, ");

      rb_str_cat2(inspected, "database: <");
      database = grn_ctx_db(context);
      rb_database = GRNDB2RVAL(context, database, GRN_FALSE);
      rb_str_concat(inspected, rb_inspect(rb_database));
      rb_str_cat2(inspected, ">");
    } else {
      rb_str_cat2(inspected, "(closed)");
    }

    rb_str_cat2(inspected, ">");
    return inspected;
}

/*
 * call-seq:
 *   context.encoding -> Groonga::Encoding
 *
 * コンテキストが使うエンコーディングを返す。
 */
static VALUE
rb_grn_context_get_encoding (VALUE self)
{
    return GRNENCODING2RVAL(GRN_CTX_GET_ENCODING(SELF(self)));
}

/*
 * call-seq:
 *   context.encoding=(encoding)
 *
 * コンテキストが使うエンコーディングを設定する。エンコーディ
 * ングの指定のしかたはGroonga::Encodingを参照。
 */
static VALUE
rb_grn_context_set_encoding (VALUE self, VALUE rb_encoding)
{
    grn_ctx *context;
    grn_encoding encoding;

    context = SELF(self);
    encoding = RVAL2GRNENCODING(rb_encoding, NULL);
    GRN_CTX_SET_ENCODING(context, encoding);

    return rb_encoding;
}

/*
 * call-seq:
 *   context.match_escalation_threshold -> Integer
 *
 * このコンテキストを使って検索したときに検索の挙動をエスカレー
 * ションする閾値を返します。エスカレーションの詳細はgroonga
 * の検索の仕様に関するドキュメントを参照してください。
 * http://groonga.org/docs/spec/search.html#spec-search
 */
static VALUE
rb_grn_context_get_match_escalation_threshold (VALUE self)
{
    return LL2NUM(grn_ctx_get_match_escalation_threshold(SELF(self)));
}

/*
 * call-seq:
 *   context.match_escalation_threshold=(match_escalation_threshold)
 *
 * このコンテキストを使って検索したときに検索の挙動をエスカレー
 * ションする閾値を設定します。エスカレーションの詳細はgroonga
 * の検索の仕様に関するドキュメントを参照してください。
 * http://groonga.org/docs/spec/search.html#spec-search
 */
static VALUE
rb_grn_context_set_match_escalation_threshold (VALUE self, VALUE threshold)
{
    grn_ctx *context;

    context = SELF(self);
    grn_ctx_set_match_escalation_threshold(context, NUM2LL(threshold));

    return threshold;
}

/*
 * call-seq:
 *   context.support_zlib?
 *
 * groongaがZlibサポート付きでビルドされていれば +true+ 、そう
 * でなければ +false+ を返す。
 */
static VALUE
rb_grn_context_support_zlib_p (VALUE self)
{
    VALUE rb_support_p;
    grn_ctx *context;
    grn_obj support_p;

    context = SELF(self);
    GRN_BOOL_INIT(&support_p, 0);
    grn_obj_get_info(context, NULL, GRN_INFO_SUPPORT_ZLIB, &support_p);
    rb_support_p = CBOOL2RVAL(GRN_BOOL_VALUE(&support_p));
    GRN_OBJ_FIN(context, &support_p);

    return rb_support_p;
}

/*
 * call-seq:
 *   context.support_lzo?
 *
 * groongaがLZOサポート付きでビルドされていれば +true+ 、そう
 * でなければ +false+ を返す。
 */
static VALUE
rb_grn_context_support_lzo_p (VALUE self)
{
    VALUE rb_support_p;
    grn_ctx *context;
    grn_obj support_p;

    context = SELF(self);
    GRN_BOOL_INIT(&support_p, 0);
    grn_obj_get_info(context, NULL, GRN_INFO_SUPPORT_LZO, &support_p);
    rb_support_p = CBOOL2RVAL(GRN_BOOL_VALUE(&support_p));
    GRN_OBJ_FIN(context, &support_p);

    return rb_support_p;
}

/*
 * call-seq:
 *   context.database -> Groonga::Database
 *
 * コンテキストが使うデータベースを返す。
 */
static VALUE
rb_grn_context_get_database (VALUE self)
{
    grn_ctx *context;

    context = SELF(self);
    return GRNDB2RVAL(context, grn_ctx_db(context), GRN_FALSE);
}

/*
 * call-seq:
 *   context.connect(options=nil)
 *
 * groongaサーバに接続する。 _options_ に指定可能な値は以下の通
 * り。
 * @param [::Hash] options The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :host (localhost) The groonga server host name
 *
 *   groongaサーバのホスト名。またはIPアドレス。省略すると
 *   "localhost"に接続する。
 * @option options :port (10041) The port number
 *
 *   groongaサーバのポート番号。省略すると10041番ポートに接
 *   続する。
 */
static VALUE
rb_grn_context_connect (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    const char *host;
    int port;
    int flags = 0;
    grn_rc rc;
    VALUE options, rb_host, rb_port;

    rb_scan_args(argc, argv, "01", &options);
    rb_grn_scan_options(options,
			"host", &rb_host,
			"port", &rb_port,
			NULL);

    context = SELF(self);

    if (NIL_P(rb_host)) {
	host = "localhost";
    } else {
	host = StringValueCStr(rb_host);
    }

    if (NIL_P(rb_port)) {
	port = 10041;
    } else {
	port = NUM2INT(rb_port);
    }

    rc = grn_ctx_connect(context, host, port, flags);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   context.send(string) -> ID
 *
 * groongaサーバにクエリ文字列を送信する。
 */
static VALUE
rb_grn_context_send (VALUE self, VALUE rb_string)
{
    grn_ctx *context;
    char *string;
    unsigned int string_size;
    int flags = 0;
    unsigned int query_id;

    context = SELF(self);
    string = StringValuePtr(rb_string);
    string_size = RSTRING_LEN(rb_string);
    query_id = grn_ctx_send(context, string, string_size, flags);
    rb_grn_context_check(context, self);

    return UINT2NUM(query_id);
}

/*
 * call-seq:
 *   context.receive -> [ID, String]
 *
 * groongaサーバからクエリ実行結果文字列を受信する。
 */
static VALUE
rb_grn_context_receive (VALUE self)
{
    grn_ctx *context;
    char *result = NULL;
    unsigned result_size;
    VALUE rb_result;
    int flags = 0;
    unsigned int query_id;

    context = SELF(self);
    query_id = grn_ctx_recv(context, &result, &result_size, &flags);
    if (result) {
	rb_result = rb_str_new(result, result_size);
    } else {
	rb_result = Qnil;
    }
    rb_grn_context_check(context, self);

    return rb_ary_new3(2, UINT2NUM(query_id), rb_result);
}

static const char *
grn_type_name_old_to_new (const char *name, unsigned int name_size)
{
    unsigned int i;

    for (i = 0; i < name_size; i++) {
	if (name[i] == '\0')
	    return NULL;
    }

    if (strcmp(name, "<int>") == 0) {
	return "Int32";
    } else if (strcmp(name, "<uint>") == 0) {
	return "UInt32";
    } else if (strcmp(name, "<int64>") == 0) {
	return "Int64";
    } else if (strcmp(name, "<uint64>") == 0) {
	return "UInt64";
    } else if (strcmp(name, "<float>") == 0) {
	return "Float";
    } else if (strcmp(name, "<time>") == 0) {
	return "Time";
    } else if (strcmp(name, "<shorttext>") == 0) {
	return "ShortText";
    } else if (strcmp(name, "<text>") == 0) {
	return "Text";
    } else if (strcmp(name, "<longtext>") == 0) {
	return "LongText";
    } else if (strcmp(name, "<token:delimit>") == 0) {
	return "TokenDelimit";
    } else if (strcmp(name, "<token:unigram>") == 0) {
	return "TokenUnigram";
    } else if (strcmp(name, "<token:bigram>") == 0) {
	return "TokenBigram";
    } else if (strcmp(name, "<token:trigram>") == 0) {
	return "TokenTrigram";
    } else if (strcmp(name, "<token:mecab>") == 0) {
	return "TokenMecab";
    }

    return NULL;
}

grn_obj *
rb_grn_context_get_backward_compatibility (grn_ctx *context,
					   const char *name,
					   unsigned int name_size)
{
    grn_obj *object;

    object = grn_ctx_get(context, name, name_size);
    if (!object) {
	const char *new_type_name;

	new_type_name = grn_type_name_old_to_new(name, name_size);
	if (new_type_name) {
	    object = grn_ctx_get(context, new_type_name, strlen(new_type_name));
#if 0
	    if (object) {
		rb_warn("deprecated old data type name <%s> is used. "
			"Use new data type name <%s> instead.",
			name, new_type_name);
	    }
#endif
	}
    }

    return object;
}


/*
 * call-seq:
 *   context[name] -> Groonga::Object or nil
 *   context[id]   -> Groonga::Object or nil
 *
 * コンテキスト管理下にあるオブジェクトを返す。
 *
 * _name_ として文字列を指定した場合はオブジェクト名でオブジェ
 * クトを検索する。
 *
 * _id_ として数値を指定した場合はオブジェクトIDでオブジェク
 * トを検索する。
 */
static VALUE
rb_grn_context_array_reference (VALUE self, VALUE name_or_id)
{
    grn_ctx *context;
    grn_obj *object;
    const char *name;
    unsigned int name_size;
    grn_id id;

    context = SELF(self);
    switch (TYPE(name_or_id)) {
      case T_SYMBOL:
	name = rb_id2name(SYM2ID(name_or_id));
	name_size = strlen(name);
	object = rb_grn_context_get_backward_compatibility(context,
							   name, name_size);
	break;
      case T_STRING:
	name = StringValuePtr(name_or_id);
	name_size = RSTRING_LEN(name_or_id);
	object = rb_grn_context_get_backward_compatibility(context,
							   name, name_size);
	break;
      case T_FIXNUM:
	id = NUM2UINT(name_or_id);
	object = grn_ctx_at(context, id);
	break;
      default:
	rb_raise(rb_eArgError,
		 "should be String, Symbol or unsigned integer: %s",
		 rb_grn_inspect(name_or_id));
	break;
    }

    return GRNOBJECT2RVAL(Qnil, context, object, GRN_FALSE);
}

void
rb_grn_init_context (VALUE mGrn)
{
    cGrnContext = rb_define_class_under(mGrn, "Context", rb_cObject);
    rb_define_alloc_func(cGrnContext, rb_grn_context_alloc);

    rb_cv_set(cGrnContext, "@@default", Qnil);
    rb_cv_set(cGrnContext, "@@default_options", Qnil);

    rb_define_singleton_method(cGrnContext, "default",
			       rb_grn_context_s_get_default, 0);
    rb_define_singleton_method(cGrnContext, "default=",
			       rb_grn_context_s_set_default, 1);
    rb_define_singleton_method(cGrnContext, "default_options",
			       rb_grn_context_s_get_default_options, 0);
    rb_define_singleton_method(cGrnContext, "default_options=",
			       rb_grn_context_s_set_default_options, 1);

    rb_define_method(cGrnContext, "initialize", rb_grn_context_initialize, -1);

    rb_define_method(cGrnContext, "close", rb_grn_context_close, 0);
    rb_define_method(cGrnContext, "closed?", rb_grn_context_closed_p, 0);

    rb_define_method(cGrnContext, "inspect", rb_grn_context_inspect, 0);

    rb_define_method(cGrnContext, "encoding", rb_grn_context_get_encoding, 0);
    rb_define_method(cGrnContext, "encoding=", rb_grn_context_set_encoding, 1);
    rb_define_method(cGrnContext, "match_escalation_threshold",
		     rb_grn_context_get_match_escalation_threshold, 0);
    rb_define_method(cGrnContext, "match_escalation_threshold=",
		     rb_grn_context_set_match_escalation_threshold, 1);

    rb_define_method(cGrnContext, "support_zlib?",
		     rb_grn_context_support_zlib_p, 0);
    rb_define_method(cGrnContext, "support_lzo?",
		     rb_grn_context_support_lzo_p, 0);

    rb_define_method(cGrnContext, "database", rb_grn_context_get_database, 0);

    rb_define_method(cGrnContext, "[]", rb_grn_context_array_reference, 1);

    rb_define_method(cGrnContext, "connect", rb_grn_context_connect, -1);
    rb_define_method(cGrnContext, "send", rb_grn_context_send, 1);
    rb_define_method(cGrnContext, "receive", rb_grn_context_receive, 0);
}
