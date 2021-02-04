/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2010-2021  Sutou Kouhei <kou@clear-code.com>
  Copyright (C) 2016  Masafumi Yokoyama <yokoyama@clear-code.com>
  Copyright (C) 2019  Horimoto Yasuhiro <horimoto@clear-code.com>

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
 * Document-class: Groonga::Context
 *
 * groonga全体に渡る情報を管理するオブジェクト。通常のアプリ
 * ケーションでは1つのコンテキストを作成し、それを利用する。
 * 複数のコンテキストを利用する必要はない。
 *
 * デフォルトで使用されるコンテキストは
 * {Groonga::Context.default} でアクセスできる。コンテキ
 * ストを指定できる箇所でコンテキストの指定を省略したり +nil+
 * を指定した場合は {Groonga::Context.default} が利用さ
 * れる。
 *
 * また、デフォルトのコンテキストは必要になると暗黙のうちに
 * 作成される。そのため、コンテキストを意識することは少ない。
 *
 * 暗黙のうちに作成されるコンテキストにオプションを指定する
 * 場合は {Groonga::Context.default_options=} を使用
 * する。
 */

#include "rb-grn.h"

#define SELF(object) (RVAL2GRNCONTEXT(object))

static VALUE cGrnContext;

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

    rb_grn_context_object_created(rb_grn_context->self, rb_grn_object->self);
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
        rb_grn_object_close_raw(*floating_object);
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

void
rb_grn_context_mark_grn_id (grn_ctx *context, grn_id id)
{
    grn_obj *object;
    grn_user_data *user_data;
    RbGrnObject *rb_grn_object;

    object = grn_ctx_at(context, id);
    if (!object)
        return;

    user_data = grn_obj_user_data(context, object);
    if (!user_data)
        return;

    rb_grn_object = RB_GRN_OBJECT(user_data->ptr);
    if (!rb_grn_object)
        return;

    rb_gc_mark(rb_grn_object->self);
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
rb_grn_context_fin (RbGrnContext *rb_grn_context)
{
    grn_ctx *context;

    context = rb_grn_context->context;

    debug("context-fin: %p\n", context);

    rb_grn_context_close_floating_objects(rb_grn_context);

    if (context && context->stat != GRN_CTX_FIN && !rb_grn_exited) {
        if (!(context->flags & GRN_CTX_PER_DB)) {
            rb_grn_context_unlink_database(context);
        }
        grn_ctx_fin(context);
    }

    debug("context-fin: %p: done\n", context);

    rb_grn_context->context = NULL;
}

static void
rb_grn_context_free (void *pointer)
{
    RbGrnContext *rb_grn_context = pointer;

    debug("context-free: %p\n", rb_grn_context);
    rb_grn_context_fin(rb_grn_context);
    debug("context-free: %p: done\n", rb_grn_context);
    xfree(rb_grn_context);
}

static rb_data_type_t data_type = {
    "Groonga::Context",
    {
        NULL,
        rb_grn_context_free,
        NULL,
    },
    NULL,
    NULL,
    RUBY_TYPED_FREE_IMMEDIATELY
};

RbGrnContext *
rb_grn_context_get_struct (VALUE rb_context)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_context, cGrnContext))) {
        rb_raise(rb_eTypeError,
                 "not a Groonga context: %" PRIsVALUE,
                 rb_context);
    }

    RbGrnContext *rb_grn_context;
    TypedData_Get_Struct(rb_context, RbGrnContext, &data_type, rb_grn_context);
    return rb_grn_context;
}

grn_ctx *
rb_grn_context_from_ruby_object (VALUE rb_context)
{
    RbGrnContext *rb_grn_context = rb_grn_context_get_struct(rb_context);
    if (!rb_grn_context)
        rb_raise(rb_eGrnError, "Groonga context is NULL");
    if (!rb_grn_context->context)
        rb_raise(rb_eGrnClosed,
                 "can't access already closed Groonga context");
    return rb_grn_context->context;
}

static VALUE
rb_grn_context_alloc (VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &data_type, NULL);
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
    message = grn_rc_to_string(context->rc);

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
    return rb_enc_str_new(string, length,
                          rb_grn_encoding_to_ruby_encoding(context->encoding));
}

VALUE
rb_grn_context_rb_string_encode (grn_ctx *context, VALUE rb_string)
{
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
    return rb_string;
}

void
rb_grn_context_text_set (grn_ctx *context, grn_obj *bulk, VALUE rb_string)
{
    rb_string = rb_grn_context_rb_string_encode(context, rb_string);
    GRN_TEXT_SET(context, bulk, RSTRING_PTR(rb_string), RSTRING_LEN(rb_string));
}

/*
 * デフォルトのコンテキストを返す。デフォルトのコンテキスト
 * が作成されていない場合は暗黙のうちに作成し、それを返す。
 *
 * 暗黙のうちにコンテキストを作成する場合は、
 * {Groonga::Context.default_options} に設定されているオプ
 * ションを利用する。
 *
 * @overload default
 *   @return [Groonga::Context]
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
 * デフォルトのコンテキストを設定する。 +nil+ を指定すると、
 * デフォルトのコンテキストをリセットする。リセットすると、次
 * 回 {Groonga::Context.default} を呼び出したときに新しくコ
 * ンテキストが作成される。
 *
 * @overload default=(context)
 */
static VALUE
rb_grn_context_s_set_default (VALUE self, VALUE context)
{
    rb_cv_set(self, "@@default", context);
    return Qnil;
}

/*
 * コンテキストを作成する時に利用するデフォルトのオプション
 * を返す。
 *
 * @overload default_options
 *   @return [::Hash or nil]
 */
static VALUE
rb_grn_context_s_get_default_options (VALUE self)
{
    return rb_cv_get(self, "@@default_options");
}

/*
 * コンテキストを作成する時に利用するデフォルトのオプション
 * を設定する。利用可能なオプションは
 * {Groonga::Context.new} を参照。
 *
 * @overload default_options=(options)
 */
static VALUE
rb_grn_context_s_set_default_options (VALUE self, VALUE options)
{
    rb_cv_set(self, "@@default_options", options);
    return Qnil;
}

/*
 * Creates a new context.
 *
 * @overload new(encoding: nil)
 *   @param encoding [Groonga::Encoding] The encoding to be used in
 *     the newly created context. See {Groonga::Encoding} how to specify
 *     encoding.
 *   @return [Groonga::Context] The newly created context.
 */
static VALUE
rb_grn_context_initialize (int argc, VALUE *argv, VALUE self)
{
    VALUE options;
    rb_scan_args(argc, argv, ":", &options);

    static ID keyword_ids[1];
    if (!keyword_ids[0]) {
        CONST_ID(keyword_ids[0], "encoding");
    }
    VALUE kwargs[1];
    VALUE rb_encoding = Qundef;
    if (!NIL_P(options)) {
        rb_get_kwargs(options, keyword_ids, 0, 1, kwargs);
        rb_encoding = kwargs[0];
    }
    if (rb_encoding == Qundef) {
        VALUE default_options =
            rb_grn_context_s_get_default_options(rb_obj_class(self));
        if (!NIL_P(default_options)) {
            rb_get_kwargs(default_options, keyword_ids, 0, 1, kwargs);
            rb_encoding = kwargs[0];
        }
        if (rb_encoding == Qundef) {
            rb_encoding = Qnil;
        }
    }

    RbGrnContext *rb_grn_context = ALLOC(RbGrnContext);
    RTYPEDDATA_DATA(self) = rb_grn_context;
    rb_grn_context->self = self;
    int flags = 0; /* TODO: GRN_CTX_PER_DB */
    grn_ctx_init(&(rb_grn_context->context_entity), flags);
    grn_ctx *context =
        rb_grn_context->context =
        &(rb_grn_context->context_entity);
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

    rb_iv_set(self, "@memory_pools", rb_ary_new());

    debug("context new: %p\n", context);

    return Qnil;
}

/*
 * Closes the _context_. Closed _context_ can't be used
 * anymore.
 *
 * @overload close
 */
static VALUE
rb_grn_context_close (VALUE self)
{
    RbGrnContext *rb_grn_context = RTYPEDDATA_DATA(self);
    if (rb_grn_context->context) {
        rb_grn_context_fin(rb_grn_context);
    }

    return Qnil;
}

/*
 * Returns whether the _context_ is closed by #close or not.
 *
 * @overload closed?
 */
static VALUE
rb_grn_context_closed_p (VALUE self)
{
    RbGrnContext *rb_grn_context = RTYPEDDATA_DATA(self);
    return CBOOL2RVAL(rb_grn_context->context == NULL);
}

/*
 * コンテキストの中身を人に見やすい文字列で返す。
 *
 * @overload inspect
 *   @return [String]
 */
static VALUE
rb_grn_context_inspect (VALUE self)
{
    VALUE inspected;
    grn_ctx *context;
    grn_obj *database;
    VALUE rb_database;

    context = SELF(self);

    inspected = rb_str_new_cstr("#<");
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
 * コンテキストが使うエンコーディングを返す。
 *
 * @overload encoding
 *   @return [Groonga::Encoding]
 */
static VALUE
rb_grn_context_get_encoding (VALUE self)
{
    return GRNENCODING2RVAL(GRN_CTX_GET_ENCODING(SELF(self)));
}

/*
 * コンテキストが使うエンコーディングを設定する。エンコーディ
 * ングの指定のしかたは {Groonga::Encoding} を参照。
 *
 * @overload encoding=(encoding)
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
 * Gets the encoding used by the context as Ruby's encoding object.
 *
 * @overload ruby_encoding
 *
 * @return [::Encoding] the encoding used by the context
 *
 * @since 2.0.5
 */
static VALUE
rb_grn_context_get_ruby_encoding (VALUE self)
{
    grn_encoding encoding;

    encoding = GRN_CTX_GET_ENCODING(SELF(self));
    return rb_grn_encoding_to_ruby_encoding_object(encoding);
}

/*
 * @overload force_match_escalation?
 *   @return [Bool]
 */
static VALUE
rb_grn_context_force_match_escalation_p (VALUE self)
{
    return CBOOL2RVAL(grn_ctx_get_force_match_escalation(SELF(self)));
}

/*
 * @overload force_match_escalation=(force_match_escalation)
 */
static VALUE
rb_grn_context_set_force_match_escalation (VALUE self, VALUE force_match_escalation)
{
    grn_ctx_set_force_match_escalation(SELF(self),
                                       RVAL2CBOOL(force_match_escalation));
    return force_match_escalation;
}

/*
 * このコンテキストを使って検索したときに検索の挙動をエスカレー
 * ションする閾値を返します。
 * エスカレーションの詳細は
 * "groongaの検索の仕様に関するドキュメント":http://groonga.org/docs/spec/search.html#match-escalation-threshold
 * を参照してください。
 *
 * @overload match_escalation_threshold
 *   @return [Integer]
 */
static VALUE
rb_grn_context_get_match_escalation_threshold (VALUE self)
{
    return LL2NUM(grn_ctx_get_match_escalation_threshold(SELF(self)));
}

/*
 * このコンテキストを使って検索したときに検索の挙動をエスカレー
 * ションする閾値を設定します。
 * エスカレーションの詳細は
 * "groongaの検索の仕様に関するドキュメント":http://groonga.org/docs/spec/search.html#match-escalation-threshold
 * を参照してください。
 *
 * @overload match_escalation_threshold=(match_escalation_threshold)
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
 * groongaがZlibサポート付きでビルドされていれば +true+ 、そう
 * でなければ +false+ を返す。
 *
 * @overload support_zlib?
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
 * If Groonga supports LZO compression, it returns +true+,
 * otherwise it returns +false+.
 *
 * Groonga does not support LZO compression 4.0.7 or later.
 * This method always returns +false+.
 *
 * @overload support_lzo?
 */
static VALUE
rb_grn_context_support_lzo_p (VALUE self)
{
    return Qfalse;
}

/*
 * If Groonga supports LZ4 compression, it returns +true+,
 * otherwise it returns +false+.
 *
 * @overload support_lz4?
 */
static VALUE
rb_grn_context_support_lz4_p (VALUE self)
{
    VALUE rb_support_p;
    grn_ctx *context;
    grn_obj support_p;

    context = SELF(self);
    GRN_BOOL_INIT(&support_p, 0);
    grn_obj_get_info(context, NULL, GRN_INFO_SUPPORT_LZ4, &support_p);
    rb_support_p = CBOOL2RVAL(GRN_BOOL_VALUE(&support_p));
    GRN_OBJ_FIN(context, &support_p);

    return rb_support_p;
}

/*
 * @overload support_zstd?
 *
 *   @return [Boolean] `true` if Groonga supports Zstandard compression,
 *     `false` otherwise.
 */
static VALUE
rb_grn_context_support_zstd_p (VALUE self)
{
    VALUE rb_support_p;
    grn_ctx *context;
    grn_obj support_p;

    context = SELF(self);
    GRN_BOOL_INIT(&support_p, 0);
    grn_obj_get_info(context, NULL, GRN_INFO_SUPPORT_ZSTD, &support_p);
    rb_support_p = CBOOL2RVAL(GRN_BOOL_VALUE(&support_p));
    GRN_OBJ_FIN(context, &support_p);

    return rb_support_p;
}

/*
 * @overload support_arrow?
 *
 *   @return [Boolean] `true` if Groonga supports Apache Arrow,
 *     `false` otherwise.
 */
static VALUE
rb_grn_context_support_arrow_p (VALUE self)
{
    VALUE rb_support_p;
    grn_ctx *context;
    grn_obj support_p;

    context = SELF(self);
    GRN_BOOL_INIT(&support_p, 0);
    grn_obj_get_info(context, NULL, GRN_INFO_SUPPORT_ARROW, &support_p);
    rb_support_p = CBOOL2RVAL(GRN_BOOL_VALUE(&support_p));
    GRN_OBJ_FIN(context, &support_p);

    return rb_support_p;
}

/*
 * コンテキストが使うデータベースを返す。
 *
 * @overload database
 *   @return [Groonga::Database]
 */
static VALUE
rb_grn_context_get_database (VALUE self)
{
    grn_ctx *context;

    context = SELF(self);
    return GRNDB2RVAL(context, grn_ctx_db(context), GRN_FALSE);
}

/*
 * groongaサーバに接続する。
 * @overload connect(options=nil)
 *   @param [::Hash] options 利用可能なオプションは次の通り。
 *   @option options :host (localhost)
 *     groongaサーバのホスト名。またはIPアドレス。省略すると
 *     "localhost"に接続する。
 *   @option options :port (10041)
 *     groongaサーバのポート番号。省略すると10041番ポートに接
 *     続する。
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
 * groongaサーバにクエリ文字列を送信する。
 * @return [Integer] ID
 * @overload send(string)
 *   @param [String] string クエリ文字列
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
 * groongaサーバからクエリ実行結果文字列を受信する。
 * @overload receive
 * @return [[ID, String]] クエリ実行結果
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

    return rb_ary_new_from_args(2, UINT2NUM(query_id), rb_result);
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
 * コンテキスト管理下にあるオブジェクトを返す。
 *
 * _name_ として文字列を指定した場合はオブジェクト名でオブジェ
 * クトを検索する。
 *
 * _id_ として数値を指定した場合はオブジェクトIDでオブジェク
 * トを検索する。
 *
 * @overload [](name)
 *   @return [Groonga::Object or nil]
 * @overload [](id)
 *   @return [Groonga::Object or nil]
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
        name = RSYMBOL2CSTR(name_or_id);
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
    rb_grn_context_check(context, name_or_id);

    return GRNOBJECT2RVAL(Qnil, context, object, GRN_FALSE);
}

/*
 * Checks whether object with the ID is opened or not.
 *
 * @overload opened?(id)
 *   @param id [Integer] The ID to be checked
 *   @return [Boolean] `true` if object with the `id` is opened,
 *      `false` otherwise.
 */
static VALUE
rb_grn_context_is_opened (VALUE self, VALUE rb_id)
{
    grn_ctx *context;
    grn_id id;
    grn_bool is_opened;

    context = SELF(self);
    id = NUM2UINT(rb_id);
    is_opened = grn_ctx_is_opened(context, id);

    return CBOOL2RVAL(is_opened);
}

void
rb_grn_context_object_created (VALUE rb_context, VALUE rb_object)
{
    ID id_object_created;
    CONST_ID(id_object_created, "object_created");
    rb_funcall(rb_context, id_object_created, 1, rb_object);
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
    rb_define_method(cGrnContext, "ruby_encoding",
                     rb_grn_context_get_ruby_encoding, 0);

    rb_define_method(cGrnContext, "force_match_escalation?",
                     rb_grn_context_force_match_escalation_p, 0);
    rb_define_method(cGrnContext, "force_match_escalation=",
                     rb_grn_context_set_force_match_escalation, 1);
    rb_define_method(cGrnContext, "match_escalation_threshold",
                     rb_grn_context_get_match_escalation_threshold, 0);
    rb_define_method(cGrnContext, "match_escalation_threshold=",
                     rb_grn_context_set_match_escalation_threshold, 1);

    rb_define_method(cGrnContext, "support_zlib?",
                     rb_grn_context_support_zlib_p, 0);
    rb_define_method(cGrnContext, "support_lzo?",
                     rb_grn_context_support_lzo_p, 0);
    rb_define_method(cGrnContext, "support_lz4?",
                     rb_grn_context_support_lz4_p, 0);
    rb_define_method(cGrnContext, "support_zstd?",
                     rb_grn_context_support_zstd_p, 0);
    rb_define_method(cGrnContext, "support_arrow?",
                     rb_grn_context_support_arrow_p, 0);

    rb_define_method(cGrnContext, "database", rb_grn_context_get_database, 0);

    rb_define_method(cGrnContext, "[]", rb_grn_context_array_reference, 1);

    rb_define_method(cGrnContext, "connect", rb_grn_context_connect, -1);
    rb_define_method(cGrnContext, "send", rb_grn_context_send, 1);
    rb_define_method(cGrnContext, "receive", rb_grn_context_receive, 0);

    rb_define_method(cGrnContext, "opened?", rb_grn_context_is_opened, 1);
}
