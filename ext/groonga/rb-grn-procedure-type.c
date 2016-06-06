/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2014-2016  Kouhei Sutou <kou@clear-code.com>

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

void
rb_grn_init_procedure_type (VALUE mGrn)
{
    VALUE rb_mGrnProcedureType;

    rb_mGrnProcedureType = rb_define_module_under(mGrn, "ProcedureType");

    rb_define_const(rb_mGrnProcedureType,
                    "INVALID", INT2NUM(GRN_PROC_INVALID));
    rb_define_const(rb_mGrnProcedureType,
                    "TOKENIZER", INT2NUM(GRN_PROC_TOKENIZER));
    rb_define_const(rb_mGrnProcedureType,
                    "COMMAND", INT2NUM(GRN_PROC_COMMAND));
    rb_define_const(rb_mGrnProcedureType,
                    "FUNCTION", INT2NUM(GRN_PROC_FUNCTION));
    rb_define_const(rb_mGrnProcedureType,
                    "HOOK", INT2NUM(GRN_PROC_HOOK));
    rb_define_const(rb_mGrnProcedureType,
                    "NORMALIZER", INT2NUM(GRN_PROC_NORMALIZER));
    rb_define_const(rb_mGrnProcedureType,
                    "TOKEN_FILTER", INT2NUM(GRN_PROC_TOKEN_FILTER));
    rb_define_const(rb_mGrnProcedureType,
                    "SCORER", INT2NUM(GRN_PROC_SCORER));

    /*
     * It's a type for window function.
     *
     * @since 6.0.4
     */
    rb_define_const(rb_mGrnProcedureType,
                    "WINDOW_FUNCTION", INT2NUM(GRN_PROC_WINDOW_FUNCTION));
}
