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

#define SELF(object) ((RbGrnColumn *)DATA_PTR(object))

VALUE rb_cGrnColumn;
VALUE rb_cGrnVarSizeColumn;

/*
 * Document-class: Groonga::Column < Groonga::Object
 *
 * テーブルに情報を付加するためのオブジェクト。テーブルに複
 * 数のカラムを定義することによりレコード毎に複数の情報を付
 * 加することができる。
 *
 * カラムには大きく分けて3種類ある。
 * [Groonga::FixSizeColumn]
 *   固定長のデータを格納するカラム。
 * [Groonga::VarSizeColumn]
 *   可変長のデータを格納するカラム。
 * [Groonga::IndexColumn]
 *   転置インデックスを格納するカラム。全文検索や参照元レコー
 *   ドの検索を行う場合はこのカラムを使用する。
 *
 * 固定長データ用カラム・可変長データ用カラムは1つのデータだ
 * けを格納するか複数のデータを格納するかを選ぶことができる。
 * 1つのデータの場合はスカラ値、複数のデータの場合はスカラー
 * 値を格納するという。
 *
 * カラムは名前を持ち、1つのテーブルでは同じカラム名を持つカ
 * ラムを複数定義することはできない。
 */

grn_obj *
rb_grn_column_from_ruby_object (VALUE object, grn_ctx **context)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnColumn))) {
	rb_raise(rb_eTypeError, "not a groonga column");
    }

    return RVAL2GRNOBJECT(object, context);
}

VALUE
rb_grn_column_to_ruby_object (VALUE klass, grn_ctx *context, grn_obj *column,
			      rb_grn_boolean owner)
{
    return GRNOBJECT2RVAL(klass, context, column, owner);
}

static VALUE
rb_grn_column_get_table (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj *table;

    rb_grn_object_deconstruct((RbGrnObject *)(SELF(self)), &column, &context,
			      NULL, NULL,
			      NULL, NULL);
    table = grn_column_table(context, column);
    rb_grn_context_check(context, self);

    return GRNOBJECT2RVAL(Qnil, context, table, RB_GRN_FALSE);
}

void
rb_grn_init_column (VALUE mGrn)
{
    rb_cGrnColumn = rb_define_class_under(mGrn, "Column", rb_cGrnObject);

    rb_cGrnVarSizeColumn =
	rb_define_class_under(mGrn, "VarSizeColumn", rb_cGrnColumn);

    rb_define_method(rb_cGrnColumn, "table", rb_grn_column_get_table, 0);

    rb_grn_init_fix_size_column(mGrn);
    rb_grn_init_index_column(mGrn);
}
