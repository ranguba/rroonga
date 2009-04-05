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

#define SELF(object) (RVAL2GRNDB(object))

static VALUE cGrnDatabase;

typedef struct _RbGrnObject RbGrnObject;
struct _RbGrnObject
{
    grn_ctx *context;
    grn_obj *object;
};

grn_obj *
rb_grn_database_from_ruby_object (VALUE object)
{
    RbGrnObject *grn_object;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, cGrnDatabase))) {
	rb_raise(rb_eTypeError, "not a groonga database");
    }

    Data_Get_Struct(object, RbGrnObject, grn_object);
    if (!grn_object)
	rb_raise(rb_eGrnError, "groonga database is NULL");
    return grn_object->object;
}

static void
rb_grn_database_free (void *object)
{
    xfree(object);
}

VALUE
rb_grn_database_to_ruby_object (grn_ctx *context, grn_obj *database)
{
    RbGrnObject *grn_object;

    if (!database)
        return Qnil;

    grn_object = ALLOC(RbGrnObject);
    grn_object->context = context;
    grn_object->object = database;

    return Data_Wrap_Struct(cGrnDatabase, NULL,
                            rb_grn_database_free, grn_object);
}

static VALUE
rb_grn_database_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_database_free, NULL);
}

static VALUE
rb_grn_database_s_create (VALUE argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context;
    grn_db_create_optarg create_args;
    RbGrnObject *object;
    const char *path;
    VALUE self;
    VALUE rb_path, options, rb_context, encoding, builtin_type_names;

    rb_scan_args(argc, argv, "11", &rb_path, &options);

    path = StringValuePtr(rb_path);
    rb_grn_scan_options(options,
			"context", &rb_context,
                        "encoding", &encoding,
                        "builtin_type_names", &builtin_type_names,
			NULL);

    context = rb_grn_context_ensure(rb_context);

    create_args.encoding = RVAL2GRNENCODING(encoding);
    create_args.builtin_type_names = NULL;
    create_args.n_builtin_type_names = 0;

    self = rb_grn_database_alloc(klass);
    object = ALLOC(RbGrnObject);
    DATA_PTR(self) = object;
    object->context = context;
    object->object = grn_db_create(context, path, &create_args);
    rb_grn_context_check(context);

    return self;
}

static VALUE
rb_grn_database_initialize (VALUE argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    RbGrnObject *object;
    const char *path;
    VALUE rb_path, options, rb_context;

    rb_scan_args(argc, argv, "11", &rb_path, &options);

    path = StringValuePtr(rb_path);
    rb_grn_scan_options(options,
			"context", &rb_context,
			NULL);

    context = rb_grn_context_ensure(rb_context);

    object = ALLOC(RbGrnObject);
    DATA_PTR(self) = object;
    object->context = context;
    object->object = grn_db_open(context, path);
    rb_grn_context_check(context);

    return Qnil;
}

static VALUE
rb_grn_database_get_id (VALUE self)
{
    return UINT2NUM(SELF(self)->header.domain);
}

static VALUE
rb_grn_database_eql_p (VALUE self, VALUE other)
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
rb_grn_database_hash (VALUE self)
{
    return rb_funcall(rb_grn_database_get_id(self), rb_intern("hash"), 0);
}

void
rb_grn_init_database (VALUE mGroonga)
{
    cGrnDatabase = rb_define_class_under(mGroonga, "Database", rb_cObject);
    rb_define_alloc_func(cGrnDatabase, rb_grn_database_alloc);

    rb_define_singleton_method(cGrnDatabase, "create",
			       rb_grn_database_s_create, -1);

    rb_define_method(cGrnDatabase, "initialize", rb_grn_database_initialize, -1);

    rb_define_method(cGrnDatabase, "id", rb_grn_database_get_id, 0);

    rb_define_method(cGrnDatabase, "eql?", rb_grn_database_eql_p, 1);
    rb_define_method(cGrnDatabase, "hash", rb_grn_database_hash, 0);
    rb_define_alias(cGrnDatabase, "==", "eql?");
}
