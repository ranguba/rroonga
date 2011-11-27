/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>

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

VALUE rb_cGrnRecord;

VALUE
rb_grn_record_new (VALUE table, grn_id id, VALUE values)
{
    return rb_grn_record_new_raw(table, UINT2NUM(id), values);
}

VALUE
rb_grn_record_new_added (VALUE table, grn_id id, VALUE values)
{
    VALUE record;

    record = rb_grn_record_new(table, id, values);
    rb_funcall(record, rb_intern("added="), 1, Qtrue);
    return record;
}

VALUE
rb_grn_record_new_raw (VALUE table, VALUE rb_id, VALUE values)
{
    return rb_funcall(rb_cGrnRecord, rb_intern("new"), 3,
		      table, rb_id, values);
}

void
rb_grn_init_record (VALUE mGrn)
{
    rb_cGrnRecord = rb_const_get(mGrn, rb_intern("Record"));
}
