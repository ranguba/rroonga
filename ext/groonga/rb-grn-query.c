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

/*
 * Document-class: Groonga::Query
 *
 * インデックスを用いた検索用のクエリのためのオブジェクト。
 * Groonga::IndexColumn#searchに渡すことができる。(このクラ
 * スは非推奨で、代わりにGroonga::Expressionを使用すること)
 * 
 */

#define SELF(object) (rb_rb_grn_query_from_ruby_object(object))

typedef struct _RbGrnQuery RbGrnQuery;
struct _RbGrnQuery
{
    grn_ctx *context;
    grn_query *query;
    grn_bool owner;
};

VALUE rb_cGrnQuery;

static RbGrnQuery *
rb_rb_grn_query_from_ruby_object (VALUE object)
{
    RbGrnQuery *rb_grn_query;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnQuery))) {
	rb_raise(rb_eTypeError, "not a groonga query");
    }

    Data_Get_Struct(object, RbGrnQuery, rb_grn_query);
    if (!rb_grn_query)
	rb_raise(rb_eGrnError, "groonga query is NULL");

    return rb_grn_query;
}

grn_query *
rb_grn_query_from_ruby_object (VALUE object)
{
    if (NIL_P(object))
        return NULL;

    return SELF(object)->query;
}

static void
rb_rb_grn_query_free (void *object)
{
    RbGrnQuery *rb_grn_query = object;

    if (rb_grn_query->owner && rb_grn_query->context && rb_grn_query->query)
	grn_query_close(rb_grn_query->context, rb_grn_query->query);

    xfree(object);
}

VALUE
rb_grn_query_to_ruby_object (grn_ctx *context, grn_query *query)
{
    RbGrnQuery *rb_grn_query;

    if (!query)
        return Qnil;

    rb_grn_query = ALLOC(RbGrnQuery);
    rb_grn_query->context = context;
    rb_grn_query->query = query;
    rb_grn_query->owner = GRN_FALSE;

    return Data_Wrap_Struct(rb_cGrnQuery, NULL,
                            rb_rb_grn_query_free, rb_grn_query);
}

static VALUE
rb_grn_query_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_rb_grn_query_free, NULL);
}

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

/*
 * call-seq:
 *   query.new(string, options={})
 *
 * _string_をパースした上で、クエリを作成する。作成されたオ
 * ブジェクトはGroonga::IndexColumn#searchに渡すことで使用す
 * ることができる。
 *
 * _options_に指定可能な値は以下の通り。
 *
 * [+:context+]
 *   クエリが利用するGroonga::Context。省略すると
 *   Groonga::Context.defaultを用いる。
 *
 * [+:default_operator+]
 *   演算子の既定値(演算子を省略した場合にどの演算を行うか)
 *   を指定する。
 *
 *   [Groonga::Operator::OR]
 *   [Groonga::Operator::AND]
 *   [Groonga::Operator::BUT]
 *   [Groonga::Operator::ADJUST]
 *     (FIXME: 挙動の違いを検証する必要性あり?
 *            Groonga::Expressionとの関連性は?)
 *
 * [+:max_expressions+]
 *   検索クエリに指定する式の最大値を指定する。
 */
static VALUE
rb_grn_query_initialize (int argc, VALUE *argv, VALUE self)
{
    RbGrnQuery *rb_grn_query;
    grn_ctx *context = NULL;
    grn_query *query;
    char *query_string;
    unsigned int query_string_length;
    grn_operator default_operator;
    int max_expressions = RB_GRN_QUERY_DEFAULT_MAX_EXPRESSIONS;
    VALUE rb_query_string, options, rb_context, rb_default_operator;
    VALUE rb_max_expressions;

    rb_scan_args(argc, argv, "11", &rb_query_string, &options);

    query_string = StringValuePtr(rb_query_string);
    query_string_length = RSTRING_LEN(rb_query_string);

    rb_grn_scan_options(options,
                        "context", &rb_context,
                        "default_operator", &rb_default_operator,
                        "max_expressions", &rb_max_expressions,
                        NULL);

    context = rb_grn_context_ensure(&rb_context);

    default_operator = RVAL2GRNOPERATOR(rb_default_operator);

    if (!NIL_P(rb_max_expressions))
        max_expressions = NUM2INT(rb_max_expressions);

    query = grn_query_open(context, query_string, query_string_length,
                           default_operator, max_expressions);
    rb_grn_context_check(context, rb_ary_new4(argc, argv));

    rb_grn_query = ALLOC(RbGrnQuery);
    DATA_PTR(self) = rb_grn_query;
    rb_grn_query->context = context;
    rb_grn_query->query = query;
    rb_grn_query->owner = GRN_TRUE;

    rb_iv_set(self, "@context", rb_context);

    return Qnil;
}

/*
 * call-seq:
 *   query.close
 *
 * _query_が使用しているリソースを開放する。これ以降_query_を
 * 使うことはできない。
 */
static VALUE
rb_grn_query_close (VALUE self)
{
    RbGrnQuery *rb_grn_query;

    rb_grn_query = SELF(self);
    if (rb_grn_query->context && rb_grn_query->query) {
        grn_rc rc;

        rc = grn_query_close(rb_grn_query->context, rb_grn_query->query);
        rb_grn_query->context = NULL;
        rb_grn_query->query = NULL;
        rb_grn_rc_check(rc, self);
    }
    return Qnil;
}

/*
 * call-seq:
 *   query.closed? -> true/false
 *
 * _query_が開放済みの場合は+true+を返し、そうでない場合は
 * +false+を返す。
 */
static VALUE
rb_grn_query_closed_p (VALUE self)
{
    RbGrnQuery *rb_grn_query;

    rb_grn_query = SELF(self);
    if (rb_grn_query->context && rb_grn_query->query)
        return Qfalse;
    else
        return Qtrue;
}

void
rb_grn_init_query (VALUE mGrn)
{
    rb_cGrnQuery = rb_define_class_under(mGrn, "Query", rb_cObject);
    rb_define_alloc_func(rb_cGrnQuery, rb_grn_query_alloc);

    rb_define_method(rb_cGrnQuery, "initialize",
                     rb_grn_query_initialize, -1);
    rb_define_method(rb_cGrnQuery, "close",
                     rb_grn_query_close, 0);
    rb_define_method(rb_cGrnQuery, "closed?",
                     rb_grn_query_closed_p, 0);
}
