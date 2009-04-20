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

#define SELF(object) (rb_rb_grn_table_cursor_from_ruby_object(object))

typedef struct _RbGrnTableCursor RbGrnTableCursor;
struct _RbGrnTableCursor
{
    grn_ctx *context;
    grn_table_cursor *cursor;
};

VALUE rb_cGrnTableCursor;
VALUE rb_cGrnHashCursor;
VALUE rb_cGrnPatriciaTrieCursor;
VALUE rb_cGrnArrayCursor;

static RbGrnTableCursor *
rb_rb_grn_table_cursor_from_ruby_object (VALUE object)
{
    RbGrnTableCursor *rb_grn_table_cursor;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnTableCursor))) {
	rb_raise(rb_eTypeError, "not a groonga table cursor");
    }

    Data_Get_Struct(object, RbGrnTableCursor, rb_grn_table_cursor);
    if (!rb_grn_table_cursor)
	rb_raise(rb_eGrnError, "groonga table cursor is NULL");

    return rb_grn_table_cursor;
}

grn_table_cursor *
rb_grn_table_cursor_from_ruby_object (VALUE object)
{
    if (NIL_P(object))
        return NULL;

    return SELF(object)->cursor;
}

static void
rb_rb_grn_table_cursor_free (void *object)
{
    xfree(object);
}

VALUE
rb_grn_table_cursor_to_ruby_class (grn_obj *object)
{
    VALUE klass = Qnil;

    switch (object->header.type) {
      case GRN_CURSOR_TABLE_HASH_KEY:
	klass = rb_cGrnHashCursor;
	break;
      case GRN_CURSOR_TABLE_PAT_KEY:
	klass = rb_cGrnPatriciaTrieCursor;
	break;
      case GRN_CURSOR_TABLE_NO_KEY:
	klass = rb_cGrnArrayCursor;
	break;
      default:
	rb_raise(rb_eTypeError,
		 "unsupported groonga object type: %d",
		 object->header.type);
	break;
    }

    return klass;
}

VALUE
rb_grn_table_cursor_to_ruby_object (VALUE klass, grn_ctx *context,
                                    grn_table_cursor *cursor)
{
    RbGrnTableCursor *rb_grn_table_cursor;

    if (!cursor)
        return Qnil;

    rb_grn_table_cursor = ALLOC(RbGrnTableCursor);
    rb_grn_table_cursor->context = context;
    rb_grn_table_cursor->cursor = cursor;

    if (NIL_P(klass))
        klass = GRNTABLECURSOR2RCLASS(cursor);

    return Data_Wrap_Struct(klass, NULL,
                            rb_rb_grn_table_cursor_free, rb_grn_table_cursor);
}

static VALUE
rb_grn_table_cursor_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_rb_grn_table_cursor_free, NULL);
}

VALUE
rb_grn_table_cursor_close (VALUE self)
{
    RbGrnTableCursor *rb_grn_table_cursor;

    rb_grn_table_cursor = SELF(self);
    if (rb_grn_table_cursor->context && rb_grn_table_cursor->cursor) {
        grn_rc rc;

        rc = grn_table_cursor_close(rb_grn_table_cursor->context,
                                    rb_grn_table_cursor->cursor);
        rb_grn_table_cursor->context = NULL;
        rb_grn_table_cursor->cursor = NULL;
        rb_grn_rc_check(rc, self);
    }
    return Qnil;
}

static VALUE
rb_grn_table_cursor_closed_p (VALUE self)
{
    RbGrnTableCursor *rb_grn_table_cursor;

    rb_grn_table_cursor = SELF(self);
    if (rb_grn_table_cursor->context && rb_grn_table_cursor->cursor)
        return Qfalse;
    else
        return Qtrue;
}

static VALUE
rb_grn_table_cursor_get_key (VALUE self)
{
    VALUE rb_key = Qnil;
    RbGrnTableCursor *rb_grn_table_cursor;

    rb_grn_table_cursor = SELF(self);
    if (rb_grn_table_cursor->context && rb_grn_table_cursor->cursor) {
        int n;
        void *key;

        n = grn_table_cursor_get_key(rb_grn_table_cursor->context,
                                     rb_grn_table_cursor->cursor,
                                     &key);
        rb_key = rb_str_new(key, n);
    }

    return rb_key;
}

static VALUE
rb_grn_table_cursor_get_value (VALUE self)
{
    VALUE rb_value = Qnil;
    RbGrnTableCursor *rb_grn_table_cursor;

    rb_grn_table_cursor = SELF(self);
    if (rb_grn_table_cursor->context && rb_grn_table_cursor->cursor) {
        int n;
        void *value;

        n = grn_table_cursor_get_value(rb_grn_table_cursor->context,
                                       rb_grn_table_cursor->cursor,
                                       &value);
        rb_value = rb_str_new(value, n);
    }

    return rb_value;
}

static VALUE
rb_grn_table_cursor_set_value (VALUE self, VALUE value)
{
    RbGrnTableCursor *rb_grn_table_cursor;

    rb_grn_table_cursor = SELF(self);
    if (rb_grn_table_cursor->context && rb_grn_table_cursor->cursor) {
        grn_rc rc;

        rc = grn_table_cursor_set_value(rb_grn_table_cursor->context,
                                        rb_grn_table_cursor->cursor,
                                        StringValuePtr(value), GRN_OBJ_SET);
        rb_grn_rc_check(rc, self);
    }

    return Qnil;
}

static VALUE
rb_grn_table_cursor_delete (VALUE self)
{
    RbGrnTableCursor *rb_grn_table_cursor;

    rb_grn_table_cursor = SELF(self);
    if (rb_grn_table_cursor->context && rb_grn_table_cursor->cursor) {
        grn_rc rc;

        rc = grn_table_cursor_delete(rb_grn_table_cursor->context,
                                     rb_grn_table_cursor->cursor);
        rb_grn_rc_check(rc, self);
    }

    return Qnil;
}

static VALUE
rb_grn_table_cursor_next (VALUE self)
{
    VALUE rb_record = Qnil;
    RbGrnTableCursor *rb_grn_table_cursor;

    rb_grn_table_cursor = SELF(self);
    if (rb_grn_table_cursor->context && rb_grn_table_cursor->cursor) {
        grn_id record_id;

        record_id = grn_table_cursor_next(rb_grn_table_cursor->context,
                                          rb_grn_table_cursor->cursor);
        if (record_id != GRN_ID_NIL)
            rb_record = rb_grn_record_new(rb_iv_get(self, "@table"), record_id);
    }

    return rb_record;
}

static VALUE
rb_grn_table_cursor_each (VALUE self)
{
    grn_id record_id;
    RbGrnTableCursor *rb_grn_table_cursor;
    grn_ctx *context;
    grn_table_cursor *cursor;

    rb_grn_table_cursor = SELF(self);
    if (!rb_grn_table_cursor->context)
        return Qnil;
    if (!rb_grn_table_cursor->cursor)
        return Qnil;

    context = rb_grn_table_cursor->context;
    cursor = rb_grn_table_cursor->cursor;
    while ((record_id = grn_table_cursor_next(context, cursor))) {
        rb_yield(rb_grn_record_new(rb_iv_get(self, "@table"), record_id));
    }

    return Qnil;
}

void
rb_grn_init_table_cursor (VALUE mGrn)
{
    rb_cGrnTableCursor = rb_define_class_under(mGrn, "TableCurosr", rb_cObject);
    rb_define_alloc_func(rb_cGrnTableCursor, rb_grn_table_cursor_alloc);

    rb_include_module(rb_cGrnTableCursor, rb_mEnumerable);

    rb_cGrnHashCursor =
        rb_define_class_under(mGrn, "HashCursor", rb_cGrnTableCursor);
    rb_cGrnPatriciaTrieCursor =
        rb_define_class_under(mGrn, "PatriciaTrieCursor", rb_cGrnTableCursor);
    rb_cGrnArrayCursor =
        rb_define_class_under(mGrn, "ArrayCursor", rb_cGrnTableCursor);

    rb_define_method(rb_cGrnTableCursor, "close",
                     rb_grn_table_cursor_close, 0);
    rb_define_method(rb_cGrnTableCursor, "closed?",
                     rb_grn_table_cursor_closed_p, 0);

    rb_define_method(rb_cGrnTableCursor, "key",
                     rb_grn_table_cursor_get_key, 0);
    rb_define_method(rb_cGrnTableCursor, "value",
                     rb_grn_table_cursor_get_value, 0);
    rb_define_method(rb_cGrnTableCursor, "value=",
                     rb_grn_table_cursor_set_value, 1);
    rb_define_method(rb_cGrnTableCursor, "delete",
                     rb_grn_table_cursor_delete, 0);
    rb_define_method(rb_cGrnTableCursor, "next",
                     rb_grn_table_cursor_next, 0);

    rb_define_method(rb_cGrnTableCursor, "each",
                     rb_grn_table_cursor_each, 0);
}
