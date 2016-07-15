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
 * Document-class: Groonga::Name
 *
 * This module provides object name related features.
 */

/*
 * @overload column?(name)
 *   @param [String] name The name to be confirmed.
 *   @return [Boolean] `true` if the `name` is column, `false` otherwise.
 *
 * @since 6.0.5
 */
static VALUE
rb_grn_name_s_column_p (VALUE self, VALUE rb_name)
{
    grn_ctx *ctx = NULL;
    const char *name;
    size_t name_size;

    name = StringValueCStr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    return CBOOL2RVAL(grn_obj_name_is_column(ctx, name, name_size));
}

void
rb_grn_init_name (VALUE mGrn)
{
    VALUE mGrnName;

    mGrnName = rb_define_module_under(mGrn, "Name");

    rb_define_singleton_method(mGrnName, "column?", rb_grn_name_s_column_p, 1);
}
