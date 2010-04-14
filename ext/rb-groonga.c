/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>

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

rb_grn_boolean rb_grn_exited = RB_GRN_FALSE;
extern grn_ctx grn_gctx;

static VALUE
finish_groonga (VALUE self, VALUE object_id)
{
#ifndef WIN32
    grn_ctx *context = grn_gctx.next;

    debug("finish\n");
    while (context && context != &grn_gctx) {
	rb_grn_context_fin(context);
	context = context->next;
    }
    grn_fin();
    debug("finish: done\n");
#endif
    rb_grn_exited = RB_GRN_TRUE;

    return Qnil;
}

void
Init_groonga (void)
{
    VALUE mGrn;
    VALUE cGrnBuildVersion, cGrnBindingsVersion;
    VALUE groonga_finalizer, groonga_finalizer_keeper;

    mGrn = rb_define_module("Groonga");

    groonga_finalizer = rb_funcall(rb_cObject, rb_intern("new"), 0);
    rb_define_singleton_method(groonga_finalizer, "call", finish_groonga, 1);
    groonga_finalizer_keeper = rb_funcall(rb_cObject, rb_intern("new"), 0);
    rb_funcall(rb_const_get(rb_cObject, rb_intern("ObjectSpace")),
	       rb_intern("define_finalizer"),
	       2, groonga_finalizer_keeper, groonga_finalizer);
    rb_iv_set(mGrn, "finalizer", groonga_finalizer_keeper);

    cGrnBuildVersion = rb_ary_new3(3,
				   INT2NUM(GRN_MAJOR_VERSION),
				   INT2NUM(GRN_MINOR_VERSION),
				   INT2NUM(GRN_MICRO_VERSION));
    rb_obj_freeze(cGrnBuildVersion);
    /*
     * ビルドしたgroongaのバージョン。<tt>[メジャーバージョン,
     * マイナーバージョン, マイクロバージョン]</tt>の配列。
     */
    rb_define_const(mGrn, "BUILD_VERSION", cGrnBuildVersion);

    /* FIXME: API to get runtime groonga version doesn't exist */

    /*
     * 利用しているgroongaのバージョン（にしたい。現在は
     * BUILD_VERSIONと同じ）。<tt>[メジャーバージョ
     * ン, マイナーバージョン, マイクロバージョン]</tt>の配列。
     */
    rb_define_const(mGrn, "VERSION", cGrnBuildVersion);

    cGrnBindingsVersion = rb_ary_new3(3,
				      INT2NUM(RB_GRN_MAJOR_VERSION),
				      INT2NUM(RB_GRN_MINOR_VERSION),
				      INT2NUM(RB_GRN_MICRO_VERSION));
    rb_obj_freeze(cGrnBindingsVersion);
    /*
     * Ruby/groongaのバージョン。<tt>[メジャーバージョン, マ
     * イナーバージョン, マイクロバージョン]</tt>の配列。
     */
    rb_define_const(mGrn, "BINDINGS_VERSION", cGrnBindingsVersion);

    rb_grn_init_exception(mGrn);

    rb_grn_rc_check(grn_init(), Qnil);

    rb_grn_init_utils(mGrn);
    rb_grn_init_encoding(mGrn);
    rb_grn_init_encoding_support(mGrn);
    rb_grn_init_context(mGrn);
    rb_grn_init_object(mGrn);
    rb_grn_init_database(mGrn);
    rb_grn_init_table(mGrn);
    rb_grn_init_table_cursor(mGrn);
    rb_grn_init_type(mGrn);
    rb_grn_init_procedure(mGrn);
    rb_grn_init_column(mGrn);
    rb_grn_init_accessor(mGrn);
    rb_grn_init_view_accessor(mGrn);
    rb_grn_init_record(mGrn);
    rb_grn_init_view_record(mGrn);
    rb_grn_init_query(mGrn);
    rb_grn_init_variable(mGrn);
    rb_grn_init_operation(mGrn);
    rb_grn_init_expression(mGrn);
    rb_grn_init_expression_builder(mGrn);
    rb_grn_init_logger(mGrn);
    rb_grn_init_snippet(mGrn);
}
