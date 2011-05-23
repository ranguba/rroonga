/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2011  Haruka Yoshihara <yoshihara@clear-code.com>

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

VALUE rb_cGrnPosting;

VALUE
rb_grn_posting_new (grn_posting *posting, grn_id term_id)
{
    VALUE parameters;

    parameters = rb_hash_new();
    rb_hash_aset(parameters, ID2SYM(rb_to_id(rb_str_new2("record_id"))), INT2NUM(posting->rid));
    rb_hash_aset(parameters, ID2SYM(rb_to_id(rb_str_new2("section_id"))), INT2NUM(posting->sid));
    rb_hash_aset(parameters, ID2SYM(rb_to_id(rb_str_new2("term_id"))), INT2NUM(term_id));
    rb_hash_aset(parameters, ID2SYM(rb_to_id(rb_str_new2("position"))), INT2NUM(posting->pos));
    rb_hash_aset(parameters, ID2SYM(rb_to_id(rb_str_new2("term_frequency"))), INT2NUM(posting->tf));
    rb_hash_aset(parameters, ID2SYM(rb_to_id(rb_str_new2("weight"))), INT2NUM(posting->weight));
    rb_hash_aset(parameters, ID2SYM(rb_to_id(rb_str_new2("n_rest_postings"))), INT2NUM(posting->rest));

    return rb_funcall(rb_cGrnPosting, rb_intern("new"), 1,
		      parameters);
}

void
rb_grn_init_posting (VALUE mGrn)
{
    rb_cGrnPosting = rb_const_get(mGrn, rb_intern("Posting"));
}
