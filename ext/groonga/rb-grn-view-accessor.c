/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>

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

#define SELF(object) (RVAL2GRNVIEWACCESSOR(object))

VALUE rb_cGrnViewAccessor;

grn_obj *
rb_grn_view_accessor_from_ruby_object (VALUE object)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnViewAccessor))) {
	rb_raise(rb_eTypeError, "not a groonga accessor view");
    }

    return RVAL2GRNOBJECT(object, NULL);
}

VALUE
rb_grn_view_accessor_to_ruby_object (grn_ctx *context, grn_obj *table,
                                     grn_bool owner)
{
    return GRNOBJECT2RVAL(rb_cGrnViewAccessor, context, table, owner);
}

/*
 * Document-class: Groonga::ViewAccessor < Groonga::Object
 *
 * キー、値、スコアなど種々の値へのアクセスをカプセル化した
 * オブジェクト。Groonga::Table#columnで取得できる。
 */
void
rb_grn_init_view_accessor (VALUE mGrn)
{
    rb_cGrnViewAccessor =
        rb_define_class_under(mGrn, "ViewAccessor", rb_cGrnObject);
}
