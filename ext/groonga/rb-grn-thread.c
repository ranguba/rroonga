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

VALUE rb_cGrnDatabase;

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

void
rb_grn_init_thread (VALUE mGrn)
{
    VALUE rb_mGrnThread;

    rb_mGrnThread = rb_define_module_under(mGrn, "Thread");

    rb_define_singleton_method(rb_mGrnThread, "limit",
                               rb_grn_thread_s_get_limit, 0);
    rb_define_singleton_method(rb_mGrnThread, "limit=",
                               rb_grn_thread_s_set_limit, 1);
}
