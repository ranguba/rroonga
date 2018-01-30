/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2012-2018  Kouhei Sutou <kou@clear-code.com>

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
 * Document-class: Groonga::QueryLogger
 *
 * A class for logging query log.
 *
 */

#define GRNQUERYLOGFLAGS2RVAL(flags)  (rb_grn_query_log_flags_to_ruby_object(flags))

VALUE cGrnQueryLogger;
VALUE mGrnQueryLoggerFlags;
VALUE cGrnCallbackQueryLogger;

static ID id_new;
static ID id_parse;
static ID id_log;
static ID id_reopen;
static ID id_fin;

static grn_query_logger rb_grn_query_logger;

static VALUE
rb_grn_query_log_flags_to_ruby_object (unsigned int flags)
{
    return UINT2NUM(flags);
}

/*
 * Logs a message.
 *
 * @overload log(message, options={})
 *   @param message [String] The log message.
 *   @param options [::Hash]
 *   @option options :context [Groonga::Context] (Groonga::Context.default)
 *     The context for the message.
 *   @option options :flags [nil, Integer, String] (0)
 *     The flags for the message.
 *
 *     The flags are passed to query logger. You can custom query
 *     logger behavior by the flags. For example, you can omit elapsed
 *     time by passing `Groonga::QueryLogger::COMMAND` flag or
 *     `Groonga::QueryLogger::DESTINATION`.
 *
 *     If `:flags` value is `String`, parsed by
 *     {Groonga::QueryLogger.parse}.
 *
 *     `nil` equals to `0`.
 *   @option options :mark [String] ("")
 *     The mark for the message.
 *
 *     Normally, a character is used as a mark such as `":"`, `"<"` and `">"`.
 *   @return [void]
 *
 * @since 5.0.2
 */
static VALUE
rb_grn_query_logger_s_log (int argc, VALUE *argv, VALUE klass)
{
    VALUE rb_message;
    const char *message;
    VALUE rb_context = Qnil;
    grn_ctx *context;
    VALUE rb_flags;
    unsigned int flags = GRN_QUERY_LOG_NONE;
    VALUE rb_mark;
    const char *mark = "";
    VALUE rb_options;

    rb_scan_args(argc, argv, "11", &rb_message, &rb_options);

    message = StringValueCStr(rb_message);

    rb_grn_scan_options(rb_options,
                        "context", &rb_context,
                        "flags",   &rb_flags,
                        "mark",    &rb_mark,
                        NULL);

    context = rb_grn_context_ensure(&rb_context);

    if (!NIL_P(rb_flags)) {
        flags = rb_funcall(mGrnQueryLoggerFlags, id_parse, 2,
                           rb_flags, UINT2NUM(flags));
    }
    if (!NIL_P(rb_mark)) {
        mark = StringValueCStr(rb_mark);
    }

    grn_query_logger_put(context, flags, mark, "%s", message);

    return Qnil;
}

static void
rb_grn_query_logger_log (grn_ctx *ctx, unsigned int flag,
                         const char *timestamp, const char *info,
                         const char *message, void *user_data)
{
    VALUE handler = (VALUE)user_data;

    if (NIL_P(handler))
        return;

    /* TODO: use rb_protect(). */
    rb_funcall(handler, id_log, 4,
               GRNQUERYLOGFLAGS2RVAL(flag),
               rb_str_new_cstr(timestamp),
               rb_str_new_cstr(info),
               rb_str_new_cstr(message));
}

static void
rb_grn_query_logger_reopen (grn_ctx *ctx, void *user_data)
{
    VALUE handler = (VALUE)user_data;

    if (NIL_P(handler))
        return;

    /* TODO: use rb_protect(). */
    rb_funcall(handler, id_reopen, 0);
}

static void
rb_grn_query_logger_fin (grn_ctx *ctx, void *user_data)
{
    VALUE handler = (VALUE)user_data;

    if (NIL_P(handler))
        return;

    /* TODO: use rb_protect(). */
    rb_funcall(handler, id_fin, 0);
}

/*
 * Registers a query logger or a callback that is called when a
 * query log event is emitted.
 *
 * @overload register(logger, options={})
 *   @param logger [#log, #reopen, #fin] The query logger. It is easy to
 *     inherit {QueryLogger}.
 *
 *   @!macro query-logger.register.options
 *     @param options [::Hash] The options.
 *     @option options [Symbol, String, Integer or nil] :flags (:default)
 *       Flags describe what query log should be logged.
 *
 *       If `flags` is String, it is parsed by {QueryLogger::Flags.parse}.
 *
 *   @return void
 *
 * @overload register(options={})
 *   @yield [action, flag, timestamp, info, message]
 *     ...
 *
 *   @!macro query-logger.register.options
 */
static VALUE
rb_grn_query_logger_s_register (int argc, VALUE *argv, VALUE klass)
{
    VALUE rb_context = Qnil;
    grn_ctx *context;
    VALUE rb_logger, rb_callback;
    VALUE rb_options, rb_command, rb_result_code, rb_destination;
    VALUE rb_cache, rb_size, rb_score, rb_default, rb_all, rb_flags;
    unsigned int flags = GRN_QUERY_LOG_NONE;

    rb_scan_args(argc, argv, "02&", &rb_logger, &rb_options, &rb_callback);

    if (rb_block_given_p()) {
        rb_logger = rb_funcall(cGrnCallbackQueryLogger, id_new, 1, rb_callback);
    }

    rb_grn_scan_options(rb_options,
                        "command",     &rb_command,
                        "result_code", &rb_result_code,
                        "destination", &rb_destination,
                        "cache",       &rb_cache,
                        "size",        &rb_size,
                        "score",       &rb_score,
                        "default",     &rb_default,
                        "all",         &rb_all,
                        "flags",       &rb_flags,
                        NULL);

    if (RVAL2CBOOL(rb_command)) {
        flags |= GRN_QUERY_LOG_COMMAND;
    }
    if (RVAL2CBOOL(rb_result_code)) {
        flags |= GRN_QUERY_LOG_RESULT_CODE;
    }
    if (RVAL2CBOOL(rb_destination)) {
        flags |= GRN_QUERY_LOG_DESTINATION;
    }
    if (RVAL2CBOOL(rb_cache)) {
        flags |= GRN_QUERY_LOG_CACHE;
    }
    if (RVAL2CBOOL(rb_size)) {
        flags |= GRN_QUERY_LOG_SIZE;
    }
    if (RVAL2CBOOL(rb_score)) {
        flags |= GRN_QUERY_LOG_SCORE;
    }
    if (RVAL2CBOOL(rb_default)) {
        flags |= GRN_QUERY_LOG_DEFAULT;
    }
    if (RVAL2CBOOL(rb_all)) {
        flags |= GRN_QUERY_LOG_ALL;
    }
    if (!NIL_P(rb_flags)) {
        flags = rb_funcall(mGrnQueryLoggerFlags, id_parse, 2,
                           rb_flags, UINT2NUM(flags));
    }

    rb_grn_query_logger.flags     = flags;
    rb_grn_query_logger.user_data = (void *)rb_logger;

    context = rb_grn_context_ensure(&rb_context);
    grn_query_logger_set(context, &rb_grn_query_logger);
    rb_grn_context_check(context, rb_logger);
    rb_cv_set(klass, "@@current_logger", rb_logger);

    return Qnil;
}

static VALUE
rb_grn_query_logger_s_unregister (VALUE klass)
{
    VALUE current_logger;
    VALUE rb_context = Qnil;
    grn_ctx *context;

    current_logger = rb_cv_get(klass, "@@current_logger");
    if (NIL_P(current_logger))
        return Qnil;

    rb_cv_set(klass, "@@current_logger", Qnil);

    context = rb_grn_context_ensure(&rb_context);
    grn_query_logger_set(context, NULL);
    rb_grn_context_check(context, klass);

    return Qnil;
}

/*
 * Sends reopen request to the current query logger. It is useful for
 * rotating log file.
 *
 * @overload reopen
 * @return void
 */
static VALUE
rb_grn_query_logger_s_reopen (VALUE klass)
{
    VALUE rb_context = Qnil;
    grn_ctx *context;

    context = rb_grn_context_ensure(&rb_context);
    grn_query_logger_reopen(context);
    rb_grn_context_check(context, klass);

    return Qnil;
}

/*
 * Gets the current query log path that is used the default query logger.
 *
 * @overload path
 *   @return [String or nil] The current query log path
 *
 * @since 3.0.1
 */
static VALUE
rb_grn_query_logger_s_get_path (VALUE klass)
{
    const char *path;
    VALUE rb_path = Qnil;

    path = grn_default_query_logger_get_path();
    if (path) {
        rb_path = rb_str_new_cstr(path);
    }
    return rb_path;
}

/*
 * Sets the query log path that is used the default query logger. If
 * you're using custom query logger by {.register}, the query log path
 * isn't used. Because it is for the default query logger.
 *
 * If you specify nil as path, query logging by the default query
 * logger is disabled.
 *
 * @example Changes the query log path for the default query logger
 *   Groonga::QueryLogger.path = "/tmp/query.log"
 *
 * @example Disables query log by the default query logger
 *   Groonga::QueryLogger.path = nil
 *
 * @overload path=(path)
 *   @param path [String or nil] The query log path for the default query
 *     logger. If nil is specified, query logging by the default query logger
 *     is disabled.
 *   @return void
 *
 * @since 3.0.1
 */
static VALUE
rb_grn_query_logger_s_set_path (VALUE klass, VALUE rb_path)
{
    grn_bool need_reopen = GRN_FALSE;
    const char *old_path = NULL;
    const char *path = NULL;

    rb_path = rb_grn_check_convert_to_string(rb_path);
    if (!NIL_P(rb_path)) {
        path = StringValuePtr(rb_path);
    }

    old_path = grn_default_query_logger_get_path();
    if (!rb_grn_equal_string(old_path, path)) {
        need_reopen = GRN_TRUE;
    }

    grn_default_query_logger_set_path(path);

    if (need_reopen) {
        rb_grn_query_logger_s_reopen(klass);
    }

    return Qnil;
}

/*
 * Gets the current rotate threshold size that is used by the default
 * query logger.
 *
 * If the size is larger than 0, log rotate feature is enabled in the
 * default query logger.
 *
 * @overload threshold
 *   @return [Integer] The current rotate threshold size
 *
 * @since 5.0.2
 */
static VALUE
rb_grn_query_logger_s_get_rotate_threshold_size (VALUE klass)
{
    return OFFT2NUM(grn_default_query_logger_get_rotate_threshold_size());
}

/*
 * Sets the rotate threshold size that is used by the default query
 * logger. If you're using custom query logger by {.register}, the
 * rotate threshold size isn't used. Because it is for the default
 * query logger.
 *
 * If you specify `0` as size, log rotation by the default query
 * logger is disabled.
 *
 * The default rotate threshold size is 0. It means that log rotation
 * is disabled by default.
 *
 * @example Changes the rotate threshold size for the default query logger
 *   Groonga::QueryLogger.rotate_threshold_size = 1 * 1024 * 1024 # 1MiB
 *
 * @example Disables log ration by the default query logger
 *   Groonga::QueryLogger.rotate_threshold_size = 0
 *
 * @overload rotate_threshold_size=(size)
 *   @param size [Integer] The rotate threshold size for the default
 *     query logger. If `nil` is specified, log rotate by the default
 *     query logger is disabled.
 *
 *   @return `size` itself.
 *
 * @since 5.0.2
 */
static VALUE
rb_grn_query_logger_s_set_rotate_threshold_size (VALUE klass, VALUE rb_size)
{
    grn_default_query_logger_set_rotate_threshold_size(NUM2OFFT(rb_size));

    return rb_size;
}

/*
 * Gets the current flags that are used by the default query logger.
 *
 * @overload flags
 *   @return [Integer] The current flags
 *
 * @since 7.1.1
 */
static VALUE
rb_grn_query_logger_s_get_flags (VALUE klass)
{
    return UINT2NUM(grn_default_query_logger_get_flags());
}

/*
 * Sets the flags that are used by the default query logger. If you're
 * using custom query logger by {.register}, the flags aren't
 * used. Because it is for the default query logger.
 *
 * @example Changes the rotate threshold size for the default query logger
 *   Groonga::QueryLogger.rotate_threshold_size = 1 * 1024 * 1024 # 1MiB
 *
 * @example Disables log ration by the default query logger
 *   Groonga::QueryLogger.rotate_threshold_size = 0
 *
 * @overload flags=(flags)
 *   @param flags [Integer] The flags for the default query logger.
 *   @return void
 *
 * @since 7.1.1
 */
static VALUE
rb_grn_query_logger_s_set_flags (VALUE klass, VALUE rb_flags)
{
    VALUE rb_parsed_flags;

    rb_parsed_flags = rb_funcall(mGrnQueryLoggerFlags, id_parse, 1, rb_flags);
    grn_default_query_logger_set_flags(NUM2UINT(rb_parsed_flags));

    return rb_flags;
}

void
rb_grn_init_query_logger (VALUE mGrn)
{
    id_new    = rb_intern("new");
    id_parse  = rb_intern("parse");
    id_log    = rb_intern("log");
    id_reopen = rb_intern("reopen");
    id_fin    = rb_intern("fin");

    rb_grn_query_logger.log    = rb_grn_query_logger_log;
    rb_grn_query_logger.reopen = rb_grn_query_logger_reopen;
    rb_grn_query_logger.fin    = rb_grn_query_logger_fin;

    rb_grn_query_logger.user_data = (void *)Qnil;

    cGrnQueryLogger = rb_define_class_under(mGrn, "QueryLogger", rb_cObject);

    rb_cv_set(cGrnQueryLogger, "@@current_logger", Qnil);
    rb_define_singleton_method(cGrnQueryLogger, "log",
                               rb_grn_query_logger_s_log, -1);
    rb_define_singleton_method(cGrnQueryLogger, "register",
                               rb_grn_query_logger_s_register, -1);
    rb_define_singleton_method(cGrnQueryLogger, "unregister",
                               rb_grn_query_logger_s_unregister, 0);
    rb_define_singleton_method(cGrnQueryLogger, "reopen",
                               rb_grn_query_logger_s_reopen, 0);
    rb_define_singleton_method(cGrnQueryLogger, "path",
                               rb_grn_query_logger_s_get_path, 0);
    rb_define_singleton_method(cGrnQueryLogger, "path=",
                               rb_grn_query_logger_s_set_path, 1);
    rb_define_singleton_method(cGrnQueryLogger, "rotate_threshold_size",
                               rb_grn_query_logger_s_get_rotate_threshold_size,
                               0);
    rb_define_singleton_method(cGrnQueryLogger, "rotate_threshold_size=",
                               rb_grn_query_logger_s_set_rotate_threshold_size,
                               1);
    rb_define_singleton_method(cGrnQueryLogger, "flags",
                               rb_grn_query_logger_s_get_flags,
                               0);
    rb_define_singleton_method(cGrnQueryLogger, "flags=",
                               rb_grn_query_logger_s_set_flags,
                               1);

    mGrnQueryLoggerFlags = rb_define_module_under(cGrnQueryLogger, "Flags");
#define DEFINE_FLAG(NAME)                                       \
    rb_define_const(mGrnQueryLoggerFlags,                       \
                    #NAME, UINT2NUM(GRN_QUERY_LOG_ ## NAME))
    DEFINE_FLAG(NONE);
    DEFINE_FLAG(COMMAND);
    DEFINE_FLAG(RESULT_CODE);
    DEFINE_FLAG(DESTINATION);
    DEFINE_FLAG(CACHE);
    DEFINE_FLAG(SIZE);
    DEFINE_FLAG(SCORE);
    DEFINE_FLAG(ALL);
    DEFINE_FLAG(DEFAULT);
#undef DEFINE_FLAG

    cGrnCallbackQueryLogger =
        rb_define_class_under(mGrn, "CallbackQueryLogger", cGrnQueryLogger);
}
