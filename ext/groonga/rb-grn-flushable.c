/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2015  Masafumi Yokoyama <yokoyama@clear-code.com>
  Copyright (C) 2021  Sutou Kouhei <kou@clear-code.com>

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

#define SELF(object) (RB_GRN_OBJECT(RTYPEDDATA_DATA(object)))

VALUE rb_mGrnFlushable;

/*
 * Document-module: Groonga::Flushable
 *
 * It provides the ability to flush data on memory to disk.
 */

/*
 * Flush memory mapped data to disk.
 *
 * @overload flush(options={})
 *   @param [::Hash] options
 *   @option options [Boolean] :recursive (true) Whether to flush objects
 *     which a target object has recursively.
 *   @return [void]
 */
static VALUE
rb_grn_flushable_flush (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *object = NULL;
    VALUE rb_recursive_p;
    VALUE rb_dependent_p;
    VALUE rb_options;

    rb_scan_args(argc, argv, "01", &rb_options);
    rb_grn_scan_options(rb_options,
                        "recursive", &rb_recursive_p,
                        "dependent", &rb_dependent_p,
                        NULL);
    if (NIL_P(rb_recursive_p)) {
        rb_recursive_p = Qtrue;
    }
    if (NIL_P(rb_dependent_p)) {
        rb_dependent_p = Qfalse;
    }

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);
    if (!object) {
        rb_raise(rb_eGrnClosed,
                 "can't access already closed Groonga object: <%s>",
                 rb_grn_inspect(self));
    }

    if (RVAL2CBOOL(rb_recursive_p)) {
        grn_obj_flush_recursive(context, object);
    } else if (RVAL2CBOOL(rb_dependent_p)) {
        grn_obj_flush_recursive_dependent(context, object);
    } else {
        grn_obj_flush(context, object);
    }
    rb_grn_context_check(context, self);

    return Qnil;
}

void
rb_grn_init_flushable (VALUE mGrn)
{
    rb_mGrnFlushable = rb_define_module_under(mGrn, "Flushable");

    rb_define_method(rb_mGrnFlushable, "flush",
                     rb_grn_flushable_flush, -1);
}
