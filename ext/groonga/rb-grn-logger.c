/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2015  Kouhei Sutou <kou@clear-code.com>
  Copyright (C) 2016-2017  Masafumi Yokoyama <yokoyama@clear-code.com>
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

#include "rb-grn.h"

/*
 * Document-class: Groonga::Logger
 *
 * groongaから出力されるログを記録するためのクラス。
 *
 */

#define GRNLOGLEVEL2RVAL(level)    (rb_grn_log_level_to_ruby_object(level))
#define RVAL2GRNLOGLEVEL(rb_level) (rb_grn_log_level_from_ruby_object(rb_level))

VALUE rb_cGrnLogger;
VALUE rb_mGrnLoggerFlags;
VALUE rb_cGrnCallbackLogger;

static ID id_caller_locations;
static ID id_path;
static ID id_lineno;
static ID id_label;

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

/*
 * Logs a message.
 *
 * @overload log(message, options={})
 *   @param message [String] The log message.
 *   @param options [::Hash]
 *   @option options :context [Groonga::Context] (Groonga::Context.default)
 *     The context for the message.
 *   @option options :level [nil, :none, :emergency, :alert, :critical,
 *     :error, :warning, :notice, :info, :debug, :dump] (:notice)
 *     The level for the message.
 *
 *     `nil` equals to `:notice`.
 *   @option options :file [nil, String] (nil)
 *     The file name where the message is occurred.
 *
 *     If all of `:file`, `:line` and `:function` are nil, these
 *     values are guessed from `Kernel.#caller_locations` result.
 *   @option options :line [nil, Integer] (nil)
 *     The line number where the message is occurred.
 *   @option options :function [nil, String] (nil)
 *     The function or related name such as method name where the
 *     message is occurred.
 *   @return [void]
 *
 * @since 5.0.2
 */
static VALUE
rb_grn_logger_s_log (int argc, VALUE *argv, VALUE klass)
{
    VALUE rb_message;
    const char *message;
    VALUE rb_context = Qnil;
    grn_ctx *context;
    VALUE rb_level;
    grn_log_level level = GRN_LOG_DEFAULT_LEVEL;
    VALUE rb_file;
    const char *file = NULL;
    VALUE rb_line;
    int line = 0;
    VALUE rb_function;
    const char *function = NULL;
    VALUE rb_options;

    rb_scan_args(argc, argv, "11", &rb_message, &rb_options);

    message = StringValueCStr(rb_message);

    rb_grn_scan_options(rb_options,
                        "context",  &rb_context,
                        "level",    &rb_level,
                        "file",     &rb_file,
                        "line",     &rb_line,
                        "function", &rb_function,
                        NULL);

    context = rb_grn_context_ensure(&rb_context);

    if (!NIL_P(rb_level)) {
        level = RVAL2GRNLOGLEVEL(rb_level);
    }

    if (NIL_P(rb_file) && NIL_P(rb_line) && NIL_P(rb_function)) {
        VALUE rb_locations;
        VALUE rb_location;
        rb_locations = rb_funcall(rb_cObject,
                                  id_caller_locations,
                                  2,
                                  INT2NUM(1), INT2NUM(1));
        rb_location = RARRAY_PTR(rb_locations)[0];
        rb_file = rb_funcall(rb_location, id_path, 0);
        rb_line = rb_funcall(rb_location, id_lineno, 0);
        rb_function = rb_funcall(rb_location, id_label, 0);
    }

    if (!NIL_P(rb_file)) {
        file = StringValueCStr(rb_file);
    }
    if (!NIL_P(rb_line)) {
        line = NUM2INT(rb_line);
    }
    if (!NIL_P(rb_function)) {
        function = StringValueCStr(rb_function);
    }

    grn_logger_put(context, level, file, line, function, "%s", message);

    return Qnil;
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
               rb_str_new_cstr(timestamp),
               rb_str_new_cstr(title),
               rb_str_new_cstr(message),
               rb_str_new_cstr(location));
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
 *   @yield [event, level, time, title, message, location]
 *     _event_ と _level_ はSymbol、それ以外は全て文字列で渡される。
 *     _event_ と _level_ 以外
 *     の4つについては _options_ で +false+ を指定することでブロックに
 *     渡さないようにすることができ、その場合は空文字列が実際には渡される。
 *   @param options [::Hash] The name and value
 *     pairs. Omitted names are initialized as the default value.
 *   @option options :max_level (:notice)
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
 *   @option options [Bool] :thread_id (true)
 *     Specifies whether `location` includes thread ID or not.
 */
static VALUE
rb_grn_logger_s_register (int argc, VALUE *argv, VALUE klass)
{
    VALUE rb_context = Qnil;
    grn_ctx *context;
    VALUE rb_logger;
    VALUE rb_callback;
    VALUE rb_options;
    VALUE rb_max_level;
    VALUE rb_time;
    VALUE rb_title;
    VALUE rb_message;
    VALUE rb_location;
    VALUE rb_thread_id;
    VALUE rb_flags;
    grn_log_level max_level = GRN_LOG_DEFAULT_LEVEL;
    int flags = 0;

    rb_scan_args(argc, argv, "02&", &rb_logger, &rb_options, &rb_callback);

    if (rb_block_given_p()) {
        if (!NIL_P(rb_logger)) {
            rb_options = rb_logger;
        }
        rb_logger = rb_funcall(rb_cGrnCallbackLogger, id_new, 1, rb_callback);
    }

    rb_grn_scan_options(rb_options,
                        "max_level", &rb_max_level,
                        "time",      &rb_time,
                        "title",     &rb_title,
                        "message",   &rb_message,
                        "location",  &rb_location,
                        "thread_id", &rb_thread_id,
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
    if (NIL_P(rb_thread_id) || CBOOL2RVAL(rb_thread_id)) {
        flags |= GRN_LOG_THREAD_ID;
    }
    if (!NIL_P(rb_flags)) {
        flags = rb_funcall(rb_mGrnLoggerFlags, id_parse, 2,
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
 * @overload max_level
 *   @return [Symbol] The max level of the current logger.
 *
 * @since 5.0.5
 */
static VALUE
rb_grn_logger_s_get_max_level (VALUE klass)
{
    VALUE rb_context = Qnil;
    grn_ctx *context;
    grn_log_level max_level;

    context = rb_grn_context_ensure(&rb_context);
    max_level = grn_logger_get_max_level(context);

    return GRNLOGLEVEL2RVAL(max_level);
}

/*
 * @overload max_level=(max_level)
 *   Sets the max level of the current logger.
 *
 *   @param max_level [Symbol, String] The max level.
 *
 *   @return [void]
 *
 * @since 5.0.5
 */
static VALUE
rb_grn_logger_s_set_max_level (VALUE klass, VALUE rb_max_level)
{
    VALUE rb_context = Qnil;
    grn_ctx *context;

    context = rb_grn_context_ensure(&rb_context);
    grn_logger_set_max_level(context, RVAL2GRNLOGLEVEL(rb_max_level));

    return Qnil;
}

/*
 * Gets the current log path that is used by the default logger.
 *
 * @overload path
 *   @return [String or nil] The current log path
 *
 * @since 3.0.1
 */
static VALUE
rb_grn_logger_s_get_path (VALUE klass)
{
    const char *path;
    VALUE rb_path = Qnil;

    path = grn_default_logger_get_path();
    if (path) {
        rb_path = rb_str_new_cstr(path);
    }
    return rb_path;
}

/*
 * Sets the log path that is used by the default logger. If you're using
 * custom logger by {.register}, the log path isn't used. Because it
 * is for the default logger.
 *
 * If you specify nil as path, logging by the default logger is
 * disabled.
 *
 * @example Changes the log path for the default logger
 *   Groonga::Logger.path = "/tmp/groonga.log"
 *
 * @example Disables log by the default logger
 *   Groonga::Logger.path = nil
 *
 * @overload path=(path)
 *   @param path [String or nil] The log path for the default logger.
 *     If nil is specified, logging by the default logger is disabled.
 *   @return void
 *
 * @since 3.0.1
 */
static VALUE
rb_grn_logger_s_set_path (VALUE klass, VALUE rb_path)
{
    grn_bool need_reopen = GRN_FALSE;
    const char *old_path = NULL;
    const char *path = NULL;

    rb_path = rb_grn_check_convert_to_string(rb_path);
    if (!NIL_P(rb_path)) {
        path = StringValuePtr(rb_path);
    }

    old_path = grn_default_logger_get_path();
    if (!rb_grn_equal_string(old_path, path)) {
        need_reopen = GRN_TRUE;
    }

    grn_default_logger_set_path(path);

    if (need_reopen) {
        rb_grn_logger_s_reopen_with_related_object(klass, rb_path);
    }

    return Qnil;
}

/*
 * Gets the current log flags that is used by the default logger.
 *
 * @overload flags
 *   @return [Integer] The current log flags.
 *
 * @since 6.1.3
 */
static VALUE
rb_grn_logger_s_get_flags (VALUE klass)
{
    int flags = 0;
    VALUE rb_flags;

    flags = grn_default_logger_get_flags();
    rb_flags = INT2NUM(flags);

    return rb_flags;
}

/*
 * Sets the log flags that is used by the default logger.
 *
 * @overload flags=(flags)
 *   @param flags [Integer] The log flags for the default logger.
 *   @return void
 *
 * @since 6.1.3
 */
static VALUE
rb_grn_logger_s_set_flags (VALUE klass, VALUE rb_flags)
{
    grn_default_logger_set_flags(NUM2INT(rb_flags));

    return Qnil;
}

/*
 * Gets the current rotate threshold size that is used by the default
 * logger.
 *
 * If the size is larger than 0, log rotate feature is enabled in the
 * default logger.
 *
 * @overload threshold
 *   @return [Integer] The current rotate threshold size
 *
 * @since 5.0.2
 */
static VALUE
rb_grn_logger_s_get_rotate_threshold_size (VALUE klass)
{
    return OFFT2NUM(grn_default_logger_get_rotate_threshold_size());
}

/*
 * Sets the rotate threshold size that is used by the default
 * logger. If you're using custom logger by {.register}, the rotate
 * threshold size isn't used. Because it is for the default logger.
 *
 * If you specify `0` as size, log rotation by the default logger is
 * disabled.
 *
 * The default rotate threshold size is 0. It means that log rotation
 * is disabled by default.
 *
 * @example Changes the rotate threshold size for the default logger
 *   Groonga::Logger.rotate_threshold_size = 1 * 1024 * 1024 # 1MiB
 *
 * @example Disables log ration by the default logger
 *   Groonga::Logger.rotate_threshold_size = 0
 *
 * @overload rotate_threshold_size=(size)
 *   @param size [Integer] The log path for the default logger.
 *     If nil is specified, log rotate by the default logger is disabled.
 *   @return void
 *
 * @since 5.0.2
 */
static VALUE
rb_grn_logger_s_set_rotate_threshold_size (VALUE klass, VALUE rb_size)
{
    grn_default_logger_set_rotate_threshold_size(NUM2OFFT(rb_size));

    return Qnil;
}

void
rb_grn_init_logger (VALUE mGrn)
{
    id_caller_locations = rb_intern("caller_locations");
    id_path             = rb_intern("path");
    id_lineno           = rb_intern("lineno");
    id_label            = rb_intern("label");

    id_new    = rb_intern("new");
    id_parse  = rb_intern("parse");
    id_log    = rb_intern("log");
    id_reopen = rb_intern("reopen");
    id_fin    = rb_intern("fin");

    rb_grn_logger.log    = rb_grn_logger_log;
    rb_grn_logger.reopen = rb_grn_logger_reopen;
    rb_grn_logger.fin    = rb_grn_logger_fin;

    rb_grn_logger.user_data = (void *)Qnil;

    rb_cGrnLogger = rb_define_class_under(mGrn, "Logger", rb_cObject);

    rb_cv_set(rb_cGrnLogger, "@@current_logger", Qnil);
    rb_define_singleton_method(rb_cGrnLogger, "log",
                               rb_grn_logger_s_log, -1);
    rb_define_singleton_method(rb_cGrnLogger, "register",
                               rb_grn_logger_s_register, -1);
    rb_define_singleton_method(rb_cGrnLogger, "unregister",
                               rb_grn_logger_s_unregister, 0);
    rb_define_singleton_method(rb_cGrnLogger, "reopen",
                               rb_grn_logger_s_reopen, 0);
    rb_define_singleton_method(rb_cGrnLogger, "max_level",
                               rb_grn_logger_s_get_max_level, 0);
    rb_define_singleton_method(rb_cGrnLogger, "max_level=",
                               rb_grn_logger_s_set_max_level, 1);
    rb_define_singleton_method(rb_cGrnLogger, "path",
                               rb_grn_logger_s_get_path, 0);
    rb_define_singleton_method(rb_cGrnLogger, "path=",
                               rb_grn_logger_s_set_path, 1);
    rb_define_singleton_method(rb_cGrnLogger, "flags",
                               rb_grn_logger_s_get_flags, 0);
    rb_define_singleton_method(rb_cGrnLogger, "flags=",
                               rb_grn_logger_s_set_flags, 1);
    rb_define_singleton_method(rb_cGrnLogger, "rotate_threshold_size",
                               rb_grn_logger_s_get_rotate_threshold_size, 0);
    rb_define_singleton_method(rb_cGrnLogger, "rotate_threshold_size=",
                               rb_grn_logger_s_set_rotate_threshold_size, 1);
    rb_set_end_proc(rb_grn_logger_reset, rb_cGrnLogger);

    rb_mGrnLoggerFlags = rb_define_module_under(rb_cGrnLogger, "Flags");
#define DEFINE_FLAG(NAME)                                       \
    rb_define_const(rb_mGrnLoggerFlags,                         \
                    #NAME, INT2NUM(GRN_LOG_ ## NAME))
    DEFINE_FLAG(TIME);
    DEFINE_FLAG(TITLE);
    DEFINE_FLAG(MESSAGE);
    DEFINE_FLAG(LOCATION);
    DEFINE_FLAG(PID);
    DEFINE_FLAG(THREAD_ID);
#undef DEFINE_FLAG

    rb_cGrnCallbackLogger =
        rb_define_class_under(mGrn, "CallbackLogger", rb_cGrnLogger);
}
