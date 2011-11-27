/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>

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

/* FIXME */
void grn_log_reopen(grn_ctx *ctx);
extern const char *grn_log_path;
extern const char *grn_qlog_path;


/*
 * Document-class: Groonga::Logger
 *
 * groongaから出力されるログを記録するためのクラス。
 *
 */

#define RVAL2GRNWRAPPER(object)  (rb_grn_logger_info_wrapper_from_ruby_object(object))
#define RVAL2GRNLOGLEVEL(object) (rb_grn_log_level_from_ruby_object(object))
#define GRNLOGLEVEL2RVAL(level)  (rb_grn_log_level_to_ruby_object(level))

VALUE cGrnLogger;

typedef struct _rb_grn_logger_info_wrapper
{
    grn_logger_info *logger;
    VALUE handler;
} rb_grn_logger_info_wrapper;

static rb_grn_logger_info_wrapper *
rb_grn_logger_info_wrapper_from_ruby_object (VALUE object)
{
    rb_grn_logger_info_wrapper *wrapper;

    if (NIL_P(object))
        return NULL;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, cGrnLogger))) {
	rb_raise(rb_eTypeError, "not a groonga logger");
    }

    Data_Get_Struct(object, rb_grn_logger_info_wrapper, wrapper);
    if (!wrapper)
        rb_raise(rb_eTypeError, "groonga logger is NULL");

    return wrapper;
}

grn_logger_info *
rb_grn_logger_from_ruby_object (VALUE object)
{
    rb_grn_logger_info_wrapper *wrapper;

    wrapper = RVAL2GRNWRAPPER(object);
    if (!wrapper)
        return NULL;

    return wrapper->logger;
}

static void
rb_grn_logger_free (void *object)
{
    rb_grn_logger_info_wrapper *wrapper = object;

    xfree(wrapper->logger);
    xfree(wrapper);
}

static VALUE
rb_grn_logger_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_logger_free, NULL);
}

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
rb_grn_log (int level, const char *time, const char *title,
            const char *message, const char *location, void *func_arg)
{
    rb_grn_logger_info_wrapper *wrapper = func_arg;

    rb_funcall(wrapper->handler, rb_intern("call"), 5,
               GRNLOGLEVEL2RVAL(level),
               rb_str_new2(time),
               rb_str_new2(title),
               rb_str_new2(message),
               rb_str_new2(location));
}

static void
rb_grn_logger_set_handler (VALUE self, VALUE rb_handler)
{
    rb_grn_logger_info_wrapper *wrapper;
    grn_logger_info *logger;

    wrapper = RVAL2GRNWRAPPER(self);
    wrapper->handler = rb_handler;
    rb_iv_set(self, "@handler", rb_handler);

    logger = wrapper->logger;
    if (NIL_P(rb_handler)) {
        logger->func = NULL;
        logger->func_arg = NULL;
    } else {
        logger->func = rb_grn_log;
        logger->func_arg = wrapper;
    }
}

static VALUE
rb_grn_logger_initialize (int argc, VALUE *argv, VALUE self)
{
    rb_grn_logger_info_wrapper *wrapper;
    grn_logger_info *logger;
    grn_log_level level;
    int flags = 0;
    VALUE options, rb_level, rb_time, rb_title, rb_message, rb_location;
    VALUE rb_handler;

    rb_scan_args(argc, argv, "01&", &options, &rb_handler);

    rb_grn_scan_options(options,
                        "level", &rb_level,
                        "time", &rb_time,
                        "title", &rb_title,
                        "message", &rb_message,
                        "location", &rb_location,
                        NULL);

    level = RVAL2GRNLOGLEVEL(rb_level);

    if (NIL_P(rb_time) || RVAL2CBOOL(rb_time))
        flags |= GRN_LOG_TIME;
    if (NIL_P(rb_title) || RVAL2CBOOL(rb_title))
        flags |= GRN_LOG_TITLE;
    if (NIL_P(rb_message) || RVAL2CBOOL(rb_message))
        flags |= GRN_LOG_MESSAGE;
    if (NIL_P(rb_location) || RVAL2CBOOL(rb_location))
        flags |= GRN_LOG_LOCATION;

    wrapper = ALLOC(rb_grn_logger_info_wrapper);
    logger = ALLOC(grn_logger_info);
    wrapper->logger = logger;
    DATA_PTR(self) = wrapper;

    logger->max_level = level;
    logger->flags = flags;
    rb_grn_logger_set_handler(self, rb_handler);

    return Qnil;
}

/*
 * call-seq:
 *   Groonga::Logger.register(options={})
       {|level, time, title, message, location| ...}
 *
 * groongaがログを出力する度に呼び出されるブロックを登録す
 * る。
 *
 * ブロックに渡されてくる引数は _level_ ,  _time_ ,  _title_ ,
 * _message_ ,  _location_ の5つで、 _level_ はSymbol、それ以外は
 * 全て文字列で渡される。その4つについては _options_ で +false+
 * を指定することでブロックに渡さないようにすることができ、
 * その場合は空文字列が実際には渡される。
 *
 * _options_ に指定可能な値は以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :level (:notice) The log level
 *
 *   ログのレベルを +:none+ ,  +:emergency+ ,  +:alert+ ,
 *   +:critical+ , +:error+ , +:warning+ , +:notice+ , +:info+ ,
 *   +:debug+ ,  +:dump+ のいずれかで指定する。それより重要度が
 *   低いログはブロックに渡されなくなる。デフォルトでは
 *   +:notice+ 。
 *
 * @option options :time
 *
 *   ログが出力された時間をブロックに渡したいなら +true+ を指
 *   定する。デフォルトでは渡す。
 *
 * @option options :title The title
 *
 *   ログのタイトルをブロックに渡したいなら +true+ を指定す
 *   る。デフォルトでは渡す。(FIXME: groongaで実装されてい
 *   ない?)
 *
 * @option options :message The message
 *   ログのメッセージをブロックに渡したいなら +true+ を指定す
 *   る。デフォルトでは渡す。
 *
 * @option options :location The location
 *
 *   ログの発生元のプロセスIDとgroongaのソースコードのファイ
 *   ル名、行番号、関数名をブロックに渡したいなら +true+ を指
 *   定する。デフォルトでは渡す。
 */
static VALUE
rb_grn_logger_s_register (int argc, VALUE *argv, VALUE klass)
{
    VALUE logger, rb_context = Qnil;
    grn_ctx *context;

    logger = rb_funcall2(klass, rb_intern("new"), argc, argv);
    rb_grn_logger_set_handler(logger, rb_block_proc());
    context = rb_grn_context_ensure(&rb_context);
    grn_logger_info_set(context, RVAL2GRNLOGGER(logger));
    rb_grn_context_check(context, logger);
    rb_cv_set(klass, "@@current_logger", logger);

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
	grn_logger_info_set(context, NULL);
	rb_grn_context_check(context, current_logger);
    } else {
	grn_logger_info_set(NULL, NULL);
    }
}

static void
rb_grn_logger_reset (VALUE klass)
{
    rb_grn_logger_reset_with_error_check(klass, NULL);
}

static VALUE
rb_grn_logger_s_reopen_with_related_object (VALUE klass, VALUE related_object)
{
#ifdef WIN32
    rb_raise(rb_eNotImpError, "grn_log_reopen() isn't available on Windows.");
#else
    VALUE rb_context = Qnil;
    grn_ctx *context;

    context = rb_grn_context_ensure(&rb_context);
    rb_grn_logger_reset_with_error_check(klass, context);
    grn_log_reopen(context);
    rb_grn_context_check(context, related_object);
#endif

    return Qnil;
}

/*
 * call-seq:
 *   Groonga::Logger.reopen
 *
 * groongaのデフォルトロガーがログを出力するファイルを再オー
 * プンする。ログファイルのバックアップ時などに使用する。
 *
 * Groonga::Logger.registerで独自のロガーを設定している場合
 * は例外が発生する。
 */
static VALUE
rb_grn_logger_s_reopen (VALUE klass)
{
    return rb_grn_logger_s_reopen_with_related_object(klass, klass);
}

#ifndef WIN32
static VALUE
rb_grn_logger_s_set_path (VALUE klass, VALUE rb_path,
			  const char **path, const char *class_variable_name)
{
    grn_bool need_reopen = GRN_FALSE;

    if (NIL_P(rb_path)) {
	need_reopen = *path != NULL;
	*path = NULL;
    } else {
	const char *current_path = *path;
	*path = RSTRING_PTR(rb_path);
	if (!current_path || strcmp(*path, current_path) != 0) {
	    need_reopen = GRN_TRUE;
	}
    }
    rb_cv_set(klass, class_variable_name, rb_path);

    if (need_reopen) {
	rb_grn_logger_s_reopen_with_related_object(klass, rb_path);
    }

    return Qnil;
}
#endif

/*
 * call-seq:
 *   Groonga::Logger.log_path # => path
 *
 * groongaのデフォルトロガーがログを出力するファイルの
 * パスを返す。
 */
static VALUE
rb_grn_logger_s_get_log_path (VALUE klass)
{
#ifdef WIN32
    rb_raise(rb_eNotImpError, "grn_log_path isn't available on Windows.");
    return Qnil;
#else
    if (grn_log_path) {
	return rb_str_new2(grn_log_path);
    } else {
	return Qnil;
    }
#endif
}

/*
 * call-seq:
 *   Groonga::Logger.log_path= path
 *
 * groongaのデフォルトロガーがログを出力するファイルのパスを
 * 指定する。
 *
 * Groonga::Logger.registerで独自のロガーを設定している場合、
 * 設定している独自ロガーは無効になる。
 */
static VALUE
rb_grn_logger_s_set_log_path (VALUE klass, VALUE path)
{
#ifdef WIN32
    rb_raise(rb_eNotImpError, "grn_qlog_path isn't available on Windows.");
    return Qnil;
#else
    return rb_grn_logger_s_set_path(klass, path, &grn_log_path, "@@log_path");
#endif
}

/*
 * call-seq:
 *   Groonga::Logger.query_log_path # => path
 *
 * groongaのデフォルトロガーがクエリログを出力するファイルの
 * パスを返す。
 */
static VALUE
rb_grn_logger_s_get_query_log_path (VALUE klass)
{
#ifdef WIN32
    rb_raise(rb_eNotImpError, "grn_qlog_path isn't available on Windows.");
    return Qnil;
#else
    if (grn_qlog_path) {
	return rb_str_new2(grn_qlog_path);
    } else {
	return Qnil;
    }
#endif
}

/*
 * call-seq:
 *   Groonga::Logger.query_log_path= path
 *
 * groongaのデフォルトロガーがクエリログを出力するファイルの
 * パスを指定する。
 *
 * Groonga::Logger.registerで独自のロガーを設定している場合、
 * 設定している独自ロガーは無効になる。
 */
static VALUE
rb_grn_logger_s_set_query_log_path (VALUE klass, VALUE path)
{
#ifdef WIN32
    rb_raise(rb_eNotImpError, "grn_qlog_path isn't available on Windows.");
    return Qnil;
#else
    return rb_grn_logger_s_set_path(klass, path, &grn_qlog_path,
				    "@@query_log_path");
#endif
}

void
rb_grn_init_logger (VALUE mGrn)
{
    cGrnLogger = rb_define_class_under(mGrn, "Logger", rb_cObject);
    rb_define_alloc_func(cGrnLogger, rb_grn_logger_alloc);

    rb_cv_set(cGrnLogger, "@@current_logger", Qnil);
    rb_cv_set(cGrnLogger, "@@log_path", Qnil);
    rb_cv_set(cGrnLogger, "@@query_log_path", Qnil);
    rb_define_singleton_method(cGrnLogger, "register",
                               rb_grn_logger_s_register, -1);
    rb_define_singleton_method(cGrnLogger, "reopen",
                               rb_grn_logger_s_reopen, 0);
    rb_define_singleton_method(cGrnLogger, "log_path",
                               rb_grn_logger_s_get_log_path, 0);
    rb_define_singleton_method(cGrnLogger, "log_path=",
                               rb_grn_logger_s_set_log_path, 1);
    rb_define_singleton_method(cGrnLogger, "query_log_path",
                               rb_grn_logger_s_get_query_log_path, 0);
    rb_define_singleton_method(cGrnLogger, "query_log_path=",
                               rb_grn_logger_s_set_query_log_path, 1);
    rb_set_end_proc(rb_grn_logger_reset, cGrnLogger);

    rb_define_method(cGrnLogger, "initialize", rb_grn_logger_initialize, -1);
}
