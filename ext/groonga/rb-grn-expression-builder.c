/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-grn.h"

VALUE rb_cGrnRecordExpressionBuilder;
VALUE rb_cGrnColumnExpressionBuilder;

VALUE
rb_grn_record_expression_builder_new (VALUE table, VALUE name)
{
    return rb_funcall(rb_cGrnRecordExpressionBuilder,
		      rb_intern("new"), 2, table, name);
}

VALUE
rb_grn_column_expression_builder_new (VALUE column, VALUE name, VALUE query)
{
    return rb_funcall(rb_cGrnColumnExpressionBuilder,
		      rb_intern("new"), 3, column, name, query);
}

static VALUE
build (VALUE self)
{
    return rb_funcall(self, rb_intern("build"), 0);
}

static VALUE
build_block (VALUE self)
{
    return rb_funcall(rb_block_proc(), rb_intern("call"), 1, self);
}

VALUE
rb_grn_record_expression_builder_build (VALUE self)
{
    if (rb_block_given_p())
	return rb_iterate(build, self, build_block, self);
    else
	return build(self);
}

VALUE
rb_grn_column_expression_builder_build (VALUE self)
{
    if (rb_block_given_p())
	return rb_iterate(build, self, build_block, self);
    else
	return build(self);
}

void
rb_grn_init_expression_builder (VALUE mGrn)
{
    rb_cGrnRecordExpressionBuilder =
        rb_const_get(mGrn, rb_intern("RecordExpressionBuilder"));
    rb_cGrnColumnExpressionBuilder =
        rb_const_get(mGrn, rb_intern("ColumnExpressionBuilder"));
}
