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

VALUE
rb_grn_object_to_ruby_object (VALUE klass, grn_ctx *context, grn_obj *object)
{
    RbGrnObject *grn_object;

    if (!object)
        return Qnil;

    grn_object = ALLOC(RbGrnObject);
    grn_object->context = context;
    grn_object->object = object;

    return Data_Wrap_Struct(klass, NULL, rb_grn_object_free, grn_object);
}

VALUE
rb_grn_object_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_object_free, NULL);
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
    return UINT2NUM(SELF(self)->header.domain);
}

static VALUE
rb_grn_object_eql_p (VALUE self, VALUE other)
{
    if (self == other)
        return Qtrue;

    if (!RVAL2CBOOL(rb_funcall(rb_obj_class(self), rb_intern("=="), 1,
                               rb_obj_class(other))))
        return Qfalse;

    return CBOOL2RVAL(SELF(self)->header.domain ==
                      SELF(other)->header.domain);
}

static VALUE
rb_grn_object_hash (VALUE self)
{
    return rb_funcall(rb_grn_object_get_id(self), rb_intern("hash"), 0);
}

void
rb_grn_init_object (VALUE mGroonga)
{
    rb_cGrnObject = rb_define_class_under(mGroonga, "Object", rb_cObject);
    rb_define_alloc_func(rb_cGrnObject, rb_grn_object_alloc);

    rb_define_method(rb_cGrnObject, "id", rb_grn_object_get_id, 0);

    rb_define_method(rb_cGrnObject, "eql?", rb_grn_object_eql_p, 1);
    rb_define_method(rb_cGrnObject, "hash", rb_grn_object_hash, 0);
    rb_define_alias(rb_cGrnObject, "==", "eql?");

    rb_define_method(rb_cGrnObject, "close", rb_grn_object_close, 0);
    rb_define_method(rb_cGrnObject, "closed?", rb_grn_object_closed_p, 0);
}
