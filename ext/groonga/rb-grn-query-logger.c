/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2012-2013  Kouhei Sutou <kou@clear-code.com>

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
               rb_str_new2(timestamp),
               rb_str_new2(info),
               rb_str_new2(message));
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
 *       @see {QueryLogger::Flags.parse}
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
                           UINT2NUM(flags), rb_flags);
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
 */
static VALUE
rb_grn_query_logger_s_get_path (VALUE klass)
{
    const char *path;
    VALUE rb_path = Qnil;

    path = grn_default_query_logger_get_path();
    if (path) {
	rb_path = rb_str_new2(path);
    }
    return rb_path;
}

/*
 * Sets the query log path that is used the default query logger. If
 * you're using custom query logger by {#register}, the query log path
 * isn't used. Because it is for the default query logger.
 *
 * You can disable query logging by the default query logger by
 * specifing nil as path.
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
 */
static VALUE
rb_grn_query_logger_s_set_path (VALUE klass, VALUE rb_path)
{
    const char *path = NULL;

    rb_path = rb_grn_check_convert_to_string(rb_path);
    if (!NIL_P(rb_path)) {
        path = StringValuePtr(rb_path);
    }
    grn_default_query_logger_set_path(path);

    return Qnil;
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
