/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-groonga-private.h"

static void
rb_grn_context_free (void *context)
{
    grn_ctx_fin (context);
    free (context);
}

static VALUE
rb_grn_context_alloc (VALUE klass)
{
    return Data_Wrap_Struct (klass, NULL, rb_grn_context_free, NULL);
}

static VALUE
rb_grn_context_initialize (VALUE self, VALUE argc, VALUE argv)
{
    VALUE options;

    rb_scan_args(argc, argv, "01", &options);
    if (NIL_P(options))
        options = rb_hash_new();

    /* FIXME */
    /* options = {:use_ql => true, :batch_mode => true, :encoding => :default} */
}

void
rb_grn_init_context (VALUE mGroonga)
{
    VALUE cGrnContext;

    cGrnCotnext = rb_define_class_under(mGroonga, "Context", rb_cObject);
    rb_define_alloc_func(cGrnContext, rb_grn_context_alloc);

    rb_define_method (cGrnContext, "initialize", rb_grn_context_initialize, -1);
}
