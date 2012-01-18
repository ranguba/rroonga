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

grn_bool rb_grn_exited = GRN_FALSE;

static void
finish_groonga (VALUE data)
{
    debug("finish\n");
    grn_fin();
    rb_grn_exited = GRN_TRUE;
    debug("finish: done\n");
}

static void
rb_grn_init_runtime_version (VALUE mGrn)
{
    const char *component_start, *component_end;
    int component_length;
    VALUE runtime_version;
    VALUE major, minor, micro, tag;

    runtime_version = rb_ary_new();

    component_start = grn_get_version();
    component_end = strstr(component_start, ".");
    component_length = component_end - component_start;
    major = rb_str_new(component_start, component_length);
    rb_ary_push(runtime_version, rb_Integer(major));

    component_start = component_end + 1;
    component_end = strstr(component_start, ".");
    component_length = component_end - component_start;
    minor = rb_str_new(component_start, component_length);
    rb_ary_push(runtime_version, rb_Integer(minor));

    component_start = component_end + 1;
    component_end = strstr(component_start, "-");
    if (component_end) {
	component_length = component_end - component_start;
    } else {
	component_length = strlen(component_start);
    }
    micro = rb_str_new(component_start, component_length);
    rb_ary_push(runtime_version, rb_Integer(micro));

    if (component_end) {
	tag = rb_str_new2(component_end + 1);
    } else {
	tag = Qnil;
    }
    rb_ary_push(runtime_version, tag);

    rb_obj_freeze(runtime_version);
    /*
     * 利用しているgroongaのバージョン。<tt>[メジャーバージョ
     * ン, マイナーバージョン, マイクロバージョン, タグ]</tt>の
     * 配列。
     */
    rb_define_const(mGrn, "VERSION", runtime_version);
}

static void
rb_grn_init_version (VALUE mGrn)
{
    VALUE build_version, bindings_version;

    rb_grn_init_runtime_version(mGrn);

    build_version = rb_ary_new3(3,
				INT2NUM(GRN_MAJOR_VERSION),
				INT2NUM(GRN_MINOR_VERSION),
				INT2NUM(GRN_MICRO_VERSION));
    rb_obj_freeze(build_version);
    /*
     * ビルドしたgroongaのバージョン。<tt>[メジャーバージョン,
     * マイナーバージョン, マイクロバージョン]</tt>の配列。
     */
    rb_define_const(mGrn, "BUILD_VERSION", build_version);

    bindings_version = rb_ary_new3(3,
				   INT2NUM(RB_GRN_MAJOR_VERSION),
				   INT2NUM(RB_GRN_MINOR_VERSION),
				   INT2NUM(RB_GRN_MICRO_VERSION));
    rb_obj_freeze(bindings_version);
    /*
     * rroongaのバージョン。<tt>[メジャーバージョン, マ
     * イナーバージョン, マイクロバージョン]</tt>の配列。
     */
    rb_define_const(mGrn, "BINDINGS_VERSION", bindings_version);
}

void
Init_groonga (void)
{
    VALUE mGrn;

    mGrn = rb_define_module("Groonga");

    rb_grn_init_exception(mGrn);

    rb_grn_rc_check(grn_init(), Qnil);
    rb_set_end_proc(finish_groonga, Qnil);

    rb_grn_init_version(mGrn);

    rb_grn_init_utils(mGrn);
    rb_grn_init_encoding(mGrn);
    rb_grn_init_encoding_support(mGrn);
    rb_grn_init_context(mGrn);
    rb_grn_init_object(mGrn);
    rb_grn_init_database(mGrn);
    rb_grn_init_table(mGrn);
    rb_grn_init_table_cursor(mGrn);
    rb_grn_init_index_cursor(mGrn);
    rb_grn_init_posting(mGrn);
    rb_grn_init_type(mGrn);
    rb_grn_init_procedure(mGrn);
    rb_grn_init_column(mGrn);
    rb_grn_init_accessor(mGrn);
    rb_grn_init_view_accessor(mGrn);
    rb_grn_init_record(mGrn);
    rb_grn_init_view_record(mGrn);
    rb_grn_init_variable(mGrn);
    rb_grn_init_operator(mGrn);
    rb_grn_init_expression(mGrn);
    rb_grn_init_expression_builder(mGrn);
    rb_grn_init_logger(mGrn);
    rb_grn_init_snippet(mGrn);
    rb_grn_init_plugin(mGrn);
}
