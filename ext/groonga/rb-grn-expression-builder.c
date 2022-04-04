/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2022  Sutou Kouhei <kou@clear-code.com>

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

VALUE rb_cGrnRecordExpressionBuilder;
VALUE rb_cGrnColumnExpressionBuilder;

static ID id_build;
static ID id_call;
static ID id_new;

VALUE
rb_grn_record_expression_builder_new (VALUE table, VALUE name)
{
    return rb_funcall(rb_cGrnRecordExpressionBuilder,
                      id_new, 2, table, name);
}

VALUE
rb_grn_column_expression_builder_new (VALUE column, VALUE name, VALUE query)
{
    return rb_funcall(rb_cGrnColumnExpressionBuilder,
                      id_new, 3, column, name, query);
}

static VALUE
build_block (RB_BLOCK_CALL_FUNC_ARGLIST(yielded_arg, callback_arg))
{
    return rb_funcall(rb_block_proc(), id_call, 1, yielded_arg);
}

VALUE
rb_grn_record_expression_builder_build (VALUE self)
{
    if (rb_block_given_p())
        return rb_block_call(self, id_build, 0, NULL, build_block, self);
    else
        return rb_funcall(self, id_build, 0);
}

VALUE
rb_grn_column_expression_builder_build (VALUE self)
{
    if (rb_block_given_p())
        return rb_block_call(self, id_build, 0, NULL, build_block, self);
    else
        return rb_funcall(self, id_build, 0);
}

void
rb_grn_init_expression_builder (VALUE mGrn)
{
    id_build = rb_intern("build");
    id_call = rb_intern("call");
    id_new = rb_intern("new");

    rb_cGrnRecordExpressionBuilder =
        rb_const_get(mGrn, rb_intern("RecordExpressionBuilder"));
    rb_cGrnColumnExpressionBuilder =
        rb_const_get(mGrn, rb_intern("ColumnExpressionBuilder"));
}
