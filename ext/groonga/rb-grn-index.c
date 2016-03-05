/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2016  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "rb-grn.h"

VALUE rb_cGrnIndex;

VALUE
rb_grn_index_new (VALUE rb_index_column, VALUE rb_section)
{
    return rb_funcall(rb_cGrnIndex, rb_intern("new"), 2,
                      rb_index_column, rb_section);
}

void
rb_grn_init_index (VALUE mGrn)
{
    rb_cGrnIndex = rb_const_get(mGrn, rb_intern("Index"));
}
