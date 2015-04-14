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

VALUE rb_cGrnRegexpOperator;

/*
 * Executes a regular expression match operation.
 *
 * @example Executes regular expression match operations with the default context
 *   Groonga::Operator::REGEXP.exec("Hello Rroonga", /Rro+nga/) # => true
 *   Groonga::Operator::REGEXP.exec("Hello Rroonga", /Gro+nga/) # => false
 *
 * @example Executes regular expression match operations with the specified context
 *   context = Groonga::Context.new
 *   Groonga::Operator::REGEXP.exec("Hello Rroonga", /Rro+nga/,
 *                                :context => context) # => true
 *   Groonga::Operator::REGEXP.exec("Hello Rroonga", /Gro+nga/,
 *                                :context => context) # => false
 *
 * @overload exec(text, regexp, options={})
 *   @param text [String] The text to be matched.
 *   @param regexp [Regexp] The regular expression.
 *   @param options [::Hash] The options.
 *   @option options [Groonga::Context] (Groonga::Context.default)
 *      The context to executes the operation.
 *   @return [Boolean] `true` if `text` matches `regexp`, `false`
 *      otherwise.
 */
static VALUE
rb_grn_regexp_operator_exec (int argc, VALUE *argv, VALUE self)
{
    grn_bool matched;
    VALUE rb_text;
    VALUE rb_regexp;
    VALUE rb_options;
    VALUE rb_context;
    grn_ctx *context;
    grn_obj text;
    grn_obj regexp;

    rb_scan_args(argc, argv, "21", &rb_text, &rb_regexp, &rb_options);

    rb_grn_scan_options(rb_options,
                        "context", &rb_context,
                        NULL);
    context = rb_grn_context_ensure(&rb_context);

    GRN_VOID_INIT(&text);
    GRN_VOID_INIT(&regexp);
    RVAL2GRNBULK(rb_text, context, &text);
    RVAL2GRNBULK(rb_regexp, context, &regexp);
    matched = grn_operator_exec_regexp(context, &text, &regexp);
    GRN_OBJ_FIN(context, &text);
    GRN_OBJ_FIN(context, &regexp);

    return CBOOL2RVAL(matched);
}

void
rb_grn_init_regexp_operator (VALUE mGrn)
{
    rb_cGrnRegexpOperator = rb_define_class_under(mGrn,
                                                 "RegexpOperator",
                                                 rb_cGrnOperator);

    rb_define_method(rb_cGrnRegexpOperator, "exec",
                     rb_grn_regexp_operator_exec, -1);
}
