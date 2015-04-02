/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2011  Kouhei Sutou <kou@clear-code.com>

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

#define SELF(object) (RVAL2GRNCONTEXT(object))

static VALUE cGrnPlugin;

/*
 * Document-class: Groonga::Plugin
 *
 * プラグイン。現在のところ、Rubyでgroongaのプラグインを作成
 * することはできず、Cで作成された既存のプラグインを登録する
 * ことができるだけです。
 */

grn_id
rb_grn_plugin_from_ruby_object (VALUE object)
{
    RbGrnPlugin *rb_grn_plugin;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, cGrnPlugin))) {
        rb_raise(rb_eTypeError, "not a groonga plugin");
    }

    Data_Get_Struct(object, RbGrnPlugin, rb_grn_plugin);
    if (!rb_grn_plugin)
        rb_raise(rb_eGrnError, "groonga plugin is NULL");
    return rb_grn_plugin->id;
}

void
rb_grn_plugin_fin (grn_id id)
{
}

static void
rb_grn_plugin_free (void *pointer)
{
    RbGrnPlugin *rb_grn_plugin = pointer;

    xfree(rb_grn_plugin);
}

static VALUE
rb_grn_plugin_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_plugin_free, NULL);
}

/*
 * 既存のプラグインをデータベースに登録する。
 *
 * @overload register(name, options=nil)
 *   _name_ で指定した名前のプラグインを登録する。
 *   @param [String] name 登録するプラグインの名前
 *   @param options [::Hash] The name and value
 *     pairs. Omitted names are initialized as the default value.
 *   @option options :context (Groonga::Context.default)
 *     データベースを結びつけるコンテキスト。
 * @overload register({:path => path, :context => nil})
 *   @param hash [::Hash] _:path_ と _:context_ を指定したハッシュを指定する。
 *   _:path_ で指定したパスのプラグインを登録する。
 *
 *   _:context_ については _name_ を設定した時と同様。
 */
static VALUE
rb_grn_plugin_s_register (int argc, VALUE *argv, VALUE klass)
{
    const char *name = NULL, *path = NULL;
    VALUE rb_options, rb_name = Qnil, rb_path, rb_context;
    grn_ctx *context;

    if (argc >= 1) {
        rb_name = rb_grn_check_convert_to_string(argv[0]);
    }

    if (NIL_P(rb_name)) {
        rb_scan_args(argc, argv, "01", &rb_options);
        rb_grn_scan_options(rb_options,
                            "path", &rb_path,
                            "context", &rb_context,
                            NULL);
        path = StringValueCStr(rb_path);
    } else {
        rb_scan_args(argc, argv, "11", &rb_name, &rb_options);
        rb_grn_scan_options(rb_options,
                            "context", &rb_context,
                            NULL);
        name = StringValueCStr(rb_name);
    }

    if (NIL_P(rb_context)) {
        rb_context = rb_grn_context_get_default();
    }
    context = RVAL2GRNCONTEXT(rb_context);

    if (name) {
        grn_plugin_register(context, name);
    } else {
        grn_plugin_register_by_path(context, path);
    }

    rb_grn_context_check(context, rb_ary_new4(argc, argv));
    return Qnil;
}

/*
 * Unregister already registered plugin
 *
 * @overload unregister(name, options=nil)
 *   Unregister specified `name` plugin.
 *   @param [String] name The name of plugin
 *   @param options [::Hash] The name and value
 *     pairs. Omitted names are initialized as the default value.
 *   @option options :context (Groonga::Context.default)
 *     The context which is bound to database.
 */
static VALUE
rb_grn_plugin_s_unregister (int argc, VALUE *argv, VALUE klass)
{
    const char *name = NULL;
    VALUE rb_options, rb_name = Qnil, rb_context;
    grn_ctx *context;

    if (argc >= 1) {
        rb_name = rb_grn_check_convert_to_string(argv[0]);
    }

    rb_scan_args(argc, argv, "11", &rb_name, &rb_options);
    rb_grn_scan_options(rb_options,
                        "context", &rb_context,
                        NULL);
    name = StringValueCStr(rb_name);

    if (NIL_P(rb_context)) {
        rb_context = rb_grn_context_get_default();
    }
    context = RVAL2GRNCONTEXT(rb_context);

    grn_plugin_unregister(context, name);

    rb_grn_context_check(context, rb_ary_new4(argc, argv));
    return Qnil;
}

/*
 * Returns the system plugins directory.
 *
 * @overload system_plugins_dir
 *   @return [String]
 */
static VALUE
rb_grn_plugin_s_system_plugins_dir (VALUE klass)
{
    return rb_str_new_cstr(grn_plugin_get_system_plugins_dir());
}

/*
 * Returns the plugin file suffix. (e.g. ".so", ".dll" and so on.)
 *
 * @overload suffix
 *   @return [String]
 */
static VALUE
rb_grn_plugin_s_suffix (VALUE klass)
{
    return rb_str_new_cstr(grn_plugin_get_suffix());
}

void
rb_grn_init_plugin (VALUE mGrn)
{
    cGrnPlugin = rb_define_class_under(mGrn, "Plugin", rb_cObject);
    rb_define_alloc_func(cGrnPlugin, rb_grn_plugin_alloc);

    rb_define_singleton_method(cGrnPlugin, "register",
                               rb_grn_plugin_s_register, -1);
    rb_define_singleton_method(cGrnPlugin, "unregister",
                               rb_grn_plugin_s_unregister, -1);
    rb_define_singleton_method(cGrnPlugin, "system_plugins_dir",
                               rb_grn_plugin_s_system_plugins_dir, 0);
    rb_define_singleton_method(cGrnPlugin, "suffix",
                               rb_grn_plugin_s_suffix, 0);
}
