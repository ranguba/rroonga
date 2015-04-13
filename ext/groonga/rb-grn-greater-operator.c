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

VALUE rb_cGrnGreaterOperator;

/*
 * Executes a greater operation.
 *
 * @example Executes greater operations with the default context
 *   Groonga::Operator::GREATER.exec(2, 1) # => true
 *   Groonga::Operator::GREATER.exec(1, 2) # => false
 *
 * @example Executes greater operations with the specified context
 *   context = Groonga::Context.new
 *   Groonga::Operator::GREATER.exec(2, 1,
 *                                   :context => context) # => true
 *   Groonga::Operator::GREATER.exec(1, 2,
 *                                   :context => context) # => false
 *
 * @overload exec(x, y, options={})
 *   @param x [::Object] The left hand side value.
 *   @param y [::Object] The right hand side value.
 *   @param options [::Hash] The options.
 *   @option options [Groonga::Context] (Groonga::Context.default)
 *      The context to executes the operation.
 *   @return [Boolean] `true` if `x` is greater than `y`, `false`
 *      otherwise.
 */
static VALUE
rb_grn_greater_operator_exec (int argc, VALUE *argv, VALUE self)
{
    grn_bool greater;
    VALUE rb_x;
    VALUE rb_y;
    VALUE rb_options;
    VALUE rb_context;
    grn_ctx *context;
    grn_obj x;
    grn_obj y;

    rb_scan_args(argc, argv, "21", &rb_x, &rb_y, &rb_options);

    rb_grn_scan_options(rb_options,
                        "context", &rb_context,
                        NULL);
    context = rb_grn_context_ensure(&rb_context);

    GRN_VOID_INIT(&x);
    GRN_VOID_INIT(&y);
    RVAL2GRNBULK(rb_x, context, &x);
    RVAL2GRNBULK(rb_y, context, &y);
    greater = grn_operator_exec_greater(context, &x, &y);
    GRN_OBJ_FIN(context, &x);
    GRN_OBJ_FIN(context, &y);

    return CBOOL2RVAL(greater);
}

void
rb_grn_init_greater_operator (VALUE mGrn)
{
    rb_cGrnGreaterOperator = rb_define_class_under(mGrn,
                                                   "GreaterOperator",
                                                   rb_cGrnOperator);

    rb_define_method(rb_cGrnGreaterOperator, "exec",
                     rb_grn_greater_operator_exec, -1);
}
