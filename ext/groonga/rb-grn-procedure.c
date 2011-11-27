/* -*- coding: utf-8; c-file-style: "ruby" -*- */
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

#define SELF(object) (RVAL2GRNPROCEDURE(object))

VALUE rb_cGrnProcedure;

grn_obj *
rb_grn_procedure_from_ruby_object (VALUE object)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnProcedure))) {
	rb_raise(rb_eTypeError, "not a groonga procedure");
    }

    return RVAL2GRNOBJECT(object, NULL);
}

VALUE
rb_grn_procedure_to_ruby_object (grn_ctx *context, grn_obj *procedure,
				 grn_bool owner)
{
    return GRNOBJECT2RVAL(rb_cGrnProcedure, context, procedure, owner);
}

void
rb_grn_init_procedure (VALUE mGrn)
{
    rb_cGrnProcedure = rb_define_class_under(mGrn, "Procedure", rb_cGrnObject);

    rb_define_const(rb_cGrnProcedure, "DELIMIT", INT2NUM(GRN_DB_DELIMIT));
    rb_define_const(rb_cGrnProcedure, "UNIGRAM", INT2NUM(GRN_DB_UNIGRAM));
    rb_define_const(rb_cGrnProcedure, "BIGRAM", INT2NUM(GRN_DB_BIGRAM));
    rb_define_const(rb_cGrnProcedure, "TRIGRAM", INT2NUM(GRN_DB_TRIGRAM));
    rb_define_const(rb_cGrnProcedure, "MECAB", INT2NUM(GRN_DB_MECAB));
}
