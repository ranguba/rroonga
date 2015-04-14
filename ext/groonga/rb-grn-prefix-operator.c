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

VALUE rb_cGrnPrefixOperator;

/*
 * Executes a prefix-search operation. Prefix-serach operation checks
 * whether `text` starts with `prefix` or not.
 *
 * @example Executes prefix-search operations with the default context
 *   Groonga::Operator::PREFIX.exec("Hello Rroonga", "Hello")   # => true
 *   Groonga::Operator::PREFIX.exec("Hello Rroonga", "Rroonga") # => false
 *
 * @example Executes prefix-search operations with the specified context
 *   context = Groonga::Context.new
 *   Groonga::Operator::PREFIX.exec("Hello Rroonga", "Hello",
 *                                  :context => context) # => true
 *   Groonga::Operator::PREFIX.exec("Hello Rroonga", "Rroonga",
 *                                  :context => context) # => false
 *
 * @overload exec(text, prefix, options={})
 *   @param text [String] The text to be searched.
 *   @param prefix [String] The prefix to be contained.
 *   @param options [::Hash] The options.
 *   @option options [Groonga::Context] (Groonga::Context.default)
 *      The context to executes the operation.
 *   @return [Boolean] `true` if `text` starts with `prefix`, `false`
 *      otherwise.
 */
static VALUE
rb_grn_prefix_operator_exec (int argc, VALUE *argv, VALUE self)
{
    grn_bool have_prefix;
    VALUE rb_text;
    VALUE rb_prefix;
    VALUE rb_options;
    VALUE rb_context;
    grn_ctx *context;
    grn_obj text;
    grn_obj prefix;

    rb_scan_args(argc, argv, "21", &rb_text, &rb_prefix, &rb_options);

    rb_grn_scan_options(rb_options,
                        "context", &rb_context,
                        NULL);
    context = rb_grn_context_ensure(&rb_context);

    GRN_VOID_INIT(&text);
    GRN_VOID_INIT(&prefix);
    RVAL2GRNBULK(rb_text, context, &text);
    RVAL2GRNBULK(rb_prefix, context, &prefix);
    have_prefix = grn_operator_exec_prefix(context, &text, &prefix);
    GRN_OBJ_FIN(context, &text);
    GRN_OBJ_FIN(context, &prefix);

    return CBOOL2RVAL(have_prefix);
}

void
rb_grn_init_prefix_operator (VALUE mGrn)
{
    rb_cGrnPrefixOperator = rb_define_class_under(mGrn,
                                                 "PrefixOperator",
                                                 rb_cGrnOperator);

    rb_define_method(rb_cGrnPrefixOperator, "exec",
                     rb_grn_prefix_operator_exec, -1);
}
