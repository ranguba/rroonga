/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-grn.h"

#define SELF(object) (RVAL2GRNCOLUMN(object))

VALUE rb_cGrnColumn;
VALUE rb_cGrnFixSizeColumn;
VALUE rb_cGrnVarSizeColumn;
VALUE rb_cGrnIndexColumn;

grn_obj *
rb_grn_column_from_ruby_object (VALUE object)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnColumn))) {
	rb_raise(rb_eTypeError, "not a groonga column");
    }

    return RVAL2GRNOBJECT(object, NULL);
}

VALUE
rb_grn_column_to_ruby_object (grn_ctx *context, grn_obj *column)
{
    return GRNOBJECT2RVAL(rb_cGrnColumn, context, column);
}

void
rb_grn_init_column (VALUE mGrn)
{
    rb_cGrnColumn = rb_define_class_under(mGrn, "Column", rb_cGrnObject);
    rb_cGrnFixSizeColumn =
	rb_define_class_under(mGrn, "FixSizeColumn", rb_cGrnColumn);
    rb_cGrnVarSizeColumn =
	rb_define_class_under(mGrn, "VarSizeColumn", rb_cGrnColumn);
    rb_cGrnIndexColumn =
	rb_define_class_under(mGrn, "IndexColumn", rb_cGrnColumn);
}
