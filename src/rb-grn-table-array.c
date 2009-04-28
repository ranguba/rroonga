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

#define SELF(object) (RVAL2GRNTABLE(object))

VALUE rb_cGrnArray;

static VALUE
rb_grn_array_s_create (int argc, VALUE *argv, VALUE self)
{
    return rb_grn_table_s_create(argc, argv, self, GRN_TABLE_NO_KEY);
}

static VALUE
rb_grn_array_add (VALUE self)
{
    grn_ctx *context;
    grn_id id;

    context = rb_grn_object_ensure_context(self, Qnil);

    id = grn_table_add(context, SELF(self));
    rb_grn_context_check(context, self);

    if (GRN_ID_NIL == id)
	return Qnil;
    else
	return rb_grn_record_new(self, id);
}

void
rb_grn_init_table_array (VALUE mGrn)
{
    rb_cGrnArray = rb_define_class_under(mGrn, "Array", rb_cGrnTable);

    rb_define_singleton_method(rb_cGrnArray, "create",
			       rb_grn_array_s_create, -1);

    rb_define_method(rb_cGrnArray, "add", rb_grn_array_add, 0);
}
