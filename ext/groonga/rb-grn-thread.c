/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2015  Kouhei Sutou <kou@clear-code.com>

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

static ID id_call;

/*
 * Document-class: Groonga::Thread
 *
 * It's a namespace for thread related features.
 */

/*
 * @overload limit()
 *
 *   @return [Integer] the max number of threads. `0` means that
 *     thread isn't supported.
 *
 * @since 5.0.5
 */
static VALUE
rb_grn_thread_s_get_limit (VALUE self)
{
    uint32_t limit;

    limit = grn_thread_get_limit();

    return UINT2NUM(limit);
}

/*
 * @overload limit=(new_limit)
 *
 *   Sets the max number of threads.
 *
 *   @param new_limit [Integer] The new max number of threads.
 *
 *   @return [void]
 *
 * @since 5.0.5
 */
static VALUE
rb_grn_thread_s_set_limit (VALUE self, VALUE rb_new_limit)
{
    uint32_t new_limit;

    new_limit = NUM2UINT(rb_new_limit);
    grn_thread_set_limit(new_limit);

    return Qnil;
}

static uint32_t
rb_grn_thread_get_limit_func (void *data)
{
    VALUE callable = (VALUE)data;
    VALUE rb_limit;

    rb_limit = rb_funcall(callable, id_call, 0);

    return NUM2UINT(rb_limit);
}

/*
 * @overload limit_getter=(getter)
 *
 *   Sets a callable object that returns the max number of threads.
 *
 *   @param getter [#call] Callable object that returns the max number
 *     of threads.
 *
 *   @return [void]
 *
 * @since 5.0.5
 */
static VALUE
rb_grn_thread_s_set_limit_getter (VALUE self, VALUE rb_getter)
{
    rb_iv_set(self, "@limit_getter", rb_getter);

    if (NIL_P(rb_getter)) {
        grn_thread_set_get_limit_func(NULL, NULL);
    } else {
        grn_thread_set_get_limit_func(rb_grn_thread_get_limit_func,
                                      (void *)rb_getter);
    }

    return Qnil;
}

static void
rb_grn_thread_set_limit_func (uint32_t new_limit, void *data)
{
    VALUE callable = (VALUE)data;

    rb_funcall(callable, id_call, 1, UINT2NUM(new_limit));
}

/*
 * @overload limit_setter=(setter)
 *
 *   Sets a callable object that sets the max number of threads.
 *
 *   @param setter [#call] Callable object that sets the max number
 *     of threads.
 *
 *   @return [void]
 *
 * @since 5.0.5
 */
static VALUE
rb_grn_thread_s_set_limit_setter (VALUE self, VALUE rb_setter)
{
    rb_iv_set(self, "@limit_setter", rb_setter);

    if (NIL_P(rb_setter)) {
        grn_thread_set_set_limit_func(NULL, NULL);
    } else {
        grn_thread_set_set_limit_func(rb_grn_thread_set_limit_func,
                                      (void *)rb_setter);
    }

    return Qnil;
}

void
rb_grn_init_thread (VALUE mGrn)
{
    VALUE rb_mGrnThread;

    CONST_ID(id_call, "call");

    rb_mGrnThread = rb_define_module_under(mGrn, "Thread");

    rb_define_singleton_method(rb_mGrnThread, "limit",
                               rb_grn_thread_s_get_limit, 0);
    rb_define_singleton_method(rb_mGrnThread, "limit=",
                               rb_grn_thread_s_set_limit, 1);

    rb_define_singleton_method(rb_mGrnThread, "limit_getter=",
                               rb_grn_thread_s_set_limit_getter, 1);
    rb_define_singleton_method(rb_mGrnThread, "limit_setter=",
                               rb_grn_thread_s_set_limit_setter, 1);
}
