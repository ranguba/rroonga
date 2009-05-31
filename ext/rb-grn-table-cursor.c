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

#define SELF(object) ((RbGrnTableCursor *)DATA_PTR(object))

VALUE rb_cGrnTableCursor;

grn_table_cursor *
rb_grn_table_cursor_from_ruby_object (VALUE object, grn_ctx **context)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnTableCursor))) {
	rb_raise(rb_eTypeError, "not a groonga table cursor");
    }

    return RVAL2GRNOBJECT(object, context);
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
				    grn_table_cursor *cursor,
				    rb_grn_boolean owner)
{
    if (NIL_P(klass))
	klass = rb_grn_table_cursor_to_ruby_class(cursor);
    return GRNOBJECT2RVAL(klass, context, cursor, owner);
}

void
rb_grn_table_cursor_unbind (RbGrnTableCursor *rb_grn_table_cursor)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_table_cursor);
    rb_grn_object_unbind(rb_grn_object);
}

static void
rb_grn_table_cursor_free (void *object)
{
    RbGrnTableCursor *rb_grn_table_cursor = object;

    rb_grn_table_cursor_unbind(rb_grn_table_cursor);
    xfree(rb_grn_table_cursor);
}

VALUE
rb_grn_table_cursor_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_table_cursor_free, NULL);
}

void
rb_grn_table_cursor_bind (RbGrnTableCursor *rb_grn_table_cursor,
			  grn_ctx *context, grn_table_cursor *cursor,
			  rb_grn_boolean owner)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_table_cursor);
    rb_grn_object_bind(rb_grn_object, context, cursor, owner);
    rb_grn_object->unbind = RB_GRN_UNBIND_FUNCTION(rb_grn_table_cursor_unbind);
}

void
rb_grn_table_cursor_assign (VALUE self, VALUE rb_context,
			    grn_ctx *context, grn_table_cursor *cursor,
			    rb_grn_boolean owner)
{
    RbGrnTableCursor *rb_grn_table_cursor;

    rb_grn_table_cursor = ALLOC(RbGrnTableCursor);
    DATA_PTR(self) = rb_grn_table_cursor;
    rb_grn_table_cursor_bind(rb_grn_table_cursor, context, cursor, owner);

    rb_iv_set(self, "context", rb_context);
}

void
rb_grn_table_cursor_deconstruct (RbGrnTableCursor *rb_grn_table_cursor,
				 grn_table_cursor **cursor,
				 grn_ctx **context,
				 grn_id *domain_id,
				 grn_obj **domain,
				 grn_id *range_id,
				 grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_table_cursor);
    rb_grn_object_deconstruct(rb_grn_object, cursor, context,
			      domain_id, domain,
			      range_id, range);
}

VALUE
rb_grn_table_cursor_close (VALUE self)
{
    rb_grn_table_cursor_unbind(SELF(self));
    return Qnil;
}

static VALUE
rb_grn_table_cursor_closed_p (VALUE self)
{
    grn_table_cursor *cursor;
    grn_ctx *context;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);

    if (context && cursor)
        return Qfalse;
    else
        return Qtrue;
}

static VALUE
rb_grn_table_cursor_get_value (VALUE self)
{
    grn_ctx *context;
    grn_table_cursor *cursor;
    VALUE rb_value = Qnil;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);
    if (context && cursor) {
        int n;
        void *value;

        n = grn_table_cursor_get_value(context, cursor, &value);
        rb_value = rb_str_new(value, n);
    }

    return rb_value;
}

static VALUE
rb_grn_table_cursor_set_value (VALUE self, VALUE value)
{
    grn_ctx *context;
    grn_table_cursor *cursor;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);
    if (context && cursor) {
        grn_rc rc;

        rc = grn_table_cursor_set_value(context,
                                        cursor,
                                        StringValuePtr(value),
					GRN_OBJ_SET);
        rb_grn_rc_check(rc, self);
    }

    return Qnil;
}

static VALUE
rb_grn_table_cursor_delete (VALUE self)
{
    grn_ctx *context;
    grn_table_cursor *cursor;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);
    if (context && cursor) {
        grn_rc rc;

        rc = grn_table_cursor_delete(context, cursor);
        rb_grn_rc_check(rc, self);
    }

    return Qnil;
}

static VALUE
rb_grn_table_cursor_next (VALUE self)
{
    VALUE rb_record = Qnil;
    grn_ctx *context;
    grn_table_cursor *cursor;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);
    if (context && cursor) {
        grn_id record_id;

        record_id = grn_table_cursor_next(context, cursor);
        if (record_id != GRN_ID_NIL) /* FIXME: use grn_table_cursor_table */
            rb_record = rb_grn_record_new(rb_iv_get(self, "@table"), record_id);
    }

    return rb_record;
}

static VALUE
rb_grn_table_cursor_each (VALUE self)
{
    grn_id record_id;
    grn_ctx *context;
    grn_table_cursor *cursor;

    rb_grn_table_cursor_deconstruct(SELF(self), &cursor, &context,
				    NULL, NULL, NULL, NULL);

    if (context && cursor) {
	while ((record_id = grn_table_cursor_next(context, cursor))) {
	    rb_yield(rb_grn_record_new(rb_iv_get(self, "@table"), record_id));
	}
    }

    return Qnil;
}

void
rb_grn_init_table_cursor (VALUE mGrn)
{
    rb_cGrnTableCursor = rb_define_class_under(mGrn, "TableCurosr", rb_cObject);
    rb_define_alloc_func(rb_cGrnTableCursor, rb_grn_table_cursor_alloc);

    rb_include_module(rb_cGrnTableCursor, rb_mEnumerable);

    rb_define_method(rb_cGrnTableCursor, "close",
                     rb_grn_table_cursor_close, 0);
    rb_define_method(rb_cGrnTableCursor, "closed?",
                     rb_grn_table_cursor_closed_p, 0);

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

    rb_grn_init_table_cursor_key_support(mGrn);
    rb_grn_init_array_cursor(mGrn);
    rb_grn_init_hash_cursor(mGrn);
    rb_grn_init_patricia_trie_cursor(mGrn);
}
