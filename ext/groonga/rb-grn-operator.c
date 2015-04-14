/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2015  Kouhei Sutou <kou@clear-code.com>

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

VALUE rb_cGrnOperator;

grn_operator
rb_grn_operator_from_ruby_object (VALUE rb_operator)
{
    grn_operator operator;

    if (NIL_P(rb_operator)) {
        operator = GRN_OP_OR;
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(rb_operator, rb_cGrnOperator))) {
        operator = NUM2UINT(rb_iv_get(rb_operator, "@value"));
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(rb_operator, rb_cInteger))) {
        operator = NUM2UINT(rb_operator);
    } else if (rb_grn_equal_option(rb_operator, "push")) {
        operator = GRN_OP_PUSH;
    } else if (rb_grn_equal_option(rb_operator, "pop")) {
        operator = GRN_OP_POP;
    } else if (rb_grn_equal_option(rb_operator, "no-operation") ||
               rb_grn_equal_option(rb_operator, "no_operation")) {
        operator = GRN_OP_NOP;
    } else if (rb_grn_equal_option(rb_operator, "call")) {
        operator = GRN_OP_CALL;
    } else if (rb_grn_equal_option(rb_operator, "intern")) {
        operator = GRN_OP_INTERN;
    } else if (rb_grn_equal_option(rb_operator, "get-reference") ||
               rb_grn_equal_option(rb_operator, "get_reference")) {
        operator = GRN_OP_GET_REF;
    } else if (rb_grn_equal_option(rb_operator, "get-value") ||
               rb_grn_equal_option(rb_operator, "get_value")) {
        operator = GRN_OP_GET_VALUE;
    } else if (rb_grn_equal_option(rb_operator, "and")) {
        operator = GRN_OP_AND;
    } else if (rb_grn_equal_option(rb_operator, "but") ||
               rb_grn_equal_option(rb_operator, "not") ||
               rb_grn_equal_option(rb_operator, "and-not") ||
               rb_grn_equal_option(rb_operator, "and_not")) {
        operator = GRN_OP_AND_NOT;
    } else if (rb_grn_equal_option(rb_operator, "or")) {
        operator = GRN_OP_OR;
    } else if (rb_grn_equal_option(rb_operator, "assign")) {
        operator = GRN_OP_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "star-assign") ||
               rb_grn_equal_option(rb_operator, "star_assign")) {
        operator = GRN_OP_STAR_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "slash-assign") ||
               rb_grn_equal_option(rb_operator, "slash_assign")) {
        operator = GRN_OP_SLASH_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "modulo-assign") ||
               rb_grn_equal_option(rb_operator, "modulo_assign")) {
        operator = GRN_OP_MOD_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "plus-assign") ||
               rb_grn_equal_option(rb_operator, "plus_assign")) {
        operator = GRN_OP_PLUS_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "minus-assign") ||
               rb_grn_equal_option(rb_operator, "minus_assign")) {
        operator = GRN_OP_MINUS_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "shiftl-assign") ||
               rb_grn_equal_option(rb_operator, "shiftl_assign")) {
        operator = GRN_OP_SHIFTL_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "shiftr-assign") ||
               rb_grn_equal_option(rb_operator, "shiftr_assign")) {
        operator = GRN_OP_SHIFTR_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "shiftrr-assign") ||
               rb_grn_equal_option(rb_operator, "shiftrr_assign")) {
        operator = GRN_OP_SHIFTRR_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "and-assign") ||
               rb_grn_equal_option(rb_operator, "and_assign")) {
        operator = GRN_OP_AND_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "xor-assign") ||
               rb_grn_equal_option(rb_operator, "xor_assign")) {
        operator = GRN_OP_XOR_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "or-assign") ||
               rb_grn_equal_option(rb_operator, "or_assign")) {
        operator = GRN_OP_OR_ASSIGN;
    } else if (rb_grn_equal_option(rb_operator, "jump")) {
        operator = GRN_OP_JUMP;
    } else if (rb_grn_equal_option(rb_operator, "cjump")) {
        operator = GRN_OP_CJUMP;
    } else if (rb_grn_equal_option(rb_operator, "comma")) {
        operator = GRN_OP_COMMA;
    } else if (rb_grn_equal_option(rb_operator, "bitwise-or") ||
               rb_grn_equal_option(rb_operator, "bitwise_or")) {
        operator = GRN_OP_BITWISE_OR;
    } else if (rb_grn_equal_option(rb_operator, "bitwise-xor") ||
               rb_grn_equal_option(rb_operator, "bitwise_xor")) {
        operator = GRN_OP_BITWISE_XOR;
    } else if (rb_grn_equal_option(rb_operator, "bitwise-and") ||
               rb_grn_equal_option(rb_operator, "bitwise_and")) {
        operator = GRN_OP_BITWISE_AND;
    } else if (rb_grn_equal_option(rb_operator, "bitwise-not") ||
               rb_grn_equal_option(rb_operator, "bitwise_not")) {
        operator = GRN_OP_BITWISE_NOT;
    } else if (rb_grn_equal_option(rb_operator, "equal")) {
        operator = GRN_OP_EQUAL;
    } else if (rb_grn_equal_option(rb_operator, "not-equal") ||
               rb_grn_equal_option(rb_operator, "not_equal")) {
        operator = GRN_OP_NOT_EQUAL;
    } else if (rb_grn_equal_option(rb_operator, "less")) {
        operator = GRN_OP_LESS;
    } else if (rb_grn_equal_option(rb_operator, "greater")) {
        operator = GRN_OP_GREATER;
    } else if (rb_grn_equal_option(rb_operator, "less-equal") ||
               rb_grn_equal_option(rb_operator, "less_equal")) {
        operator = GRN_OP_LESS_EQUAL;
    } else if (rb_grn_equal_option(rb_operator, "greater-equal") ||
               rb_grn_equal_option(rb_operator, "greater_equal")) {
        operator = GRN_OP_GREATER_EQUAL;
    } else if (rb_grn_equal_option(rb_operator, "in")) {
        operator = GRN_OP_IN;
    } else if (rb_grn_equal_option(rb_operator, "match")) {
        operator = GRN_OP_MATCH;
    } else if (rb_grn_equal_option(rb_operator, "near")) {
        operator = GRN_OP_NEAR;
    } else if (rb_grn_equal_option(rb_operator, "near2")) {
        operator = GRN_OP_NEAR2;
    } else if (rb_grn_equal_option(rb_operator, "term-extract") ||
               rb_grn_equal_option(rb_operator, "term_extract")) {
        operator = GRN_OP_TERM_EXTRACT;
    } else if (rb_grn_equal_option(rb_operator, "shiftl")) {
        operator = GRN_OP_SHIFTL;
    } else if (rb_grn_equal_option(rb_operator, "shiftr")) {
        operator = GRN_OP_SHIFTR;
    } else if (rb_grn_equal_option(rb_operator, "shiftrr")) {
        operator = GRN_OP_SHIFTRR;
    } else if (rb_grn_equal_option(rb_operator, "plus")) {
        operator = GRN_OP_PLUS;
    } else if (rb_grn_equal_option(rb_operator, "minus")) {
        operator = GRN_OP_MINUS;
    } else if (rb_grn_equal_option(rb_operator, "star")) {
        operator = GRN_OP_STAR;
    } else if (rb_grn_equal_option(rb_operator, "modulo")) {
        operator = GRN_OP_MOD;
    } else if (rb_grn_equal_option(rb_operator, "delete")) {
        operator = GRN_OP_DELETE;
    } else if (rb_grn_equal_option(rb_operator, "increment")) {
        operator = GRN_OP_INCR;
    } else if (rb_grn_equal_option(rb_operator, "decrement")) {
        operator = GRN_OP_DECR;
    } else if (rb_grn_equal_option(rb_operator, "increment-post") ||
               rb_grn_equal_option(rb_operator, "increment_post")) {
        operator = GRN_OP_INCR_POST;
    } else if (rb_grn_equal_option(rb_operator, "decrement-post") ||
               rb_grn_equal_option(rb_operator, "decrement_post")) {
        operator = GRN_OP_DECR_POST;
    } else if (rb_grn_equal_option(rb_operator, "not")) {
        operator = GRN_OP_NOT;
    } else if (rb_grn_equal_option(rb_operator, "adjust")) {
        operator = GRN_OP_ADJUST;
    } else if (rb_grn_equal_option(rb_operator, "exact")) {
        operator = GRN_OP_EXACT;
    } else if (rb_grn_equal_option(rb_operator, "lcp") ||
               rb_grn_equal_option(rb_operator, "longest-common-prefix") ||
               rb_grn_equal_option(rb_operator, "longest_common_prefix")) {
        operator = GRN_OP_LCP;
    } else if (rb_grn_equal_option(rb_operator, "partial")) {
        operator = GRN_OP_PARTIAL;
    } else if (rb_grn_equal_option(rb_operator, "unsplit")) {
        operator = GRN_OP_UNSPLIT;
    } else if (rb_grn_equal_option(rb_operator, "prefix")) {
        operator = GRN_OP_PREFIX;
    } else if (rb_grn_equal_option(rb_operator, "suffix")) {
        operator = GRN_OP_SUFFIX;
    } else if (rb_grn_equal_option(rb_operator, "geo-distance1") ||
               rb_grn_equal_option(rb_operator, "geo_distance1")) {
        operator = GRN_OP_GEO_DISTANCE1;
    } else if (rb_grn_equal_option(rb_operator, "geo-distance2") ||
               rb_grn_equal_option(rb_operator, "geo_distance2")) {
        operator = GRN_OP_GEO_DISTANCE2;
    } else if (rb_grn_equal_option(rb_operator, "geo-distance3") ||
               rb_grn_equal_option(rb_operator, "geo_distance3")) {
        operator = GRN_OP_GEO_DISTANCE3;
    } else if (rb_grn_equal_option(rb_operator, "geo-distance4") ||
               rb_grn_equal_option(rb_operator, "geo_distance4")) {
        operator = GRN_OP_GEO_DISTANCE4;
    } else if (rb_grn_equal_option(rb_operator, "geo-withinp5") ||
               rb_grn_equal_option(rb_operator, "geo_withinp5")) {
        operator = GRN_OP_GEO_WITHINP5;
    } else if (rb_grn_equal_option(rb_operator, "geo-withinp6") ||
               rb_grn_equal_option(rb_operator, "geo_withinp6")) {
        operator = GRN_OP_GEO_WITHINP6;
    } else if (rb_grn_equal_option(rb_operator, "geo-withinp8") ||
               rb_grn_equal_option(rb_operator, "geo_withinp8")) {
        operator = GRN_OP_GEO_WITHINP8;
    } else if (rb_grn_equal_option(rb_operator, "object-search") ||
               rb_grn_equal_option(rb_operator, "object_search")) {
        operator = GRN_OP_OBJ_SEARCH;
    } else if (rb_grn_equal_option(rb_operator, "expression-get-variable") ||
               rb_grn_equal_option(rb_operator, "expression_get_variable")) {
        operator = GRN_OP_EXPR_GET_VAR;
    } else if (rb_grn_equal_option(rb_operator, "table-create") ||
               rb_grn_equal_option(rb_operator, "table_create")) {
        operator = GRN_OP_TABLE_CREATE;
    } else if (rb_grn_equal_option(rb_operator, "table-select") ||
               rb_grn_equal_option(rb_operator, "table_select")) {
        operator = GRN_OP_TABLE_SELECT;
    } else if (rb_grn_equal_option(rb_operator, "table-sort") ||
               rb_grn_equal_option(rb_operator, "table_sort")) {
        operator = GRN_OP_TABLE_SORT;
    } else if (rb_grn_equal_option(rb_operator, "table-group") ||
               rb_grn_equal_option(rb_operator, "table_group")) {
        operator = GRN_OP_TABLE_GROUP;
    } else if (rb_grn_equal_option(rb_operator, "json-put") ||
               rb_grn_equal_option(rb_operator, "json_put")) {
        operator = GRN_OP_JSON_PUT;
    } else if (rb_grn_equal_option(rb_operator, "regexp")) {
        operator = GRN_OP_REGEXP;
    } else {
        rb_raise(rb_eArgError,
                 "operator should be one of "
                 "[nil, Integer, name as String, name as Symbol]: <%s>",
                 rb_grn_inspect(rb_operator));
    }

    return operator;
}

grn_operator
rb_grn_set_operator_from_ruby_object (VALUE rb_operator)
{
    grn_operator operator = GRN_OP_OR;

    if (NIL_P(rb_operator)) {
        operator = GRN_OP_OR;
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(rb_operator, rb_cGrnOperator))) {
        operator = NUM2UINT(rb_iv_get(rb_operator, "@value"));
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(rb_operator, rb_cInteger))) {
        operator = NUM2UINT(rb_operator);
    } else if (rb_grn_equal_option(rb_operator, "or") ||
               rb_grn_equal_option(rb_operator, "||")) {
        operator = GRN_OP_OR;
    } else if (rb_grn_equal_option(rb_operator, "and") ||
               rb_grn_equal_option(rb_operator, "+") ||
               rb_grn_equal_option(rb_operator, "&&")) {
        operator = GRN_OP_AND;
    } else if (rb_grn_equal_option(rb_operator, "but") ||
               rb_grn_equal_option(rb_operator, "not") ||
               rb_grn_equal_option(rb_operator, "and-not") ||
               rb_grn_equal_option(rb_operator, "and_not") ||
               rb_grn_equal_option(rb_operator, "-") ||
               rb_grn_equal_option(rb_operator, "&!")) {
        operator = GRN_OP_AND_NOT;
    } else {
        rb_raise(rb_eArgError,
                 "operator should be one of "
                 "[:or, :||, :and, :+, :&&, :but, :not, :and_not, :-, :&!]: "
                 "<%s>",
                 rb_grn_inspect(rb_operator));
    }

    return operator;
}

static VALUE
rb_grn_operator_initialize (VALUE self, VALUE rb_name, VALUE rb_value)
{
    rb_iv_set(self, "@name", rb_name);
    rb_iv_set(self, "@value", rb_value);

    return Qnil;
}

static VALUE
rb_grn_operator_to_i (VALUE self)
{
    return rb_iv_get(self, "@value");
}

void
rb_grn_init_operator (VALUE mGrn)
{
    rb_cGrnOperator = rb_define_class_under(mGrn, "Operator", rb_cObject);
    /* @deprecated backward compatibility. */
    rb_define_const(mGrn, "Operation", rb_cGrnOperator);

    rb_define_attr(rb_cGrnOperator, "name", GRN_TRUE, GRN_FALSE);
    rb_define_attr(rb_cGrnOperator, "value", GRN_TRUE, GRN_FALSE);

    rb_define_method(rb_cGrnOperator, "initialize",
                     rb_grn_operator_initialize, 2);
    rb_define_method(rb_cGrnOperator, "to_i",
                     rb_grn_operator_to_i, 0);
    rb_define_alias(rb_cGrnOperator, "to_int", "to_i");

    rb_grn_init_equal_operator(mGrn);
    rb_grn_init_not_equal_operator(mGrn);
    rb_grn_init_less_operator(mGrn);
    rb_grn_init_greater_operator(mGrn);
    rb_grn_init_less_equal_operator(mGrn);
    rb_grn_init_greater_equal_operator(mGrn);
    rb_grn_init_match_operator(mGrn);

    rb_define_const(rb_cGrnOperator, "PUSH",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("push"),
                               UINT2NUM(GRN_OP_PUSH)));
    rb_define_const(rb_cGrnOperator, "POP",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("pop"),
                               UINT2NUM(GRN_OP_POP)));
    rb_define_const(rb_cGrnOperator, "NO_OPERATION",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("no-operation"),
                               UINT2NUM(GRN_OP_NOP)));
    rb_define_const(rb_cGrnOperator, "CALL",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("call"),
                               UINT2NUM(GRN_OP_CALL)));
    rb_define_const(rb_cGrnOperator, "INTERN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("intern"),
                               UINT2NUM(GRN_OP_INTERN)));
    rb_define_const(rb_cGrnOperator, "GET_REFERENCE",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("get-reference"),
                               UINT2NUM(GRN_OP_GET_REF)));
    rb_define_const(rb_cGrnOperator, "GET_VALUE",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("get-value"),
                               UINT2NUM(GRN_OP_GET_VALUE)));
    rb_define_const(rb_cGrnOperator, "AND",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("and"),
                               UINT2NUM(GRN_OP_AND)));
    rb_define_const(rb_cGrnOperator, "AND_NOT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("and-not"),
                               UINT2NUM(GRN_OP_AND_NOT)));
    /* Just for backward compatiblity. TODO: REMOVE ME! */
    rb_define_const(rb_cGrnOperator, "BUT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("but"),
                               UINT2NUM(GRN_OP_BUT)));
    rb_define_const(rb_cGrnOperator, "OR",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("or"),
                               UINT2NUM(GRN_OP_OR)));
    rb_define_const(rb_cGrnOperator, "ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("assign"),
                               UINT2NUM(GRN_OP_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "STAR_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("star-assign"),
                               UINT2NUM(GRN_OP_STAR_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "SLASH_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("slash-assign"),
                               UINT2NUM(GRN_OP_SLASH_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "MODULO_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("modulo-assign"),
                               UINT2NUM(GRN_OP_MOD_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "PLUS_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("plus-assign"),
                               UINT2NUM(GRN_OP_PLUS_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "MINUS_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("minus-assign"),
                               UINT2NUM(GRN_OP_MINUS_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "SHIFTL_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("shiftl-assign"),
                               UINT2NUM(GRN_OP_SHIFTL_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "SHIRTR_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("shirtr-assign"),
                               UINT2NUM(GRN_OP_SHIFTR_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "SHIFTRR_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("shiftrr-assign"),
                               UINT2NUM(GRN_OP_SHIFTRR_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "AND_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("and-assign"),
                               UINT2NUM(GRN_OP_AND_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "XOR_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("xor-assign"),
                               UINT2NUM(GRN_OP_XOR_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "OR_ASSIGN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("or-assign"),
                               UINT2NUM(GRN_OP_OR_ASSIGN)));
    rb_define_const(rb_cGrnOperator, "JUMP",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("jump"),
                               UINT2NUM(GRN_OP_JUMP)));
    rb_define_const(rb_cGrnOperator, "CJUMP",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("cjump"),
                               UINT2NUM(GRN_OP_CJUMP)));
    rb_define_const(rb_cGrnOperator, "COMMA",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("comma"),
                               UINT2NUM(GRN_OP_COMMA)));
    rb_define_const(rb_cGrnOperator, "BITWISE_OR",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("bitwise-or"),
                               UINT2NUM(GRN_OP_BITWISE_OR)));
    rb_define_const(rb_cGrnOperator, "BITWISE_XOR",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("bitwise-xor"),
                               UINT2NUM(GRN_OP_BITWISE_XOR)));
    rb_define_const(rb_cGrnOperator, "BITWISE_AND",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("bitwise-and"),
                               UINT2NUM(GRN_OP_BITWISE_AND)));
    rb_define_const(rb_cGrnOperator, "BITWISE_NOT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("bitwise-not"),
                               UINT2NUM(GRN_OP_BITWISE_NOT)));
    rb_define_const(rb_cGrnOperator, "EQUAL",
                    rb_funcall(rb_cGrnEqualOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("equal"),
                               UINT2NUM(GRN_OP_EQUAL)));
    rb_define_const(rb_cGrnOperator, "NOT_EQUAL",
                    rb_funcall(rb_cGrnNotEqualOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("not-equal"),
                               UINT2NUM(GRN_OP_NOT_EQUAL)));
    rb_define_const(rb_cGrnOperator, "LESS",
                    rb_funcall(rb_cGrnLessOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("less"),
                               UINT2NUM(GRN_OP_LESS)));
    rb_define_const(rb_cGrnOperator, "GREATER",
                    rb_funcall(rb_cGrnGreaterOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("greater"),
                               UINT2NUM(GRN_OP_GREATER)));
    rb_define_const(rb_cGrnOperator, "LESS_EQUAL",
                    rb_funcall(rb_cGrnLessEqualOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("less-equal"),
                               UINT2NUM(GRN_OP_LESS_EQUAL)));
    rb_define_const(rb_cGrnOperator, "GREATER_EQUAL",
                    rb_funcall(rb_cGrnGreaterEqualOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("greater-equal"),
                               UINT2NUM(GRN_OP_GREATER_EQUAL)));
    rb_define_const(rb_cGrnOperator, "IN",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("in"),
                               UINT2NUM(GRN_OP_IN)));
    rb_define_const(rb_cGrnOperator, "MATCH",
                    rb_funcall(rb_cGrnMatchOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("match"),
                               UINT2NUM(GRN_OP_MATCH)));
    rb_define_const(rb_cGrnOperator, "NEAR",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("near"),
                               UINT2NUM(GRN_OP_NEAR)));
    rb_define_const(rb_cGrnOperator, "NEAR2",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("near2"),
                               UINT2NUM(GRN_OP_NEAR2)));
    rb_define_const(rb_cGrnOperator, "SIMILAR",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("similar"),
                               UINT2NUM(GRN_OP_SIMILAR)));
    rb_define_const(rb_cGrnOperator, "TERM_EXTRACT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("term-extract"),
                               UINT2NUM(GRN_OP_TERM_EXTRACT)));
    rb_define_const(rb_cGrnOperator, "SHIFTL",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("shiftl"),
                               UINT2NUM(GRN_OP_SHIFTL)));
    rb_define_const(rb_cGrnOperator, "SHIFTR",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("shiftr"),
                               UINT2NUM(GRN_OP_SHIFTR)));
    rb_define_const(rb_cGrnOperator, "SHIFTRR",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("shiftrr"),
                               UINT2NUM(GRN_OP_SHIFTRR)));
    rb_define_const(rb_cGrnOperator, "PLUS",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("plus"),
                               UINT2NUM(GRN_OP_PLUS)));
    rb_define_const(rb_cGrnOperator, "MINUS",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("minus"),
                               UINT2NUM(GRN_OP_MINUS)));
    rb_define_const(rb_cGrnOperator, "STAR",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("star"),
                               UINT2NUM(GRN_OP_STAR)));
    rb_define_const(rb_cGrnOperator, "SLASH",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("slash"),
                               UINT2NUM(GRN_OP_SLASH)));
    rb_define_const(rb_cGrnOperator, "MODULO",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("modulo"),
                               UINT2NUM(GRN_OP_MOD)));
    rb_define_const(rb_cGrnOperator, "DELETE",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("delete"),
                               UINT2NUM(GRN_OP_DELETE)));
    rb_define_const(rb_cGrnOperator, "INCREMENT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("increment"),
                               UINT2NUM(GRN_OP_INCR)));
    rb_define_const(rb_cGrnOperator, "DECREMENT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("decrement"),
                               UINT2NUM(GRN_OP_DECR)));
    rb_define_const(rb_cGrnOperator, "INCREMENT_POST",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("increment-post"),
                               UINT2NUM(GRN_OP_INCR_POST)));
    rb_define_const(rb_cGrnOperator, "DECREMENT_POST",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("decrement-post"),
                               UINT2NUM(GRN_OP_DECR_POST)));
    rb_define_const(rb_cGrnOperator, "NOT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("not"),
                               UINT2NUM(GRN_OP_NOT)));
    rb_define_const(rb_cGrnOperator, "ADJUST",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("adjust"),
                               UINT2NUM(GRN_OP_ADJUST)));
    rb_define_const(rb_cGrnOperator, "EXACT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("exact"),
                               UINT2NUM(GRN_OP_EXACT)));
    rb_define_const(rb_cGrnOperator, "LONGEST_COMMON_PREFIX",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("longest-common-prefix"),
                               UINT2NUM(GRN_OP_LCP)));
    rb_define_const(rb_cGrnOperator, "PARTIAL",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("partial"),
                               UINT2NUM(GRN_OP_PARTIAL)));
    rb_define_const(rb_cGrnOperator, "UNSPLIT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("unsplit"),
                               UINT2NUM(GRN_OP_UNSPLIT)));
    rb_define_const(rb_cGrnOperator, "PREFIX",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("prefix"),
                               UINT2NUM(GRN_OP_PREFIX)));
    rb_define_const(rb_cGrnOperator, "SUFFIX",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("suffix"),
                               UINT2NUM(GRN_OP_SUFFIX)));
    rb_define_const(rb_cGrnOperator, "GEO_DISTANCE1",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-distance1"),
                               UINT2NUM(GRN_OP_GEO_DISTANCE1)));
    rb_define_const(rb_cGrnOperator, "GEO_DISTANCE2",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-distance2"),
                               UINT2NUM(GRN_OP_GEO_DISTANCE2)));
    rb_define_const(rb_cGrnOperator, "GEO_DISTANCE3",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-distance3"),
                               UINT2NUM(GRN_OP_GEO_DISTANCE3)));
    rb_define_const(rb_cGrnOperator, "GEO_DISTANCE4",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-distance4"),
                               UINT2NUM(GRN_OP_GEO_DISTANCE4)));
    rb_define_const(rb_cGrnOperator, "GEO_WITHINP5",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-withinp5"),
                               UINT2NUM(GRN_OP_GEO_WITHINP5)));
    rb_define_const(rb_cGrnOperator, "GEO_WITHINP6",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-withinp6"),
                               UINT2NUM(GRN_OP_GEO_WITHINP6)));
    rb_define_const(rb_cGrnOperator, "GEO_WITHINP8",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-withinp8"),
                               UINT2NUM(GRN_OP_GEO_WITHINP8)));
    rb_define_const(rb_cGrnOperator, "OBJECT_SEARCH",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("object-search"),
                               UINT2NUM(GRN_OP_OBJ_SEARCH)));
    rb_define_const(rb_cGrnOperator, "EXPRESSION_GET_VARIABLE",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("expression-get-variable"),
                               UINT2NUM(GRN_OP_EXPR_GET_VAR)));
    rb_define_const(rb_cGrnOperator, "TABLE_CREATE",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("table-create"),
                               UINT2NUM(GRN_OP_TABLE_CREATE)));
    rb_define_const(rb_cGrnOperator, "TABLE_SELECT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("table-select"),
                               UINT2NUM(GRN_OP_TABLE_SELECT)));
    rb_define_const(rb_cGrnOperator, "TABLE_SORT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("table-sort"),
                               UINT2NUM(GRN_OP_TABLE_SORT)));
    rb_define_const(rb_cGrnOperator, "TABLE_GROUP",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("table-group"),
                               UINT2NUM(GRN_OP_TABLE_GROUP)));
    rb_define_const(rb_cGrnOperator, "JSON_PUT",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("json-put"),
                               UINT2NUM(GRN_OP_JSON_PUT)));
    rb_define_const(rb_cGrnOperator, "REGEXP",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("regexp"),
                               UINT2NUM(GRN_OP_REGEXP)));


/*
    rb_define_const(rb_cGrnOperator, "GEO_DISTANCE1",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-distance1"),
                               UINT2NUM(GRN_OP_GEO_DISTANCE1)));
    rb_define_const(rb_cGrnOperator, "GEO_DISTANCE2",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-distance2"),
                               UINT2NUM(GRN_OP_GEO_DISTANCE2)));
    rb_define_const(rb_cGrnOperator, "GEO_DISTANCE3",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-distance3"),
                               UINT2NUM(GRN_OP_GEO_DISTANCE3)));
    rb_define_const(rb_cGrnOperator, "GEO_DISTANCE4",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-distance4"),
                               UINT2NUM(GRN_OP_GEO_DISTANCE4)));
    rb_define_const(rb_cGrnOperator, "GEO_WITHINP5",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-withinp5"),
                               UINT2NUM(GRN_OP_GEO_WITHINP5)));
    rb_define_const(rb_cGrnOperator, "GEO_WITHINP6",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-withinp6"),
                               UINT2NUM(GRN_OP_GEO_WITHINP6)));
    rb_define_const(rb_cGrnOperator, "GEO_WITHINP8",
                    rb_funcall(rb_cGrnOperator, rb_intern("new"), 2,
                               rb_str_new_cstr("geo-withinp8"),
                               UINT2NUM(GRN_OP_GEO_WITHINP8)));
*/
}
