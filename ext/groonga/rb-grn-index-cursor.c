/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2011  Haruka Yoshihara <yoshihara@clear-code.com>
  Copyright (C) 2012-2021  Sutou Kouhei <kou@clear-code.com>
  Copyright (C) 2019  Horimoto Yasuhiro <horimoto@clear-code.com>

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

#define SELF(object) ((RbGrnIndexCursor *)RTYPEDDATA_DATA(object))

VALUE rb_cGrnIndexCursor;

VALUE
rb_grn_index_cursor_to_ruby_object (grn_ctx *context,
                                    grn_obj *cursor,
                                    VALUE rb_table,
                                    VALUE rb_lexicon,
                                    grn_bool owner)
{
    VALUE rb_cursor;

    rb_cursor = GRNOBJECT2RVAL(rb_cGrnIndexCursor, context, cursor, owner);
    rb_iv_set(rb_cursor, "@table", rb_table);
    rb_iv_set(rb_cursor, "@lexicon", rb_lexicon);

    return rb_cursor;
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
rb_grn_index_cursor_s_set_min_p (VALUE klass)
{
    return CBOOL2RVAL(grn_ii_cursor_set_min_enable_get());
}

static VALUE
rb_grn_index_cursor_s_set_min_set (VALUE klass, VALUE enable)
{
    grn_ii_cursor_set_min_enable_set(RVAL2CBOOL(enable));
    return enable;
}

static VALUE
next_value (VALUE rb_posting,
            grn_ctx *context, grn_obj *cursor, VALUE rb_table, VALUE rb_lexicon)
{
    grn_posting *posting;
    grn_id term_id;

    posting = grn_index_cursor_next(context, cursor, &term_id);
    if (!posting) {
        return Qnil;
    }

    if (NIL_P(rb_posting)) {
        return rb_grn_posting_new(posting, term_id, rb_table, rb_lexicon);
    } else {
        rb_grn_posting_update(rb_posting, posting, term_id);
        return rb_posting;
    }
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
        VALUE rb_table;
        VALUE rb_lexicon;
        rb_table = rb_iv_get(self, "@table");
        rb_lexicon = rb_iv_get(self, "@lexicon");
        rb_posting = next_value(Qnil, context, cursor, rb_table, rb_lexicon);
    }

    return rb_posting;

}

static VALUE
rb_grn_index_cursor_each (int argc, VALUE *argv, VALUE self)
{
    grn_obj *cursor;
    grn_ctx *context;
    grn_bool reuse_posting_object;
    VALUE rb_options;
    VALUE rb_reuse_posting_object;
    VALUE rb_table;
    VALUE rb_lexicon;
    VALUE rb_posting = Qnil;

    RETURN_ENUMERATOR(self, argc, argv);

    rb_scan_args(argc, argv, "01", &rb_options);

    rb_grn_scan_options(rb_options,
                        "reuse_posting_object", &rb_reuse_posting_object,
                        NULL);

    rb_grn_index_cursor_deconstruct(SELF(self), &cursor, &context,
                                    NULL, NULL, NULL, NULL);

    if (!context) {
        return Qnil;
    }

    if (!cursor) {
        return Qnil;
    }

    rb_table = rb_iv_get(self, "@table");
    rb_lexicon = rb_iv_get(self, "@lexicon");
    reuse_posting_object = RVAL2CBOOL(rb_reuse_posting_object);

    if (reuse_posting_object) {
        rb_posting = rb_grn_posting_new(NULL, GRN_ID_NIL, rb_table, rb_lexicon);
    }
    while (GRN_TRUE) {
        if (!reuse_posting_object) {
            rb_posting = Qnil;
        }
        rb_posting = next_value(rb_posting,
                                context, cursor, rb_table, rb_lexicon);
        if (NIL_P(rb_posting)) {
            break;
        }
        rb_yield(rb_posting);
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

    rb_define_singleton_method(rb_cGrnIndexCursor, "set_min?",
                               rb_grn_index_cursor_s_set_min_p, 0);
    rb_define_singleton_method(rb_cGrnIndexCursor, "set_min=",
                               rb_grn_index_cursor_s_set_min_set, 1);

    rb_define_method(rb_cGrnIndexCursor, "next", rb_grn_index_cursor_next, 0);
    rb_define_method(rb_cGrnIndexCursor, "each", rb_grn_index_cursor_each, -1);
}
