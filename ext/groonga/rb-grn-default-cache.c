/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2017  Kouhei Sutou <kou@clear-code.com>

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
 * Document-module: Groonga::DefaultCache
 *
 * This module provides the default cache related features.
 *
 * @since 7.0.2
 */

/*
 * @overload base_path
 *   @return [String, nil] the default cache base path or `nil`.
 *
 * @since 7.0.2
 */
static VALUE
rb_grn_default_cache_s_get_base_path (VALUE klass)
{
    const char *base_path;

    base_path = grn_get_default_cache_base_path();
    if (base_path) {
        return rb_str_new_cstr(base_path);
    } else {
        return Qnil;
    }
}

/*
 * @overload base_path=(base_path)
 *   @param base_path [String, nil] The new default cache base path.
 *      If you specify `String`, the default cache will be persistent.
 *      If you specify `nil`, the default cache will be just on memory.
 *      You need to call {.reopen} to apply this change.
 *   @return [String, nil] The `base_path` itself.
 *
 * @since 7.0.2
 */
static VALUE
rb_grn_default_cache_s_set_base_path (VALUE klass, VALUE rb_base_path)
{
    if (NIL_P(rb_base_path)) {
        grn_set_default_cache_base_path(NULL);
    } else {
        const char *base_path;
        base_path = StringValueCStr(rb_base_path);
        grn_set_default_cache_base_path(base_path);
    }

    return rb_base_path;
}

/*
 * @overload reopen
 *   Reopens the default cache.
 *
 *   @return [void]
 *
 * @since 7.0.2
 */
static VALUE
rb_grn_default_cache_s_reopen (VALUE klass)
{
    grn_rc rc;

    rc = grn_cache_default_reopen();
    rb_grn_rc_check(rc, klass);

    return Qnil;
}

void
rb_grn_init_default_cache (VALUE mGrn)
{
    VALUE mGrnDefaultCache;

    mGrnDefaultCache = rb_define_module_under(mGrn, "DefaultCache");

    rb_define_singleton_method(mGrnDefaultCache, "base_path",
                               rb_grn_default_cache_s_get_base_path, 0);
    rb_define_singleton_method(mGrnDefaultCache, "base_path=",
                               rb_grn_default_cache_s_set_base_path, 1);
    rb_define_singleton_method(mGrnDefaultCache, "reopen",
                               rb_grn_default_cache_s_reopen, 0);
}
