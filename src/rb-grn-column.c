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
rb_grn_column_to_ruby_object (VALUE klass, grn_ctx *context, grn_obj *column)
{
    return GRNOBJECT2RVAL(klass, context, column);
}

static VALUE
rb_grn_index_column_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    grn_ctx *context;
    grn_rc rc;
    grn_id id;
    unsigned int section;
    grn_obj *old_value, *new_value;
    VALUE rb_section, rb_old_value, rb_new_value;

    context = rb_grn_object_ensure_context(self, Qnil);

    id = NUM2UINT(rb_id);

    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_value, rb_cHash))) {
	VALUE hash_value;
	hash_value = rb_hash_new();
	rb_hash_aset(hash_value, RB_GRN_INTERN("value"), rb_value);
	rb_value = hash_value;
    }

    rb_grn_scan_options(rb_value,
			"section", &rb_section,
			"old_value", &rb_old_value,
			"value", &rb_new_value,
			NULL);

    if (NIL_P(rb_section))
	section = 1;
    else
	section = NUM2UINT(rb_section);

    old_value = RVAL2GRNBULK(context, rb_old_value);
    new_value = RVAL2GRNBULK(context, rb_new_value);

    rc = grn_column_index_update(context, SELF(self),
				 id, section, old_value, new_value);
    grn_obj_close(context, old_value);
    grn_obj_close(context, new_value);

    rb_grn_rc_check(rc, self);

    return Qnil;
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

    rb_define_method(rb_cGrnIndexColumn, "[]=",
		     rb_grn_index_column_array_set, 2);
}
