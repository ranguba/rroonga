/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009-2012  Kouhei Sutou <kou@clear-code.com>

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

VALUE rb_mGrnOperator;

grn_operator
rb_grn_operator_from_ruby_object (VALUE rb_operator)
{
    grn_operator operator = GRN_OP_OR;

    if (NIL_P(rb_operator) ||
        rb_grn_equal_option(rb_operator, "or") ||
        rb_grn_equal_option(rb_operator, "||")) {
        operator = GRN_OP_OR;
    } else if (rb_grn_equal_option(rb_operator, "and") ||
               rb_grn_equal_option(rb_operator, "+") ||
               rb_grn_equal_option(rb_operator, "&&")) {
        operator = GRN_OP_AND;
    } else if (rb_grn_equal_option(rb_operator, "but") ||
               rb_grn_equal_option(rb_operator, "not") ||
               rb_grn_equal_option(rb_operator, "-")) {
        operator = GRN_OP_BUT;
    } else if (rb_grn_equal_option(rb_operator, "adjust") ||
               rb_grn_equal_option(rb_operator, ">")) {
        operator = GRN_OP_ADJUST;
    } else {
        rb_raise(rb_eArgError,
                 "operator should be one of "
                 "[:or, :||, :and, :+, :&&, :but, :not, :-, :adjust, :>]: <%s>",
                 rb_grn_inspect(rb_operator));
    }

    return operator;
}

void
rb_grn_init_operator (VALUE mGrn)
{
    rb_mGrnOperator = rb_define_module_under(mGrn, "Operator");
    /* deprecated: backward compatibility. */
    rb_define_const(mGrn, "Operation", rb_mGrnOperator);

    rb_define_const(rb_mGrnOperator, "PUSH",
                    UINT2NUM(GRN_OP_PUSH));
    rb_define_const(rb_mGrnOperator, "POP",
                    UINT2NUM(GRN_OP_POP));
    rb_define_const(rb_mGrnOperator, "NO_OPERATION",
                    UINT2NUM(GRN_OP_NOP));
    rb_define_const(rb_mGrnOperator, "CALL",
                    UINT2NUM(GRN_OP_CALL));
    rb_define_const(rb_mGrnOperator, "INTERN",
                    UINT2NUM(GRN_OP_INTERN));
    rb_define_const(rb_mGrnOperator, "GET_REFERENCE",
                    UINT2NUM(GRN_OP_GET_REF));
    rb_define_const(rb_mGrnOperator, "GET_VALUE",
                    UINT2NUM(GRN_OP_GET_VALUE));
    rb_define_const(rb_mGrnOperator, "AND",
                    UINT2NUM(GRN_OP_AND));
    rb_define_const(rb_mGrnOperator, "BUT",
                    UINT2NUM(GRN_OP_BUT));
    rb_define_const(rb_mGrnOperator, "OR",
                    UINT2NUM(GRN_OP_OR));
    rb_define_const(rb_mGrnOperator, "ASSIGN",
                    UINT2NUM(GRN_OP_ASSIGN));
    rb_define_const(rb_mGrnOperator, "STAR_ASSIGN",
                    UINT2NUM(GRN_OP_STAR_ASSIGN));
    rb_define_const(rb_mGrnOperator, "SLASH_ASSIGN",
                    UINT2NUM(GRN_OP_SLASH_ASSIGN));
    rb_define_const(rb_mGrnOperator, "MODULO_ASSIGN",
                    UINT2NUM(GRN_OP_MOD_ASSIGN));
    rb_define_const(rb_mGrnOperator, "PLUS_ASSIGN",
                    UINT2NUM(GRN_OP_PLUS_ASSIGN));
    rb_define_const(rb_mGrnOperator, "MINUS_ASSIGN",
                    UINT2NUM(GRN_OP_MINUS_ASSIGN));
    rb_define_const(rb_mGrnOperator, "SHIFTL_ASSIGN",
                    UINT2NUM(GRN_OP_SHIFTL_ASSIGN));
    rb_define_const(rb_mGrnOperator, "SHIRTR_ASSIGN",
                    UINT2NUM(GRN_OP_SHIFTR_ASSIGN));
    rb_define_const(rb_mGrnOperator, "SHIFTRR_ASSIGN",
                    UINT2NUM(GRN_OP_SHIFTRR_ASSIGN));
    rb_define_const(rb_mGrnOperator, "AND_ASSIGN",
                    UINT2NUM(GRN_OP_AND_ASSIGN));
    rb_define_const(rb_mGrnOperator, "XOR_ASSIGN",
                    UINT2NUM(GRN_OP_XOR_ASSIGN));
    rb_define_const(rb_mGrnOperator, "OR_ASSIGN",
                    UINT2NUM(GRN_OP_OR_ASSIGN));
    rb_define_const(rb_mGrnOperator, "JUMP",
                    UINT2NUM(GRN_OP_JUMP));
    rb_define_const(rb_mGrnOperator, "CJUMP",
                    UINT2NUM(GRN_OP_CJUMP));
    rb_define_const(rb_mGrnOperator, "COMMA",
                    UINT2NUM(GRN_OP_COMMA));
    rb_define_const(rb_mGrnOperator, "BITWISE_OR",
                    UINT2NUM(GRN_OP_BITWISE_OR));
    rb_define_const(rb_mGrnOperator, "BITWISE_XOR",
                    UINT2NUM(GRN_OP_BITWISE_XOR));
    rb_define_const(rb_mGrnOperator, "BITWISE_AND",
                    UINT2NUM(GRN_OP_BITWISE_AND));
    rb_define_const(rb_mGrnOperator, "BITWISE_NOT",
                    UINT2NUM(GRN_OP_BITWISE_NOT));
    rb_define_const(rb_mGrnOperator, "EQUAL",
                    UINT2NUM(GRN_OP_EQUAL));
    rb_define_const(rb_mGrnOperator, "NOT_EQUAL",
                    UINT2NUM(GRN_OP_NOT_EQUAL));
    rb_define_const(rb_mGrnOperator, "LESS",
                    UINT2NUM(GRN_OP_LESS));
    rb_define_const(rb_mGrnOperator, "GREATER",
                    UINT2NUM(GRN_OP_GREATER));
    rb_define_const(rb_mGrnOperator, "LESS_EQUAL",
                    UINT2NUM(GRN_OP_LESS_EQUAL));
    rb_define_const(rb_mGrnOperator, "GREATER_EQUAL",
                    UINT2NUM(GRN_OP_GREATER_EQUAL));
    rb_define_const(rb_mGrnOperator, "IN",
                    UINT2NUM(GRN_OP_IN));
    rb_define_const(rb_mGrnOperator, "MATCH",
                    UINT2NUM(GRN_OP_MATCH));
    rb_define_const(rb_mGrnOperator, "NEAR",
                    UINT2NUM(GRN_OP_NEAR));
    rb_define_const(rb_mGrnOperator, "NEAR2",
                    UINT2NUM(GRN_OP_NEAR2));
    rb_define_const(rb_mGrnOperator, "SIMILAR",
                    UINT2NUM(GRN_OP_SIMILAR));
    rb_define_const(rb_mGrnOperator, "TERM_EXTRACT",
                    UINT2NUM(GRN_OP_TERM_EXTRACT));
    rb_define_const(rb_mGrnOperator, "SHIFTL",
                    UINT2NUM(GRN_OP_SHIFTL));
    rb_define_const(rb_mGrnOperator, "SHIFTR",
                    UINT2NUM(GRN_OP_SHIFTR));
    rb_define_const(rb_mGrnOperator, "SHIFTRR",
                    UINT2NUM(GRN_OP_SHIFTRR));
    rb_define_const(rb_mGrnOperator, "PLUS",
                    UINT2NUM(GRN_OP_PLUS));
    rb_define_const(rb_mGrnOperator, "MINUS",
                    UINT2NUM(GRN_OP_MINUS));
    rb_define_const(rb_mGrnOperator, "STAR",
                    UINT2NUM(GRN_OP_STAR));
    rb_define_const(rb_mGrnOperator, "SLASH",
                    UINT2NUM(GRN_OP_SLASH));
    rb_define_const(rb_mGrnOperator, "MODULO",
                    UINT2NUM(GRN_OP_MOD));
    rb_define_const(rb_mGrnOperator, "DELETE",
                    UINT2NUM(GRN_OP_DELETE));
    rb_define_const(rb_mGrnOperator, "INCREMENT",
                    UINT2NUM(GRN_OP_INCR));
    rb_define_const(rb_mGrnOperator, "DECREMENT",
                    UINT2NUM(GRN_OP_DECR));
    rb_define_const(rb_mGrnOperator, "INCREMENT_POST",
                    UINT2NUM(GRN_OP_INCR_POST));
    rb_define_const(rb_mGrnOperator, "DECREMENT_POST",
                    UINT2NUM(GRN_OP_DECR_POST));
    rb_define_const(rb_mGrnOperator, "NOT",
                    UINT2NUM(GRN_OP_NOT));
    rb_define_const(rb_mGrnOperator, "ADJUST",
                    UINT2NUM(GRN_OP_ADJUST));
    rb_define_const(rb_mGrnOperator, "EXACT",
                    UINT2NUM(GRN_OP_EXACT));
    rb_define_const(rb_mGrnOperator, "LONGEST_COMMON_PREFIX",
                    UINT2NUM(GRN_OP_LCP));
    rb_define_const(rb_mGrnOperator, "PARTIAL",
                    UINT2NUM(GRN_OP_PARTIAL));
    rb_define_const(rb_mGrnOperator, "UNSPLIT",
                    UINT2NUM(GRN_OP_UNSPLIT));
    rb_define_const(rb_mGrnOperator, "PREFIX",
                    UINT2NUM(GRN_OP_PREFIX));
    rb_define_const(rb_mGrnOperator, "SUFFIX",
                    UINT2NUM(GRN_OP_SUFFIX));
    rb_define_const(rb_mGrnOperator, "GEO_DISTANCE1",
                    UINT2NUM(GRN_OP_GEO_DISTANCE1));
    rb_define_const(rb_mGrnOperator, "GEO_DISTANCE2",
                    UINT2NUM(GRN_OP_GEO_DISTANCE2));
    rb_define_const(rb_mGrnOperator, "GEO_DISTANCE3",
                    UINT2NUM(GRN_OP_GEO_DISTANCE3));
    rb_define_const(rb_mGrnOperator, "GEO_DISTANCE4",
                    UINT2NUM(GRN_OP_GEO_DISTANCE4));
    rb_define_const(rb_mGrnOperator, "GEO_WITHINP5",
                    UINT2NUM(GRN_OP_GEO_WITHINP5));
    rb_define_const(rb_mGrnOperator, "GEO_WITHINP6",
                    UINT2NUM(GRN_OP_GEO_WITHINP6));
    rb_define_const(rb_mGrnOperator, "GEO_WITHINP8",
                    UINT2NUM(GRN_OP_GEO_WITHINP8));
    rb_define_const(rb_mGrnOperator, "OBJECT_SEARCH",
                    UINT2NUM(GRN_OP_OBJ_SEARCH));
    rb_define_const(rb_mGrnOperator, "EXPRESSION_GET_VARIABLE",
                    UINT2NUM(GRN_OP_EXPR_GET_VAR));
    rb_define_const(rb_mGrnOperator, "TABLE_CREATE",
                    UINT2NUM(GRN_OP_TABLE_CREATE));
    rb_define_const(rb_mGrnOperator, "TABLE_SELECT",
                    UINT2NUM(GRN_OP_TABLE_SELECT));
    rb_define_const(rb_mGrnOperator, "TABLE_SORT",
                    UINT2NUM(GRN_OP_TABLE_SORT));
    rb_define_const(rb_mGrnOperator, "TABLE_GROUP",
                    UINT2NUM(GRN_OP_TABLE_GROUP));
    rb_define_const(rb_mGrnOperator, "JSON_PUT",
                    UINT2NUM(GRN_OP_JSON_PUT));


/*
    rb_define_const(rb_mGrnOperator, "GEO_DISTANCE1",
                    UINT2NUM(GRN_OP_GEO_DISTANCE1));
    rb_define_const(rb_mGrnOperator, "GEO_DISTANCE2",
                    UINT2NUM(GRN_OP_GEO_DISTANCE2));
    rb_define_const(rb_mGrnOperator, "GEO_DISTANCE3",
                    UINT2NUM(GRN_OP_GEO_DISTANCE3));
    rb_define_const(rb_mGrnOperator, "GEO_DISTANCE4",
                    UINT2NUM(GRN_OP_GEO_DISTANCE4));
    rb_define_const(rb_mGrnOperator, "GEO_WITHINP5",
                    UINT2NUM(GRN_OP_GEO_WITHINP5));
    rb_define_const(rb_mGrnOperator, "GEO_WITHINP6",
                    UINT2NUM(GRN_OP_GEO_WITHINP6));
    rb_define_const(rb_mGrnOperator, "GEO_WITHINP8",
                    UINT2NUM(GRN_OP_GEO_WITHINP8));
*/
}
