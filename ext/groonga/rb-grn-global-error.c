/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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

/*
 * Document-class: Groonga::GlobalError
 *
 * This class manages global error in the current process.
 */

/*
 * Gets the global error message in current process.
 *
 * @overload message
 *   @return [String, nil] The error message.
 *
 * @since 6.0.0
 */
static VALUE
rb_grn_global_error_get_message (VALUE klass)
{
    return rb_str_new_cstr(grn_get_global_error_message());
}

void
rb_grn_init_global_error (VALUE mGrn)
{
    VALUE cGrnGlobalError;

    cGrnGlobalError = rb_define_class_under(mGrn, "GlobalError", rb_cObject);

    rb_define_singleton_method(cGrnGlobalError, "message",
                               rb_grn_global_error_get_message, 0);
}
