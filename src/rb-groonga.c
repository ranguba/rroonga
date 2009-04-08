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

static void
finish_groonga (void)
{
    grn_fin();
}

void
Init_groonga(void)
{
    VALUE mGrn;

    mGrn = rb_define_module("Groonga");

    rb_grn_init_utils(mGrn);
    rb_grn_init_exception(mGrn);
    rb_grn_init_encoding(mGrn);
    rb_grn_init_context(mGrn);
    rb_grn_init_object(mGrn);
    rb_grn_init_database(mGrn);
    rb_grn_init_table(mGrn);
    rb_grn_init_table_cursor(mGrn);
    rb_grn_init_type(mGrn);
    rb_grn_init_procedure(mGrn);
    rb_grn_init_column(mGrn);
    rb_grn_init_record(mGrn);
    rb_grn_init_query(mGrn);

    rb_grn_check_rc(grn_init());
    atexit(finish_groonga);
}
