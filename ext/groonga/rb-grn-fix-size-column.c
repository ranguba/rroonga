/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2021  Sutou Kouhei <kou@clear-code.com>
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

#define SELF(object) ((RbGrnColumn *)RTYPEDDATA_DATA(object))

VALUE rb_cGrnFixSizeColumn;

/*
 * Document-class: Groonga::FixSizeColumn < Groonga::Column
 *
 * 固定長データ用のカラム。
 */

/*
 * _column_ の _id_ に対応する値を返す。
 *
 * @overload column[id]
 *   @return [値]
 */
VALUE
rb_grn_fix_size_column_array_reference (VALUE self, VALUE rb_id)
{
    grn_id id;
    grn_ctx *context;
    grn_obj *fix_size_column;
    grn_obj *range;
    grn_obj *value;

    rb_grn_column_deconstruct(SELF(self), &fix_size_column, &context,
                              NULL, NULL,
                              &value, NULL, &range);

    id = NUM2UINT(rb_id);
    GRN_BULK_REWIND(value);
    grn_obj_get_value(context, fix_size_column, id, value);
    rb_grn_context_check(context, self);

    return GRNVALUE2RVAL(context, value, range, self);
}

/*
 * _column_ の _id_ に対応する値を設定する。
 *
 * @overload []=(id, value)
 *   @param [Integer] id 設定する値に対応する _column_ の _id_
 *   @param [Groonga::Object] value 設定する値
 */
static VALUE
rb_grn_fix_size_column_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_id domain_id, range_id;
    grn_obj *domain, *range;
    grn_obj *value;
    grn_rc rc;
    grn_id id;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              &domain_id, &domain,
                              &value, &range_id, &range);

    id = NUM2UINT(rb_id);
    RVAL2GRNVALUE(rb_value, context, value, range_id, range);

    rc = grn_obj_set_value(context, column, id, value, GRN_OBJ_SET);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

static VALUE
rb_grn_fix_size_column_integer_set (int argc, VALUE *argv, VALUE self, int flags)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj *value;
    grn_rc rc;
    grn_id id;
    VALUE rb_id, rb_delta;

    rb_scan_args(argc, argv, "11", &rb_id, &rb_delta);

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, NULL,
                              &value, NULL, NULL);

    id = NUM2UINT(rb_id);
    if (NIL_P(rb_delta))
        rb_delta = INT2NUM(1);

    GRN_BULK_REWIND(value);
    RVAL2GRNBULK(rb_delta, context, value);

    rc = grn_obj_set_value(context, column, id, value, flags);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * _column_ の _id_ に対応する値を _delta_ だけ増加する。 _delta_
 * が +nil+ の場合は1増加する。
 *
 * @overload increment!(id, delta=nil)
 */
static VALUE
rb_grn_fix_size_column_increment (int argc, VALUE *argv, VALUE self)
{
    return rb_grn_fix_size_column_integer_set(argc, argv, self, GRN_OBJ_INCR);
}

/*
 * _column_ の _id_ に対応する値を _delta_ だけ減少する。 _delta_
 * が +nil+ の場合は1減少する。
 *
 * @overload decrement!(id, delta=nil)
 */
static VALUE
rb_grn_fix_size_column_decrement (int argc, VALUE *argv, VALUE self)
{
    return rb_grn_fix_size_column_integer_set(argc, argv, self, GRN_OBJ_DECR);
}

/*
 * Recreates all index columns for the column.
 *
 * This method is useful when you have any broken index columns for
 * the column. You don't need to specify each index column. But this
 * method spends more time rather than you specify only reindex
 * needed index columns.
 *
 * You can use {Groonga::Database#reindex} to recreate all index
 * columns in a database.
 *
 * You can use {Groonga::TableKeySupport#reindex} to recreate all
 * index columns in a table.
 *
 * You can use {Groonga::IndexColumn#reindex} to specify the reindex
 * target index column.
 *
 * @example How to recreate all index columns for the column
 *   Groonga::Schema.define do |schema|
 *     schema.create_table("Users") do |table|
 *       table.integer32("age")
 *       table.integer32("score")
 *     end
 *
 *     schema.create_table("Numbers",
 *                         :type => :patricia_trie,
 *                         :key_type => :integer32) do |table|
 *       table.index("Users.age")
 *       table.index("Users.score")
 *     end
 *
 *     schema.create_table("Ages",
 *                         :type => :patricia_trie,
 *                         :key_type => :integer32) do |table|
 *       table.index("Users.age")
 *     end
 *
 *     schema.create_table("Scores",
 *                         :type => :patricia_trie,
 *                         :key_type => :integer32) do |table|
 *       table.index("Users.score")
 *     end
 *   end
 *
 *   Groonga["Users.age"].reindex
 *   # They are called:
 *   #   Groonga["Numbers.Users_age"].reindex
 *   #   Groonga["Ages.Users_age"].reindex
 *   #
 *   # They aren't called:
 *   #   Groonga["Numbers.Users_score"].reindex
 *   #   Groonga["Scores.Users_score"].reindex
 *
 * @overload reindex
 *   @return [void]
 *
 * @see Groonga::Database#reindex
 * @see Groonga::TableKeySupport#reindex
 * @see Groonga::VariableSizeColumn#reindex
 * @see Groonga::IndexColumn#reindex
 *
 * @since 5.1.1
 */
static VALUE
rb_grn_fix_size_column_reindex (VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *column;

    rb_grn_column_deconstruct(SELF(self), &column, &context,
                              NULL, NULL,
                              NULL, NULL, NULL);

    rc = grn_obj_reindex(context, column);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

void
rb_grn_init_fix_size_column (VALUE mGrn)
{
    rb_cGrnFixSizeColumn =
        rb_define_class_under(mGrn, "FixSizeColumn", rb_cGrnDataColumn);

    rb_define_method(rb_cGrnFixSizeColumn, "[]",
                     rb_grn_fix_size_column_array_reference, 1);
    rb_define_method(rb_cGrnFixSizeColumn, "[]=",
                     rb_grn_fix_size_column_array_set, 2);

    rb_define_method(rb_cGrnFixSizeColumn, "increment!",
                     rb_grn_fix_size_column_increment, -1);
    rb_define_method(rb_cGrnFixSizeColumn, "decrement!",
                     rb_grn_fix_size_column_decrement, -1);

    rb_define_method(rb_cGrnFixSizeColumn, "reindex",
                     rb_grn_fix_size_column_reindex, 0);
}
