/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sts=4 sw=4 ts=8 noet: */
/*
  Copyright (C) 2016-2022  Sutou Kouhei <kou@clear-code.com>

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

#define SELF(object) ((RbGrnColumn *)RTYPEDDATA_DATA(object))

VALUE rb_cGrnDataColumn;

/*
 * Document-class: Groonga::DataColumn < Groonga::Column
 *
 * The superclass for data columns such as {Groonga::FixSizeColumn}
 * and {Groonga::VariableSizeColumn}.
 */

/*
 * Applies the window function to records in the table of the
 * column. Results are stored into the column.
 *
 * @example Set the nth record.
 *
 *   Groonga::Schema.define do |schema|
 *     schema.create_table("Comments") do |table|
 *       # The column for storing window function result.
 *       table.uint32("nth")
 *     end
 *   end
 *   comments = Groonga["Comments"]
 *   nth = Groonga["Comments.nth"]
 *
 *   5.times do
 *     comments.add
 *   end
 *
 *   options = {
 *     # Sort by _id in descending order.
 *     :sort_keys => [["_id", "desc"]],
 *   }
 *   nth.apply_window_function(options) do |record|
 *     # record_number() returns Nth record in the sorted order.
 *     record.call("record_number")
 *   end
 *
 *   comments.each do |comment|
 *     p [comment.id, comment.nth]
 *       # -> [1, 5]
 *       # -> [2, 4]
 *       # -> [3, 3]
 *       # -> [4, 2]
 *       # -> [5, 1]
 *   end
 *
 * @overload apply_window_function(options={}) {|record| }
 *   @param options [::Hash] The name and value pairs.
 *   @option options [::Array<String>] :sort_keys
 *     TODO: Description.
 *   @option options [::Array<String>] :group_keys
 *     TODO: Description.
 *
 *   @yield [record]
 *     It yields an object that builds expression. The block must
 *     build an expression that calls a window function.
 *   @yieldparam [Groonga::RecordExpressionBuilder] record
 *     The expression builder to create a window function call.
 *   @yieldreturn [Groonga::ExpressionBuilder]
 *     It must be an expression that calls a window function.
 *
 * @since 6.0.4
 */
static VALUE
rb_grn_data_column_apply_window_function (int argc, VALUE *argv, VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *column;
    grn_obj *table;
    VALUE rb_table;
    grn_window_definition definition;
    grn_obj *window_function_call = NULL;
    VALUE rb_options;
    VALUE rb_sort_keys;
    VALUE rb_group_keys;
    VALUE rb_builder;
    VALUE rb_window_function_call;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, &table,
                              NULL, NULL, NULL);
    rb_table = GRNOBJECT2RVAL(Qnil, context, table, GRN_FALSE);

    memset(&definition, 0, sizeof(grn_window_definition));

    rb_scan_args(argc, argv, "01", &rb_options);
    rb_grn_scan_options(rb_options,
                        "sort_keys", &rb_sort_keys,
                        "group_keys", &rb_group_keys,
                        NULL);

    if (!NIL_P(rb_sort_keys)) {
        VALUE rb_table;

        if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_sort_keys, rb_cArray)))
            rb_raise(rb_eArgError, ":sort_keys should be an array of key: <%s>",
                     rb_grn_inspect(rb_sort_keys));

        definition.n_sort_keys = RARRAY_LEN(rb_sort_keys);
        definition.sort_keys = ALLOCA_N(grn_table_sort_key,
                                        definition.n_sort_keys);
        rb_table = GRNOBJECT2RVAL(Qnil, context, table, GRN_FALSE);
        rb_grn_table_sort_keys_fill(context,
                                    definition.sort_keys,
                                    definition.n_sort_keys,
                                    rb_sort_keys,
                                    rb_table);
    }

    if (!NIL_P(rb_group_keys)) {
        VALUE rb_table;

        if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_group_keys, rb_cArray)))
            rb_raise(rb_eArgError, ":group_keys should be an array of key: <%s>",
                     rb_grn_inspect(rb_group_keys));

        definition.n_group_keys = RARRAY_LEN(rb_group_keys);
        definition.group_keys = ALLOCA_N(grn_table_sort_key,
                                         definition.n_group_keys);
        rb_table = GRNOBJECT2RVAL(Qnil, context, table, GRN_FALSE);
        rb_grn_table_sort_keys_fill(context,
                                    definition.group_keys,
                                    definition.n_group_keys,
                                    rb_group_keys,
                                    rb_table);
    }

    rb_builder = rb_grn_record_expression_builder_new(rb_table, Qnil);
    rb_window_function_call =
        rb_grn_record_expression_builder_build(rb_builder);
    rb_grn_object_deconstruct(RB_GRN_OBJECT(RTYPEDDATA_DATA(rb_window_function_call)),
                              &window_function_call, NULL,
                              NULL, NULL, NULL, NULL);

    rc = grn_table_apply_window_function(context,
                                         table,
                                         column,
                                         &definition,
                                         window_function_call);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return self;
}

/*
 * Applies the expression to all records in the table of the
 * column. Results are stored into the column.
 *
 * @example Compute column values from other column values.
 *
 *   Groonga::Schema.define do |schema|
 *     schema.create_table("Comments") do |table|
 *       # The column that has base data to compute "plus1" column data.
 *       table.uint32("base")
 *       # The column for storing computed value with "base" column data.
 *       table.uint32("plus1")
 *     end
 *   end
 *   comments = Groonga["Comments"]
 *   plus1 = Groonga["Comments.plus1"]
 *
 *   3.times do |i|
 *     comments.add(:base => i)
 *   end
 *
 *   plus1.apply_expression do |record|
 *     # Computes "base" column value plus one.
 *     record.base + 1
 *   end
 *
 *   comments.each do |comment|
 *     p [comment.base, comment.plus1]
 *       # -> [0, 1]
 *       # -> [1, 2]
 *       # -> [2, 3]
 *   end
 *
 * @overload apply_expression {|record| }
 *
 *   @yield [record]
 *     It yields an object that builds expression. The block must
 *     build an expression to be applied to all records.
 *   @yieldparam [Groonga::RecordExpressionBuilder] record
 *     The expression builder to create an expression.
 *   @yieldreturn [Groonga::ExpressionBuilder]
 *     It must be an expression.
 *
 * @since 7.0.2
 */
static VALUE
rb_grn_data_column_apply_expression (VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *column;
    grn_obj *table;
    VALUE rb_table;
    grn_obj *expression = NULL;
    VALUE rb_builder;
    VALUE rb_expression;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, &table,
                              NULL, NULL, NULL);
    rb_table = GRNOBJECT2RVAL(Qnil, context, table, GRN_FALSE);

    rb_builder = rb_grn_record_expression_builder_new(rb_table, Qnil);
    rb_expression = rb_grn_record_expression_builder_build(rb_builder);
    rb_grn_object_deconstruct(RB_GRN_OBJECT(RTYPEDDATA_DATA(rb_expression)),
                              &expression, NULL,
                              NULL, NULL, NULL, NULL);

    rc = grn_table_apply_expr(context,
                              table,
                              column,
                              expression);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return self;
}

/*
 * @overload missing_mode
 * @return [:add, :ignore, :nil] The missing mode of the column.
 * @since 12.0.2
 */
static VALUE
rb_grn_data_column_get_missing_mode(VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, NULL,
                              NULL, NULL, NULL);

    flags = grn_column_get_flags(context, column);
    switch ((flags & GRN_OBJ_MISSING_MASK)) {
    case GRN_OBJ_MISSING_IGNORE:
        return rb_id2sym(rb_intern("ignore"));
    case GRN_OBJ_MISSING_NIL:
        return rb_id2sym(rb_intern("nil"));
    default:
        return rb_id2sym(rb_intern("add"));
    }
}

/*
 * @overload missing_add?
 * @return [Boolean] @true@ if the column uses @:add@ missing mode.
 * @since 12.0.2
 */
static VALUE
rb_grn_data_column_missing_add_p(VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, NULL,
                              NULL, NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL((flags & GRN_OBJ_MISSING_MASK) == GRN_OBJ_MISSING_ADD);
}

/*
 * @overload missing_ignore?
 * @return [Boolean] @true@ if the column uses @:ignore@ missing mode.
 * @since 12.0.2
 */
static VALUE
rb_grn_data_column_missing_ignore_p(VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, NULL,
                              NULL, NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL((flags & GRN_OBJ_MISSING_MASK) == GRN_OBJ_MISSING_IGNORE);
}

/*
 * @overload missing_nil?
 * @return [Boolean] @true@ if the column uses @:nil@ missing mode.
 * @since 12.0.2
 */
static VALUE
rb_grn_data_column_missing_nil_p(VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, NULL,
                              NULL, NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL((flags & GRN_OBJ_MISSING_MASK) == GRN_OBJ_MISSING_NIL);
}

/*
 * @overload invalid_mode
 * @return [:error, :warn, :ignore] The invalid mode of the column.
 * @since 12.0.2
 */
static VALUE
rb_grn_data_column_get_invalid_mode(VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, NULL,
                              NULL, NULL, NULL);

    flags = grn_column_get_flags(context, column);
    switch ((flags & GRN_OBJ_INVALID_MASK)) {
    case GRN_OBJ_INVALID_WARN:
        return rb_id2sym(rb_intern("warn"));
    case GRN_OBJ_INVALID_IGNORE:
        return rb_id2sym(rb_intern("ignore"));
    default:
        return rb_id2sym(rb_intern("error"));
    }
}

/*
 * @overload invalid_error?
 * @return [Boolean] @true@ if the column uses @:error@ invalid mode.
 * @since 12.0.2
 */
static VALUE
rb_grn_data_column_invalid_error_p(VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, NULL,
                              NULL, NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL((flags & GRN_OBJ_INVALID_MASK) == GRN_OBJ_INVALID_ERROR);
}

/*
 * @overload invalid_warn?
 * @return [Boolean] @true@ if the column uses @:warn@ invalid mode.
 * @since 12.0.2
 */
static VALUE
rb_grn_data_column_invalid_warn_p(VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, NULL,
                              NULL, NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL((flags & GRN_OBJ_INVALID_MASK) == GRN_OBJ_INVALID_WARN);
}

/*
 * @overload invalid_ignore?
 * @return [Boolean] @true@ if the column uses @:ignore@ invalid mode.
 * @since 12.0.2
 */
static VALUE
rb_grn_data_column_invalid_ignore_p(VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, NULL,
                              NULL, NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL((flags & GRN_OBJ_INVALID_MASK) == GRN_OBJ_INVALID_IGNORE);
}

void
rb_grn_init_data_column (VALUE mGrn)
{
    rb_cGrnDataColumn = rb_define_class_under(mGrn, "DataColumn", rb_cGrnColumn);

    rb_define_method(rb_cGrnDataColumn, "apply_window_function",
                     rb_grn_data_column_apply_window_function, -1);
    rb_define_method(rb_cGrnDataColumn, "apply_expression",
                     rb_grn_data_column_apply_expression, 0);

    rb_define_method(rb_cGrnDataColumn, "missing_mode",
                     rb_grn_data_column_get_missing_mode, 0);
    rb_define_method(rb_cGrnDataColumn, "missing_add?",
                     rb_grn_data_column_missing_add_p, 0);
    rb_define_method(rb_cGrnDataColumn, "missing_ignore?",
                     rb_grn_data_column_missing_ignore_p, 0);
    rb_define_method(rb_cGrnDataColumn, "missing_nil?",
                     rb_grn_data_column_missing_nil_p, 0);

    rb_define_method(rb_cGrnDataColumn, "invalid_mode",
                     rb_grn_data_column_get_invalid_mode, 0);
    rb_define_method(rb_cGrnDataColumn, "invalid_error?",
                     rb_grn_data_column_invalid_error_p, 0);
    rb_define_method(rb_cGrnDataColumn, "invalid_warn?",
                     rb_grn_data_column_invalid_warn_p, 0);
    rb_define_method(rb_cGrnDataColumn, "invalid_ignore?",
                     rb_grn_data_column_invalid_ignore_p, 0);

    rb_grn_init_fix_size_column(mGrn);
    rb_grn_init_variable_size_column(mGrn);
}
