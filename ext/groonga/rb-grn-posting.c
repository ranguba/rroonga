/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2011  Haruka Yoshihara <yoshihara@clear-code.com>
  Copyright (C) 2012  Kouhei Sutou <kou@clear-code.com>

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

VALUE rb_cGrnPosting;

VALUE
rb_grn_posting_new (grn_posting *posting, grn_id term_id,
                    VALUE rb_table, VALUE rb_lexicon)
{
    VALUE parameters;

    parameters = rb_hash_new();

#define SET_PARAMETER(key, value) \
    rb_hash_aset(parameters, (rb_str_intern(rb_str_new2((key)))), INT2NUM((value)))

    SET_PARAMETER("record_id", posting->rid);
    SET_PARAMETER("section_id", posting->sid);
    SET_PARAMETER("term_id", term_id);
    SET_PARAMETER("position", posting->pos);
    SET_PARAMETER("term_frequency", posting->tf);
    SET_PARAMETER("weight", posting->weight);
    SET_PARAMETER("n_rest_postings", posting->rest);

#undef SET_PARAMETER

    rb_hash_aset(parameters, rb_str_intern(rb_str_new2("table")), rb_table);
    rb_hash_aset(parameters, rb_str_intern(rb_str_new2("lexicon")), rb_lexicon);

    return rb_funcall(rb_cGrnPosting, rb_intern("new"), 1,
                      parameters);
}

void
rb_grn_init_posting (VALUE mGrn)
{
    rb_cGrnPosting = rb_const_get(mGrn, rb_intern("Posting"));
}
