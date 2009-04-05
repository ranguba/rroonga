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

#define SELF(object) (RVAL2GRNDB(object))

VALUE rb_cGrnDatabase;

grn_obj *
rb_grn_database_from_ruby_object (VALUE object)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnDatabase))) {
	rb_raise(rb_eTypeError, "not a groonga database");
    }

    return RVAL2GRNOBJECT(object);
}

VALUE
rb_grn_database_to_ruby_object (grn_ctx *context, grn_obj *database)
{
    return GRNOBJECT2RVAL(rb_cGrnDatabase, context, database);
}

static VALUE
rb_grn_database_s_create (VALUE argc, VALUE *argv, VALUE klass)
{
    grn_ctx *context;
    grn_obj *database;
    grn_db_create_optarg create_args;
    const char *path = NULL;
    VALUE rb_database;
    VALUE rb_path, options, rb_context, encoding, builtin_type_names;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
                        "path", &rb_path,
			"context", &rb_context,
                        "encoding", &encoding,
                        "builtin_type_names", &builtin_type_names,
			NULL);

    if (!NIL_P(rb_path))
        path = StringValuePtr(rb_path);
    context = rb_grn_context_ensure(rb_context);

    create_args.encoding = RVAL2GRNENCODING(encoding);
    create_args.builtin_type_names = NULL;
    create_args.n_builtin_type_names = 0;

    database = grn_db_create(context, path, &create_args);
    rb_database = GRNOBJECT2RVAL(klass, context, database);
    rb_grn_context_check(context);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_database,
			 rb_grn_object_close, rb_database);
    else
        return rb_database;
}

static VALUE
rb_grn_database_initialize (VALUE argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *database;
    const char *path;
    VALUE rb_path, options, rb_context;

    rb_scan_args(argc, argv, "11", &rb_path, &options);

    path = StringValuePtr(rb_path);
    rb_grn_scan_options(options,
			"context", &rb_context,
			NULL);

    context = rb_grn_context_ensure(rb_context);

    database = grn_db_open(context, path);
    rb_grn_object_initialize(self, context, database);
    rb_grn_context_check(context);

    return Qnil;
}

static VALUE
rb_grn_database_s_open (VALUE argc, VALUE *argv, VALUE klass)
{
    VALUE database;

    database = rb_grn_object_alloc(klass);
    rb_grn_database_initialize(argc, argv, database);
    if (rb_block_given_p())
        return rb_ensure(rb_yield, database, rb_grn_object_close, database);
    else
        return database;
}

void
rb_grn_init_database (VALUE mGrn)
{
    rb_cGrnDatabase = rb_define_class_under(mGrn, "Database", rb_cGrnObject);

    rb_define_singleton_method(rb_cGrnDatabase, "create",
			       rb_grn_database_s_create, -1);
    rb_define_singleton_method(rb_cGrnDatabase, "open",
			       rb_grn_database_s_open, -1);

    rb_define_method(rb_cGrnDatabase, "initialize",
		     rb_grn_database_initialize, -1);
}
