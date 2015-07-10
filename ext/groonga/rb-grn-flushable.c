/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2015  Masafumi Yokoyama <yokoyama@clear-code.com>

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

#define SELF(object) (RB_GRN_OBJECT(DATA_PTR(object)))

/*
 * Document-module: Groonga::Flushable
 *
 * It provides the ability to flush memory for an object.
 */

/*
 * It flushes memory mapped data to disk.
 *
 * @return [void]
 */
static VALUE
rb_grn_flushable_flush (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *object = NULL;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);
    if (!object) {
        rb_raise(rb_eGrnClosed,
                 "can't access already closed Groonga object: <%s>",
                 rb_grn_inspect(self));
    }

    rb_grn_context_check(context, self);

    grn_obj_flush_recursive(context, object);

    return Qnil;
}

void
rb_grn_init_flushable (VALUE mGrn)
{
    VALUE rb_mGrnFlushable;

    rb_mGrnFlushable = rb_define_module_under(mGrn, "Flushable");

    rb_define_method(rb_mGrnFlushable, "flush",
                     rb_grn_flushable_flush, 0);
}
