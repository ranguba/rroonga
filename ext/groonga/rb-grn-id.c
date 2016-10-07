/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2016  Kouhei Sutou <kou@clear-code.com>

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
 * Document-class: Groonga::ID
 *
 * This module provides Groonga ID related features.
 */

/*
 * @overload builtin?(id)
 *   @param [Integer] id The ID to be confirmed.
 *   @return [Boolean] `true` if the `id` is builtin, `false` otherwise.
 *
 * @since 6.0.5
 */
static VALUE
rb_grn_id_s_builtin_p (VALUE self, VALUE rb_id)
{
    grn_ctx *ctx = NULL;
    grn_id id;

    id = NUM2INT(rb_id);

    return CBOOL2RVAL(grn_id_is_builtin(ctx, id));
}

/*
 * @overload builtin_type?(id)
 *   @param [Integer] id The ID to be confirmed.
 *   @return [Boolean] `true` if the `id` is builtin_type, `false` otherwise.
 *
 * @since 6.0.9
 */
static VALUE
rb_grn_id_s_builtin_type_p (VALUE self, VALUE rb_id)
{
    grn_ctx *ctx = NULL;
    grn_id id;

    id = NUM2INT(rb_id);

    return CBOOL2RVAL(grn_id_is_builtin_type(ctx, id));
}

void
rb_grn_init_id (VALUE mGrn)
{
    VALUE mGrnID;

    mGrnID = rb_define_module_under(mGrn, "ID");

    rb_define_const(mGrnID, "NIL", INT2NUM(GRN_ID_NIL));
    rb_define_const(mGrnID, "MAX", INT2NUM(GRN_ID_MAX));

    rb_define_singleton_method(mGrnID, "builtin?", rb_grn_id_s_builtin_p, 1);
    rb_define_singleton_method(mGrnID, "builtin_type?", rb_grn_id_s_builtin_type_p, 1);
}
