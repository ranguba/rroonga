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

#define SELF(object) (RVAL2GRNOBJECT(object))

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
rb_grn_object_from_ruby_object (VALUE object)
{
    if (NIL_P(object))
        return NULL;

    return rb_grn_object_wrapper_from_ruby_object(object)->object;
}

static void
rb_grn_object_free (void *object)
{
    xfree(object);
}

static VALUE
guess_object_class (grn_obj *object)
{
    VALUE klass = Qnil;

    switch (object->header.type) {
      case GRN_DB:
	klass = rb_cGrnDatabase;
	break;
      case GRN_TABLE_HASH_KEY:
      case GRN_TABLE_PAT_KEY:
      case GRN_TABLE_NO_KEY:
	klass = rb_cGrnTable;
	break;
      case GRN_TYPE:
	klass = rb_cGrnType;
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
        klass = guess_object_class(object);

    return Data_Wrap_Struct(klass, NULL, rb_grn_object_free, grn_object);
}

VALUE
rb_grn_object_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_object_free, NULL);
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

    grn_object = rb_grn_object_wrapper_from_ruby_object(self);
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

    grn_object = rb_grn_object_wrapper_from_ruby_object(self);
    if (grn_object->context && grn_object->object)
        return Qfalse;
    else
        return Qtrue;
}

static VALUE
rb_grn_object_get_id (VALUE self)
{
    RbGrnObject *grn_object;
    unsigned id;

    grn_object = rb_grn_object_wrapper_from_ruby_object(self);
    if (grn_object->object)
        id = grn_object->object->header.domain;
    else
        id = 0;
    return UINT2NUM(id);
}

static VALUE
rb_grn_object_get_name (VALUE self)
{
    RbGrnObject *grn_object;
    VALUE rb_name;
    int name_size;

    grn_object = rb_grn_object_wrapper_from_ruby_object(self);
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
rb_grn_object_eql_p (VALUE self, VALUE other)
{
    RbGrnObject *self_grn_object, *other_grn_object;

    if (self == other)
        return Qtrue;

    if (!RVAL2CBOOL(rb_funcall(rb_obj_class(self), rb_intern("=="), 1,
                               rb_obj_class(other))))
        return Qfalse;

    self_grn_object = rb_grn_object_wrapper_from_ruby_object(self);
    other_grn_object = rb_grn_object_wrapper_from_ruby_object(other);

    if (self_grn_object->object == other_grn_object->object)
        return Qtrue;
    if (!self_grn_object->object || !other_grn_object->object)
        return Qfalse;
    return CBOOL2RVAL(self_grn_object->object->header.domain ==
                      other_grn_object->object->header.domain);
}

static VALUE
rb_grn_object_hash (VALUE self)
{
    return rb_funcall(rb_grn_object_get_id(self), rb_intern("hash"), 0);
}

void
rb_grn_init_object (VALUE mGrn)
{
    rb_cGrnObject = rb_define_class_under(mGrn, "Object", rb_cObject);
    rb_define_alloc_func(rb_cGrnObject, rb_grn_object_alloc);

    rb_define_method(rb_cGrnObject, "id", rb_grn_object_get_id, 0);
    rb_define_method(rb_cGrnObject, "name", rb_grn_object_get_name, 0);

    rb_define_method(rb_cGrnObject, "eql?", rb_grn_object_eql_p, 1);
    rb_define_method(rb_cGrnObject, "hash", rb_grn_object_hash, 0);
    rb_define_alias(rb_cGrnObject, "==", "eql?");

    rb_define_method(rb_cGrnObject, "close", rb_grn_object_close, 0);
    rb_define_method(rb_cGrnObject, "closed?", rb_grn_object_closed_p, 0);
}
