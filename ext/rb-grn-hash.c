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

VALUE rb_cGrnHash;

static VALUE
rb_grn_hash_s_create (int argc, VALUE *argv, VALUE self)
{
    return rb_grn_table_s_create(argc, argv, self, GRN_TABLE_HASH_KEY);
}

void
rb_grn_init_hash (VALUE mGrn)
{
    rb_cGrnHash = rb_define_class_under(mGrn, "Hash", rb_cGrnTable);

    rb_include_module(rb_cGrnHash, rb_mGrnTableKeySupport);

    rb_define_singleton_method(rb_cGrnHash, "create",
			       rb_grn_hash_s_create, -1);
}
