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

#include "rb-grn.h"

grn_bool rb_grn_exited = GRN_FALSE;

static VALUE
finish_groonga (RB_BLOCK_CALL_FUNC_ARGLIST(yielded_arg, callback_arg))
{
    debug("finish\n");
    grn_fin();
    rb_grn_exited = GRN_TRUE;
    debug("finish: done\n");
    return Qnil;
}

/*
 * @overload error_message
 *   @return [String, nil] The process global error message.
 *
 * @since 6.0.0
 */
static VALUE
rb_grn_s_get_error_message (VALUE klass)
{
    const char *message;

    message = grn_get_global_error_message();
    if (message[0] == '\0') {
        return Qnil;
    } else {
        grn_encoding encoding = grn_get_default_encoding();
        return rb_enc_str_new_cstr(message,
                                   rb_grn_encoding_to_ruby_encoding(encoding));
    }
}

static void
rb_grn_init_error_message (VALUE mGrn)
{
    rb_define_singleton_method(mGrn, "error_message",
                               rb_grn_s_get_error_message, 0);
}

static void
rb_grn_init_runtime_version (VALUE mGrn)
{
    const char *component_start, *component_end;
    int component_length;
    VALUE runtime_version;
    VALUE major, minor, micro, tag;

    runtime_version = rb_ary_new();

    component_start = grn_get_version();
    component_end = strstr(component_start, ".");
    component_length = component_end - component_start;
    major = rb_str_new(component_start, component_length);
    rb_ary_push(runtime_version, rb_Integer(major));

    component_start = component_end + 1;
    component_end = strstr(component_start, ".");
    component_length = component_end - component_start;
    minor = rb_str_new(component_start, component_length);
    rb_ary_push(runtime_version, rb_Integer(minor));

    component_start = component_end + 1;
    component_end = strstr(component_start, "-");
    if (component_end) {
        component_length = component_end - component_start;
    } else {
        component_length = strlen(component_start);
    }
    micro = rb_str_new(component_start, component_length);
    rb_ary_push(runtime_version, rb_Integer(micro));

    if (component_end) {
        tag = rb_str_new_cstr(component_end + 1);
    } else {
        tag = Qnil;
    }
    rb_ary_push(runtime_version, tag);

    rb_obj_freeze(runtime_version);
    /*
     * 利用しているgroongaのバージョン。 @[メジャーバージョ
     * ン, マイナーバージョン, マイクロバージョン, タグ]@ の
     * 配列。
     */
    rb_define_const(mGrn, "VERSION", runtime_version);
}

static void
rb_grn_init_version (VALUE mGrn)
{
    VALUE build_version, bindings_version;

    rb_grn_init_runtime_version(mGrn);

    build_version = rb_ary_new_from_args(3,
                                         INT2NUM(GRN_MAJOR_VERSION),
                                         INT2NUM(GRN_MINOR_VERSION),
                                         INT2NUM(GRN_MICRO_VERSION));
    rb_obj_freeze(build_version);
    /*
     * ビルドしたgroongaのバージョン。 @[メジャーバージョン,
     * マイナーバージョン, マイクロバージョン]@ の配列。
     */
    rb_define_const(mGrn, "BUILD_VERSION", build_version);

    bindings_version = rb_ary_new_from_args(3,
                                            INT2NUM(RB_GRN_MAJOR_VERSION),
                                            INT2NUM(RB_GRN_MINOR_VERSION),
                                            INT2NUM(RB_GRN_MICRO_VERSION));
    rb_obj_freeze(bindings_version);
    /*
     * rroongaのバージョン。 @[メジャーバージョン, マ
     * イナーバージョン, マイクロバージョン]@ の配列。
     */
    rb_define_const(mGrn, "BINDINGS_VERSION", bindings_version);
}

/*
 * Returns the current lock timeout.
 *
 * See
 * http://groonga.org/docs/reference/api/global_configurations.html
 * about lock timeout.
 *
 * @overload lock_timeout
 *    @return [Integer] The current lock timeout.
 */
static VALUE
rb_grn_s_get_lock_timeout (VALUE klass)
{
    return INT2NUM(grn_get_lock_timeout());
}

/*
 * Sets the current lock timeout.
 *
 * @overload lock_timeout=(timeout)
 *    @param [Integer] timeout The new lock timeout.
 *
 * @see lock_timeout
 * @since 3.1.2
 */
static VALUE
rb_grn_s_set_lock_timeout (VALUE klass, VALUE timeout)
{
    grn_set_lock_timeout(NUM2INT(timeout));
    return Qnil;
}

static void
rb_grn_init_lock_timeout (VALUE mGrn)
{
    rb_define_singleton_method(mGrn, "lock_timeout",
                               rb_grn_s_get_lock_timeout, 0);
    rb_define_singleton_method(mGrn, "lock_timeout=",
                               rb_grn_s_set_lock_timeout, 1);
}

/*
 * Returns the Groonga package's label. It's `"Groonga"`.
 *
 * @example How to get the Groonga package's label
 *   Groonga.package_label # => "Groonga"
 *
 * @overload package_label
 *   @return [String] `"Groonga"`
 *
 * @since 5.1.1
 */
static VALUE
rb_grn_s_get_package_label (VALUE klass)
{
    return rb_str_new_cstr(grn_get_package_label());
}

static void
rb_grn_init_package_label (VALUE mGrn)
{
    rb_define_singleton_method(mGrn, "package_label",
                               rb_grn_s_get_package_label, 0);
}

void
Init_groonga (void)
{
#ifdef HAVE_RB_EXT_RACTOR_SAFE
    rb_ext_ractor_safe(true);
#endif

    VALUE mGrn;

    mGrn = rb_define_module("Groonga");

    rb_grn_init_error_message(mGrn);
    rb_grn_init_exception(mGrn);

    rb_grn_rc_check(grn_init(), Qnil);
    rb_define_finalizer(mGrn, rb_proc_new(finish_groonga, Qnil));

    rb_grn_init_version(mGrn);
    rb_grn_init_lock_timeout(mGrn);
    rb_grn_init_package_label(mGrn);

    rb_grn_init_utils(mGrn);
    rb_grn_init_encoding(mGrn);
    rb_grn_init_encoding_support(mGrn);
    rb_grn_init_flushable(mGrn);
    rb_grn_init_context(mGrn);
    rb_grn_init_object(mGrn);
    rb_grn_init_database(mGrn);
    rb_grn_init_table(mGrn);
    rb_grn_init_table_cursor(mGrn);
    rb_grn_init_index_cursor(mGrn);
    rb_grn_init_inverted_index_cursor(mGrn);
    rb_grn_init_posting(mGrn);
    rb_grn_init_type(mGrn);
    rb_grn_init_procedure(mGrn);
    rb_grn_init_procedure_type(mGrn);
    rb_grn_init_column(mGrn);
    rb_grn_init_accessor(mGrn);
    rb_grn_init_geo_point(mGrn);
    rb_grn_init_record(mGrn);
    rb_grn_init_variable(mGrn);
    rb_grn_init_operator(mGrn);
    rb_grn_init_expression(mGrn);
    rb_grn_init_expression_builder(mGrn);
    rb_grn_init_logger(mGrn);
    rb_grn_init_query_logger(mGrn);
    rb_grn_init_windows_event_logger(mGrn);
    rb_grn_init_snippet(mGrn);
    rb_grn_init_plugin(mGrn);
    rb_grn_init_normalizer(mGrn);
    rb_grn_init_thread(mGrn);
    rb_grn_init_config(mGrn);
    rb_grn_init_index(mGrn);
    rb_grn_init_request_canceler(mGrn);
    rb_grn_init_request_timer(mGrn);
    rb_grn_init_request_timer_id(mGrn);
    rb_grn_init_id(mGrn);
    rb_grn_init_name(mGrn);
    rb_grn_init_default_cache(mGrn);
    rb_grn_init_column_cache(mGrn);
}
