/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2016-2022  Sutou Kouhei <kou@clear-code.com>

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

/*
 * Document-class: Groonga::RequestTimerID
 *
 * This class represents request timer ID. It's used with request timer.
 */

VALUE rb_cGrnRequestTimerID;

static rb_data_type_t data_type = {
    "Groonga::RequestTimeID",
    {
        NULL,
        NULL,
        NULL,
    },
    NULL,
    NULL,
    0
};

static VALUE
rb_grn_request_timer_id_alloc (VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &data_type, NULL);
}

void *
rb_grn_request_timer_id_from_ruby_object (VALUE rb_id)
{
    if (NIL_P(rb_id)) {
        return NULL;
    } else {
        void *id;
        TypedData_Get_Struct(rb_id, void *, &data_type, id);
        return id;
    }
}

VALUE
rb_grn_request_timer_id_to_ruby_object (void *id)
{
    return TypedData_Wrap_Struct(rb_cGrnRequestTimerID, &data_type, id);
}

void
rb_grn_init_request_timer_id (VALUE mGrn)
{
    rb_cGrnRequestTimerID =
        rb_define_class_under(mGrn, "RequestTimerID", rb_cObject);
    rb_define_alloc_func(rb_cGrnRequestTimerID,
                         rb_grn_request_timer_id_alloc);
}
