/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-grn.h"

#define SELF(object) (rb_grn_object_wrapper_from_ruby_object(object))

typedef struct _RbGrnObject RbGrnObject;
struct _RbGrnObject
{
    grn_ctx *context;
    grn_obj *object;
};

VALUE rb_cGrnObject;

static RbGrnObject *
rb_grn_object_wrapper_from_ruby_object (VALUE object)
{
    RbGrnObject *grn_object;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnObject))) {
	rb_raise(rb_eTypeError, "not a groonga object");
    }

    Data_Get_Struct(object, RbGrnObject, grn_object);
    if (!grn_object)
	rb_raise(rb_eGrnError, "groonga object is NULL");

    return grn_object;
}

grn_obj *
rb_grn_object_from_ruby_object (VALUE object, grn_ctx *context)
{
    if (NIL_P(object))
        return NULL;

    if (context) {
	grn_obj *grn_object;
	if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cString))) {
	    grn_object = grn_ctx_lookup(context,
					StringValuePtr(object),
					RSTRING_LEN(object));
	    if (!grn_object)
		rb_raise(rb_eArgError,
			 "unregistered groonga object: name: <%s>",
			 rb_grn_inspect(object));
	    return grn_object;
	} else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cInteger))) {
	    grn_object = grn_ctx_get(context, NUM2UINT(object));
	    if (!grn_object)
		rb_raise(rb_eArgError,
			 "unregistered groonga object: ID: <%s>",
			 rb_grn_inspect(object));
	    return grn_object;
	}
    }

    return rb_grn_object_wrapper_from_ruby_object(object)->object;
}

static void
rb_grn_object_free (void *object)
{
    xfree(object);
}

VALUE
rb_grn_object_to_ruby_class (grn_obj *object)
{
    VALUE klass = Qnil;

    switch (object->header.type) {
      case GRN_DB:
	klass = rb_cGrnDatabase;
	break;
      case GRN_TABLE_HASH_KEY:
	klass = rb_cGrnHash;
	break;
      case GRN_TABLE_PAT_KEY:
	klass = rb_cGrnPatriciaTrie;
	break;
      case GRN_TABLE_NO_KEY:
	klass = rb_cGrnArray;
	break;
      case GRN_TYPE:
	klass = rb_cGrnType;
	break;
      case GRN_PROC:
	klass = rb_cGrnProcedure;
	break;
      case GRN_COLUMN_FIX_SIZE:
	klass = rb_cGrnFixSizeColumn;
	break;
      case GRN_COLUMN_VAR_SIZE:
	klass = rb_cGrnVarSizeColumn;
	break;
      case GRN_COLUMN_INDEX:
	klass = rb_cGrnIndexColumn;
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
rb_grn_object_to_ruby_object (VALUE klass, grn_ctx *context, grn_obj *object)
{
    RbGrnObject *grn_object;

    if (!object)
        return Qnil;

    grn_object = ALLOC(RbGrnObject);
    grn_object->context = context;
    grn_object->object = object;

    if (NIL_P(klass))
        klass = GRNOBJECT2RCLASS(object);

    return Data_Wrap_Struct(klass, NULL, rb_grn_object_free, grn_object);
}

VALUE
rb_grn_object_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_object_free, NULL);
}

grn_ctx *
rb_grn_object_ensure_context (VALUE object, VALUE rb_context)
{
    if (NIL_P(rb_context)) {
	RbGrnObject *grn_object;

	grn_object = SELF(object);
	if (grn_object && grn_object->context)
	    return grn_object->context;
    }

    return rb_grn_context_ensure(rb_context);
}

void
rb_grn_object_initialize (VALUE self, grn_ctx *context, grn_obj *object)
{
    RbGrnObject *grn_object;

    grn_object = ALLOC(RbGrnObject);
    DATA_PTR(self) = grn_object;
    grn_object->context = context;
    grn_object->object = object;
}

VALUE
rb_grn_object_close (VALUE self)
{
    RbGrnObject *grn_object;

    grn_object = SELF(self);
    if (grn_object->context && grn_object->object) {
        GRN_OBJ_FIN(grn_object->context, grn_object->object);
        grn_object->context = NULL;
        grn_object->object = NULL;
    }
    return Qnil;
}

static VALUE
rb_grn_object_closed_p (VALUE self)
{
    RbGrnObject *grn_object;

    grn_object = SELF(self);
    if (grn_object->context && grn_object->object)
        return Qfalse;
    else
        return Qtrue;
}

static VALUE
rb_grn_object_get_domain (VALUE self)
{
    RbGrnObject *grn_object;

    grn_object = SELF(self);
    if (grn_object->object)
        return UINT2NUM(grn_object->object->header.domain);
    else
        return Qnil;
}

static VALUE
rb_grn_object_get_name (VALUE self)
{
    RbGrnObject *grn_object;
    VALUE rb_name;
    int name_size;

    grn_object = SELF(self);
    if (!grn_object->object)
	return Qnil;

    name_size = grn_obj_name(grn_object->context, grn_object->object, NULL, 0);
    if (name_size == 0)
	return Qnil;

    rb_name = rb_str_buf_new(name_size);
    RSTRING_LEN(rb_name) = name_size;
    grn_obj_name(grn_object->context, grn_object->object,
		 RSTRING_PTR(rb_name), name_size);
    return rb_name;
}

static VALUE
rb_grn_object_equal (VALUE self, VALUE other)
{
    RbGrnObject *self_grn_object, *other_grn_object;

    if (self == other)
        return Qtrue;

    if (!RVAL2CBOOL(rb_funcall(rb_obj_class(self), rb_intern("=="), 1,
                               rb_obj_class(other))))
        return Qfalse;

    self_grn_object = SELF(self);
    other_grn_object = SELF(other);

    return self_grn_object->object == other_grn_object->object;
}

static VALUE
rb_grn_object_array_reference (VALUE self, VALUE rb_id)
{
    RbGrnObject *grn_object;
    grn_id id;
    grn_obj *value;
    VALUE rb_value = Qnil;

    grn_object = SELF(self);
    if (!grn_object->object)
	return Qnil;

    id = NUM2UINT(rb_id);
    value = grn_obj_get_value(grn_object->context, grn_object->object, id, NULL);
    if (!value) {
	rb_grn_context_check(grn_object->context);
	return Qnil;
    }

    if (!GRN_BULK_EMPTYP(value)) {
	rb_value = rb_str_new(GRN_BULK_HEAD(value), GRN_BULK_VSIZE(value));
	grn_obj_close(grn_object->context, value);
    }
    rb_grn_context_check(grn_object->context);

    return rb_value;
}

static VALUE
rb_grn_object_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    RbGrnObject *grn_object;
    grn_id id;
    grn_obj *value;
    grn_rc rc;

    grn_object = SELF(self);
    if (!grn_object->object)
	return Qnil;

    id = NUM2UINT(rb_id);
    value = grn_obj_open(grn_object->context, GRN_BULK, 0, 0);
    rb_grn_context_check(grn_object->context);
    rc = grn_bulk_write(grn_object->context, value,
			StringValuePtr(rb_value), RSTRING_LEN(rb_value));
    if (rc != GRN_SUCCESS) {
	grn_obj_close(grn_object->context, value);
	rb_grn_check_rc(rc);
    }
    rc = grn_obj_set_value(grn_object->context, grn_object->object, id,
			   value, GRN_OBJ_SET);
    grn_obj_close(grn_object->context, value);
    rb_grn_check_rc(rc);

    return Qnil;
}

void
rb_grn_init_object (VALUE mGrn)
{
    rb_cGrnObject = rb_define_class_under(mGrn, "Object", rb_cObject);
    rb_define_alloc_func(rb_cGrnObject, rb_grn_object_alloc);

    rb_define_method(rb_cGrnObject, "domain", rb_grn_object_get_domain, 0);
    rb_define_method(rb_cGrnObject, "name", rb_grn_object_get_name, 0);

    rb_define_method(rb_cGrnObject, "==", rb_grn_object_equal, 1);

    rb_define_method(rb_cGrnObject, "close", rb_grn_object_close, 0);
    rb_define_method(rb_cGrnObject, "closed?", rb_grn_object_closed_p, 0);

    rb_define_method(rb_cGrnObject, "[]", rb_grn_object_array_reference, 1);
    rb_define_method(rb_cGrnObject, "[]=", rb_grn_object_array_set, 2);
}
