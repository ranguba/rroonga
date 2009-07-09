/* -*- c-file-style: "ruby" -*- */
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

VALUE rb_mGrnOperation;

void
rb_grn_init_operation (VALUE mGrn)
{
    rb_mGrnOperation = rb_define_module_under(mGrn, "Operation");

    rb_define_const(rb_mGrnOperation, "NO_OPERATION",
                    UINT2NUM(GRN_OP_NOP));
    rb_define_const(rb_mGrnOperation, "PUSH",
                    UINT2NUM(GRN_OP_PUSH));
    rb_define_const(rb_mGrnOperation, "POP",
                    UINT2NUM(GRN_OP_POP));
    rb_define_const(rb_mGrnOperation, "CALL",
                    UINT2NUM(GRN_OP_CALL));
    rb_define_const(rb_mGrnOperation, "INTERN",
                    UINT2NUM(GRN_OP_INTERN));
    rb_define_const(rb_mGrnOperation, "TABLE_CREATE",
                    UINT2NUM(GRN_OP_TABLE_CREATE));
    rb_define_const(rb_mGrnOperation, "EXPRESSION_GET_VARIABLE",
                    UINT2NUM(GRN_OP_EXPR_GET_VAR));
    rb_define_const(rb_mGrnOperation, "VARIABLE_SET_VALUE",
                    UINT2NUM(GRN_OP_VAR_SET_VALUE));
    rb_define_const(rb_mGrnOperation, "OBJECT_GET_VALUE",
                    UINT2NUM(GRN_OP_OBJ_GET_VALUE));
    rb_define_const(rb_mGrnOperation, "OBJECT_SET_VALUE",
                    UINT2NUM(GRN_OP_OBJ_SET_VALUE));
    rb_define_const(rb_mGrnOperation, "OBJECT_SEARCH",
                    UINT2NUM(GRN_OP_OBJ_SEARCH));
    rb_define_const(rb_mGrnOperation, "TABLE_SELECT",
                    UINT2NUM(GRN_OP_TABLE_SELECT));
    rb_define_const(rb_mGrnOperation, "TABLE_SORT",
                    UINT2NUM(GRN_OP_TABLE_SORT));
    rb_define_const(rb_mGrnOperation, "TABLE_GROUP",
                    UINT2NUM(GRN_OP_TABLE_GROUP));
    rb_define_const(rb_mGrnOperation, "JSON_PUT",
                    UINT2NUM(GRN_OP_JSON_PUT));
    rb_define_const(rb_mGrnOperation, "AND",
                    UINT2NUM(GRN_OP_AND));
    rb_define_const(rb_mGrnOperation, "OR",
                    UINT2NUM(GRN_OP_OR));
    rb_define_const(rb_mGrnOperation, "EQUAL",
                    UINT2NUM(GRN_OP_EQUAL));
    rb_define_const(rb_mGrnOperation, "NOT_EQUAL",
                    UINT2NUM(GRN_OP_NOT_EQUAL));
    rb_define_const(rb_mGrnOperation, "LESS",
                    UINT2NUM(GRN_OP_LESS));
    rb_define_const(rb_mGrnOperation, "GREATER",
                    UINT2NUM(GRN_OP_GREATER));
    rb_define_const(rb_mGrnOperation, "LESS_EQUAL",
                    UINT2NUM(GRN_OP_LESS_EQUAL));
    rb_define_const(rb_mGrnOperation, "GREATER_EQUAL",
                    UINT2NUM(GRN_OP_GREATER_EQUAL));
    rb_define_const(rb_mGrnOperation, "MATCH",
                    UINT2NUM(GRN_OP_MATCH));
/*
    rb_define_const(rb_mGrnOperation, "GEO_DISTANCE1",
                    UINT2NUM(GRN_OP_GEO_DISTANCE1));
    rb_define_const(rb_mGrnOperation, "GEO_DISTANCE2",
                    UINT2NUM(GRN_OP_GEO_DISTANCE2));
    rb_define_const(rb_mGrnOperation, "GEO_DISTANCE3",
                    UINT2NUM(GRN_OP_GEO_DISTANCE3));
    rb_define_const(rb_mGrnOperation, "GEO_DISTANCE4",
                    UINT2NUM(GRN_OP_GEO_DISTANCE4));
    rb_define_const(rb_mGrnOperation, "GEO_WITHINP5",
                    UINT2NUM(GRN_OP_GEO_WITHINP5));
    rb_define_const(rb_mGrnOperation, "GEO_WITHINP6",
                    UINT2NUM(GRN_OP_GEO_WITHINP6));
    rb_define_const(rb_mGrnOperation, "GEO_WITHINP8",
                    UINT2NUM(GRN_OP_GEO_WITHINP8));
*/
}
