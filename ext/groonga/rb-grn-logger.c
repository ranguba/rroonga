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

/*
 * Document-class: Groonga::Logger
 *
 * groongaから出力されるログを記録するためのクラス。
 *
 */

#define GRNLOGLEVEL2RVAL(level)    (rb_grn_log_level_to_ruby_object(level))
#define RVAL2GRNLOGLEVEL(rb_level) (rb_grn_log_level_from_ruby_object(rb_level))

VALUE cGrnLogger;
VALUE mGrnLoggerFlags;
VALUE cGrnCallbackLogger;

static ID id_new;
static ID id_parse;
static ID id_log;
static ID id_reopen;
static ID id_fin;

static grn_logger rb_grn_logger;

static grn_log_level
rb_grn_log_level_from_ruby_object (VALUE rb_level)
{
    grn_log_level level = GRN_LOG_NONE;

    if (NIL_P(rb_level)) {
        level = GRN_LOG_DEFAULT_LEVEL;
    } else if (rb_grn_equal_option(rb_level, "none")) {
        level = GRN_LOG_NONE;
    } else if (rb_grn_equal_option(rb_level, "emergency")) {
        level = GRN_LOG_EMERG;
    } else if (rb_grn_equal_option(rb_level, "alert")) {
        level = GRN_LOG_ALERT;
    } else if (rb_grn_equal_option(rb_level, "critical")) {
        level = GRN_LOG_CRIT;
    } else if (rb_grn_equal_option(rb_level, "error")) {
        level = GRN_LOG_ERROR;
    } else if (rb_grn_equal_option(rb_level, "warning")) {
        level = GRN_LOG_WARNING;
    } else if (rb_grn_equal_option(rb_level, "notice")) {
        level = GRN_LOG_NOTICE;
    } else if (rb_grn_equal_option(rb_level, "info")) {
        level = GRN_LOG_INFO;
    } else if (rb_grn_equal_option(rb_level, "debug")) {
        level = GRN_LOG_DEBUG;
    } else if (rb_grn_equal_option(rb_level, "dump")) {
        level = GRN_LOG_DUMP;
    } else {
        rb_raise(rb_eArgError,
                 "log level should be one of "
                 "[nil, :none, :emergency, :alert, :critical, :error, "
                 ":warning, :notice, :info, :debug, :dump]: %s",
                 rb_grn_inspect(rb_level));
    }

    return level;
}

static VALUE
rb_grn_log_level_to_ruby_object (grn_log_level level)
{
    VALUE rb_level = Qnil;

    switch (level) {
      case GRN_LOG_NONE:
	rb_level = RB_GRN_INTERN("none");
	break;
      case GRN_LOG_EMERG:
	rb_level = RB_GRN_INTERN("emergency");
	break;
      case GRN_LOG_ALERT:
	rb_level = RB_GRN_INTERN("alert");
	break;
      case GRN_LOG_CRIT:
	rb_level = RB_GRN_INTERN("critical");
	break;
      case GRN_LOG_ERROR:
	rb_level = RB_GRN_INTERN("error");
	break;
      case GRN_LOG_WARNING:
	rb_level = RB_GRN_INTERN("warning");
	break;
      case GRN_LOG_NOTICE:
	rb_level = RB_GRN_INTERN("notice");
	break;
      case GRN_LOG_INFO:
	rb_level = RB_GRN_INTERN("info");
	break;
      case GRN_LOG_DEBUG:
	rb_level = RB_GRN_INTERN("debug");
	break;
      case GRN_LOG_DUMP:
	rb_level = RB_GRN_INTERN("dump");
	break;
      default:
	rb_level = INT2NUM(level);
	break;
    }

    return rb_level;
}

static void
rb_grn_logger_reset_with_error_check (VALUE klass, grn_ctx *context)
{
    VALUE current_logger;

    current_logger = rb_cv_get(klass, "@@current_logger");
    if (NIL_P(current_logger))
        return;
    rb_cv_set(klass, "@@current_logger", Qnil);

    if (context) {
	grn_logger_set(context, NULL);
	rb_grn_context_check(context, current_logger);
    } else {
	grn_logger_set(NULL, NULL);
    }
}

static void
rb_grn_logger_reset (VALUE klass)
{
    rb_grn_logger_reset_with_error_check(klass, NULL);
}

static void
rb_grn_logger_log (grn_ctx *ctx, grn_log_level level,
		   const char *timestamp, const char *title, const char *message,
		   const char *location, void *user_data)
{
    VALUE handler = (VALUE)user_data;

    if (NIL_P(handler))
        return;

    /* TODO: use rb_protect(). */
    rb_funcall(handler, id_log, 5,
               GRNLOGLEVEL2RVAL(level),
               rb_str_new2(timestamp),
               rb_str_new2(title),
               rb_str_new2(message),
	       rb_str_new2(location));
}

static void
rb_grn_logger_reopen (grn_ctx *ctx, void *user_data)
{
    VALUE handler = (VALUE)user_data;

    if (NIL_P(handler))
        return;

    /* TODO: use rb_protect(). */
    rb_funcall(handler, id_reopen, 0);
}

static void
rb_grn_logger_fin (grn_ctx *ctx, void *user_data)
{
    VALUE handler = (VALUE)user_data;

    if (NIL_P(handler))
        return;

    /* TODO: use rb_protect(). */
    rb_funcall(handler, id_fin, 0);
}

/*
 * groongaがログを出力する度に呼び出されるブロックを登録する。
 *
 * @overload register(options={})
 *   @yield [level, time, title, message, location]
 *     _level_ はSymbol、それ以外は全て文字列で渡される。 _level_ 以外
 *     の4つについては _options_ で +false+ を指定することでブロックに
 *     渡さないようにすることができ、その場合は空文字列が実際には渡される。
 *   @param options [::Hash] The name and value
 *     pairs. Omitted names are initialized as the default value.
 *   @option options :level (:notice)
 *     ログのレベルを +:none+ ,  +:emergency+ ,  +:alert+ ,
 *     +:critical+ , +:error+ , +:warning+ , +:notice+ , +:info+ ,
 *     +:debug+ ,  +:dump+ のいずれかで指定する。それより重要度が
 *     低いログはブロックに渡されなくなる。デフォルトでは +:notice+ 。
 *   @option options :time
 *     ログが出力された時間をブロックに渡したいなら +true+ を指
 *     定する。デフォルトでは渡す。
 *   @option options :title
 *     ログのタイトルをブロックに渡したいなら +true+ を指定す
 *     る。デフォルトでは渡す。
 *     (FIXME: groongaで実装されていない?)
 *   @option options :message
 *     ログのメッセージをブロックに渡したいなら +true+ を指定す
 *     る。デフォルトでは渡す。
 *   @option options :location
 *     ログの発生元のプロセスIDとgroongaのソースコードのファイ
 *     ル名、行番号、関数名をブロックに渡したいなら +true+ を指
 *     定する。デフォルトでは渡す。
 */
static VALUE
rb_grn_logger_s_register (int argc, VALUE *argv, VALUE klass)
{
    VALUE rb_context = Qnil;
    grn_ctx *context;
    VALUE rb_logger, rb_callback;
    VALUE rb_options, rb_max_level;
    VALUE rb_time, rb_title, rb_message, rb_location;
    VALUE rb_flags;
    grn_log_level max_level = GRN_LOG_DEFAULT_LEVEL;
    int flags = 0;

    rb_scan_args(argc, argv, "02&", &rb_logger, &rb_options, &rb_callback);

    if (rb_block_given_p()) {
        rb_logger = rb_funcall(cGrnCallbackLogger, id_new, 1, rb_callback);
    }

    rb_grn_scan_options(rb_options,
                        "max_level", &rb_max_level,
                        "time",      &rb_time,
                        "title",     &rb_title,
                        "message",   &rb_message,
                        "location",  &rb_location,
                        "flags",     &rb_flags,
                        NULL);
    if (!NIL_P(rb_max_level)) {
	max_level = RVAL2GRNLOGLEVEL(rb_max_level);
    }

    if (NIL_P(rb_time) || CBOOL2RVAL(rb_time)) {
	flags |= GRN_LOG_TIME;
    }
    if (NIL_P(rb_title) || CBOOL2RVAL(rb_title)) {
	flags |= GRN_LOG_TITLE;
    }
    if (NIL_P(rb_message) || CBOOL2RVAL(rb_message)) {
	flags |= GRN_LOG_MESSAGE;
    }
    if (NIL_P(rb_location) || CBOOL2RVAL(rb_location)) {
	flags |= GRN_LOG_LOCATION;
    }
    if (!NIL_P(rb_flags)) {
        flags = rb_funcall(mGrnLoggerFlags, id_parse, 2,
                           INT2NUM(flags), rb_flags);
    }

    rb_grn_logger.max_level = max_level;
    rb_grn_logger.flags = flags;
    rb_grn_logger.user_data = (void *)rb_logger;

    context = rb_grn_context_ensure(&rb_context);
    grn_logger_set(context, &rb_grn_logger);
    rb_grn_context_check(context, rb_logger);
    rb_cv_set(klass, "@@current_logger", rb_logger);

    return Qnil;
}

/*
 * Unregister the registered logger. The default logger is used after
 * unregistering.
 *
 * @overload unregister
 *   @return void
 */
static VALUE
rb_grn_logger_s_unregister (VALUE klass)
{
    VALUE current_logger;
    VALUE rb_context = Qnil;
    grn_ctx *context;

    current_logger = rb_cv_get(klass, "@@current_logger");
    if (NIL_P(current_logger))
        return Qnil;

    rb_cv_set(klass, "@@current_logger", Qnil);

    context = rb_grn_context_ensure(&rb_context);
    grn_logger_set(context, NULL);
    rb_grn_context_check(context, klass);

    return Qnil;
}

static VALUE
rb_grn_logger_s_reopen_with_related_object (VALUE klass, VALUE related_object)
{
    VALUE rb_context = Qnil;
    grn_ctx *context;

    context = rb_grn_context_ensure(&rb_context);
    rb_grn_logger_reset_with_error_check(klass, context);
    grn_logger_reopen(context);
    rb_grn_context_check(context, related_object);

    return Qnil;
}

/*
 * Sends reopen request to the current logger. It is useful for
 * rotating log file.
 *
 * @overload reopen
 *   @return void
 */
static VALUE
rb_grn_logger_s_reopen (VALUE klass)
{
    VALUE rb_context = Qnil;
    grn_ctx *context;

    context = rb_grn_context_ensure(&rb_context);
    grn_logger_reopen(context);
    rb_grn_context_check(context, klass);

    return Qnil;
}

/*
 * groongaのデフォルトロガーがログを出力するファイルのパスを返す。
 *
 * @overload log_path
 *   @return [String]
 */
static VALUE
rb_grn_logger_s_get_log_path (VALUE klass)
{
    const char *path;

    path = grn_default_logger_get_path();
    if (path) {
	return rb_str_new2(path);
    } else {
	return Qnil;
    }
}

/*
 * groongaのデフォルトロガーがログを出力するファイルのパスを
 * 指定する。
 *
 * {Groonga::Logger.register} で独自のロガーを設定している場合、
 * 設定している独自ロガーは無効になる。
 *
 * @overload log_path=(path)
 */
static VALUE
rb_grn_logger_s_set_log_path (VALUE klass, VALUE rb_path)
{
    grn_bool need_reopen = GRN_FALSE;
    const char *current_path;

    current_path = grn_default_logger_get_path();
    if (NIL_P(rb_path)) {
	need_reopen = current_path != NULL;
	grn_default_logger_set_path(NULL);
    } else {
	const char *new_path;
	new_path = RSTRING_PTR(rb_path);
	if (!current_path || strcmp(new_path, current_path) != 0) {
	    need_reopen = GRN_TRUE;
	}
	grn_default_logger_set_path(new_path);
    }
    rb_cv_set(klass, "@@log_path", rb_path);

    if (need_reopen) {
	rb_grn_logger_s_reopen_with_related_object(klass, rb_path);
    }

    return Qnil;
}

void
rb_grn_init_logger (VALUE mGrn)
{
    id_new    = rb_intern("new");
    id_parse  = rb_intern("parse");
    id_log    = rb_intern("log");
    id_reopen = rb_intern("reopen");
    id_fin    = rb_intern("fin");

    rb_grn_logger.log    = rb_grn_logger_log;
    rb_grn_logger.reopen = rb_grn_logger_reopen;
    rb_grn_logger.fin    = rb_grn_logger_fin;

    rb_grn_logger.user_data = (void *)Qnil;

    cGrnLogger = rb_define_class_under(mGrn, "Logger", rb_cObject);

    rb_cv_set(cGrnLogger, "@@current_logger", Qnil);
    rb_cv_set(cGrnLogger, "@@log_path", Qnil);
    rb_define_singleton_method(cGrnLogger, "register",
                               rb_grn_logger_s_register, -1);
    rb_define_singleton_method(cGrnLogger, "unregister",
                               rb_grn_logger_s_unregister, 0);
    rb_define_singleton_method(cGrnLogger, "reopen",
                               rb_grn_logger_s_reopen, 0);
    rb_define_singleton_method(cGrnLogger, "path",
                               rb_grn_logger_s_get_path, 0);
    rb_define_singleton_method(cGrnLogger, "path=",
                               rb_grn_logger_s_set_path, 1);
    rb_set_end_proc(rb_grn_logger_reset, cGrnLogger);

    mGrnLoggerFlags = rb_define_module_under(cGrnLogger, "Flags");
#define DEFINE_FLAG(NAME)                                       \
    rb_define_const(mGrnLoggerFlags,				\
                    #NAME, INT2NUM(GRN_LOG_ ## NAME))
    DEFINE_FLAG(TIME);
    DEFINE_FLAG(TITLE);
    DEFINE_FLAG(MESSAGE);
    DEFINE_FLAG(LOCATION);
#undef DEFINE_FLAG

    cGrnCallbackLogger =
        rb_define_class_under(mGrn, "CallbackLogger", cGrnLogger);
}
