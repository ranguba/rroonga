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

VALUE rb_cGrnMatchOperator;

/*
 * Executes a match operation. Match operation checks whether `text`
 * contains `sub_text` or not.
 *
 * @example Executes match operations with the default context
 *   Groonga::Operator::MATCH.exec("Hello Rroonga", "Rroonga") # => true
 *   Groonga::Operator::MATCH.exec("Hello Rroonga", "Groonga") # => false
 *
 * @example Executes match operations with the specified context
 *   context = Groonga::Context.new
 *   Groonga::Operator::MATCH.exec("Hello Rroonga", "Rroonga",
 *                                :context => context) # => true
 *   Groonga::Operator::MATCH.exec("Hello Rroonga", "Groonga",
 *                                :context => context) # => false
 *
 * @overload exec(text, sub_text, options={})
 *   @param text [String] The text to be matched.
 *   @param sub_text [String] The sub text to be contained.
 *   @param options [::Hash] The options.
 *   @option options [Groonga::Context] (Groonga::Context.default)
 *      The context to executes the operation.
 *   @return [Boolean] `true` if `text` contains `sub_text`, `false`
 *      otherwise.
 */
static VALUE
rb_grn_match_operator_exec (int argc, VALUE *argv, VALUE self)
{
    grn_bool matched;
    VALUE rb_text;
    VALUE rb_sub_text;
    VALUE rb_options;
    VALUE rb_context;
    grn_ctx *context;
    grn_obj text;
    grn_obj sub_text;

    rb_scan_args(argc, argv, "21", &rb_text, &rb_sub_text, &rb_options);

    rb_grn_scan_options(rb_options,
                        "context", &rb_context,
                        NULL);
    context = rb_grn_context_ensure(&rb_context);

    GRN_VOID_INIT(&text);
    GRN_VOID_INIT(&sub_text);
    RVAL2GRNBULK(rb_text, context, &text);
    RVAL2GRNBULK(rb_sub_text, context, &sub_text);
    matched = grn_operator_exec_match(context, &text, &sub_text);
    GRN_OBJ_FIN(context, &text);
    GRN_OBJ_FIN(context, &sub_text);

    return CBOOL2RVAL(matched);
}

void
rb_grn_init_match_operator (VALUE mGrn)
{
    rb_cGrnMatchOperator = rb_define_class_under(mGrn,
                                                 "MatchOperator",
                                                 rb_cGrnOperator);

    rb_define_method(rb_cGrnMatchOperator, "exec",
                     rb_grn_match_operator_exec, -1);
}
