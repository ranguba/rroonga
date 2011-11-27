/* -*- coding: utf-8; c-file-style: "ruby" -*- */
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

#define SELF(object) ((RbGrnIndexCursor *)DATA_PTR(object))

VALUE rb_cGrnIndexCursor;

VALUE
rb_grn_index_cursor_to_ruby_object (grn_ctx *context,
				    grn_obj *cursor,
				    grn_bool owner)
{
    return GRNOBJECT2RVAL(rb_cGrnIndexCursor, context, cursor, owner);
}

void
rb_grn_index_cursor_deconstruct (RbGrnIndexCursor *rb_grn_index_cursor,
				 grn_obj **cursor,
				 grn_ctx **context,
				 grn_id *domain_id,
				 grn_obj **domain,
				 grn_id *range_id,
				 grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_index_cursor);
    rb_grn_object_deconstruct(rb_grn_object, cursor, context,
			      domain_id, domain,
			      range_id, range);
}

static VALUE
rb_grn_index_cursor_next (VALUE self)
{
    VALUE rb_posting = Qnil;
    grn_obj *cursor;
    grn_ctx *context;

    rb_grn_index_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);

    if (context && cursor) {
	grn_posting *posting;
	grn_id tid;

	posting = grn_index_cursor_next(context, cursor, &tid);

	if (posting) {
	    rb_posting = rb_grn_posting_new(posting, tid);
	}
    }

    return rb_posting;

}

static VALUE
rb_grn_index_cursor_each (VALUE self)
{
    grn_obj *cursor;
    grn_ctx *context;

    rb_grn_index_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);

    if (context && cursor) {
	grn_posting *posting;
	grn_id tid;

	while ((posting = grn_index_cursor_next(context, cursor, &tid))) {
	    rb_yield(rb_grn_posting_new(posting, tid));
	}
    }

    return Qnil;
}

void
rb_grn_init_index_cursor (VALUE mGrn)
{
    rb_cGrnIndexCursor =
	rb_define_class_under(mGrn, "IndexCursor", rb_cGrnObject);
    rb_define_alloc_func(rb_cGrnIndexCursor, rb_grn_object_alloc);
    rb_include_module(rb_cGrnIndexCursor, rb_mEnumerable);

    rb_define_method(rb_cGrnIndexCursor, "next", rb_grn_index_cursor_next, 0);
    rb_define_method(rb_cGrnIndexCursor, "each", rb_grn_index_cursor_each, 0);
}
