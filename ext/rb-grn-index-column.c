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

#define SELF(object) ((RbGrnIndexColumn *)DATA_PTR(object))

VALUE rb_cGrnIndexColumn;

void
rb_grn_index_column_unbind (RbGrnIndexColumn *rb_grn_index_column)
{
    RbGrnObject *rb_grn_object;
    grn_ctx *context;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_index_column);
    context = rb_grn_object->context;

    if (context && rb_grn_context_alive_p(context)) {
	grn_obj_close(context, rb_grn_index_column->value);
	grn_obj_close(context, rb_grn_index_column->old_value);
    }

    rb_grn_object_unbind(rb_grn_object);
}

static void
rb_grn_index_column_free (void *object)
{
    RbGrnIndexColumn *rb_grn_index_column = object;

    rb_grn_index_column_unbind(rb_grn_index_column);
    xfree(rb_grn_index_column);
}

VALUE
rb_grn_index_column_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_index_column_free, NULL);
}

void
rb_grn_index_column_bind (RbGrnIndexColumn *rb_grn_index_column,
			  grn_ctx *context, grn_obj *column,
			  rb_grn_boolean owner)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_index_column);
    rb_grn_object_bind(rb_grn_object, context, column, owner);

    rb_grn_index_column->value = grn_obj_open(context, GRN_BULK, 0,
					      rb_grn_object->range_id);
    rb_grn_index_column->old_value = grn_obj_open(context, GRN_BULK, 0,
						  rb_grn_object->range_id);
}

void
rb_grn_index_column_assign (VALUE self, VALUE rb_context,
			    grn_ctx *context, grn_obj *column,
			    rb_grn_boolean owner)
{
    RbGrnIndexColumn *rb_grn_index_column;

    rb_grn_index_column = ALLOC(RbGrnIndexColumn);
    DATA_PTR(self) = rb_grn_index_column;
    rb_grn_index_column_bind(rb_grn_index_column, context, column, owner);

    rb_iv_set(self, "context", rb_context);
}

void
rb_grn_index_column_deconstruct (RbGrnIndexColumn *rb_grn_index_column,
				 grn_obj **column,
				 grn_ctx **context,
				 grn_id *domain_id,
				 grn_obj **domain,
				 grn_obj **value,
				 grn_obj **old_value,
				 grn_id *range_id,
				 grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_index_column);
    rb_grn_object_deconstruct(rb_grn_object, column, context,
			      domain_id, domain,
			      range_id, range);

    if (value)
	*value = rb_grn_index_column->value;
    if (old_value)
	*old_value = rb_grn_index_column->old_value;
}

static VALUE
rb_grn_index_column_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_rc rc;
    grn_id id;
    unsigned int section;
    grn_obj *old_value, *new_value;
    VALUE original_rb_value, rb_section, rb_old_value, rb_new_value;

    original_rb_value = rb_value;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
				    NULL, NULL,
				    &new_value, &old_value,
				    NULL, NULL);

    id = NUM2UINT(rb_id);

    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_value, rb_cHash))) {
	VALUE hash_value;
	hash_value = rb_hash_new();
	rb_hash_aset(hash_value, RB_GRN_INTERN("value"), rb_value);
	rb_value = hash_value;
    }

    rb_grn_scan_options(rb_value,
			"section", &rb_section,
			"old_value", &rb_old_value,
			"value", &rb_new_value,
			NULL);

    if (NIL_P(rb_section))
	section = 1;
    else
	section = NUM2UINT(rb_section);

    if (NIL_P(rb_old_value)) {
	old_value = NULL;
    } else {
	GRN_BULK_REWIND(old_value);
	RVAL2GRNBULK(rb_old_value, context, old_value);
    }

    if (NIL_P(rb_new_value)) {
	new_value = NULL;
    } else {
	GRN_BULK_REWIND(new_value);
	RVAL2GRNBULK(rb_new_value, context, new_value);
    }

    rc = grn_column_index_update(context, column,
				 id, section, old_value, new_value);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return original_rb_value;
}

static VALUE
rb_grn_index_column_get_sources (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj sources;
    grn_id *source_ids;
    VALUE rb_sources;
    int i, n;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
				    NULL, NULL,
				    NULL, NULL, NULL, NULL);

    GRN_OBJ_INIT(&sources, GRN_BULK, 0, GRN_ID_NIL);
    grn_obj_get_info(context, column, GRN_INFO_SOURCE, &sources);
    rb_grn_context_check(context, self);

    n = GRN_BULK_VSIZE(&sources) / sizeof(grn_id);
    source_ids = (grn_id *)GRN_BULK_HEAD(&sources);
    rb_sources = rb_ary_new2(n);
    for (i = 0; i < n; i++) {
	grn_obj *source;
	VALUE rb_source;

	source = grn_ctx_at(context, *source_ids);
	rb_source = GRNOBJECT2RVAL(Qnil, context, source, RB_GRN_FALSE);
	rb_ary_push(rb_sources, rb_source);
	source_ids++;
    }
    grn_obj_close(context, &sources);

    return rb_sources;
}

static VALUE
rb_grn_index_column_set_sources (VALUE self, VALUE rb_sources)
{
    VALUE exception;
    grn_ctx *context = NULL;
    grn_obj *column;
    int i, n;
    VALUE *rb_source_values;
    grn_id *sources;
    grn_rc rc;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
				    NULL, NULL,
				    NULL, NULL, NULL, NULL);

    n = RARRAY_LEN(rb_sources);
    rb_source_values = RARRAY_PTR(rb_sources);
    sources = ALLOCA_N(grn_id, n);
    for (i = 0; i < n; i++) {
	VALUE rb_source_id;
	grn_obj *source;
	grn_id source_id;

	rb_source_id = rb_source_values[i];
	if (CBOOL2RVAL(rb_obj_is_kind_of(rb_source_id, rb_cInteger))) {
	    source_id = NUM2UINT(rb_source_id);
	} else {
	    source = RVAL2GRNOBJECT(rb_source_id, &context);
	    rb_grn_context_check(context, rb_source_id);
	    source_id = grn_obj_id(context, source);
	}
	sources[i] = source_id;
    }

    {
	grn_obj bulk_sources;
	GRN_OBJ_INIT(&bulk_sources, GRN_BULK,
		     GRN_OBJ_DO_SHALLOW_COPY, GRN_ID_NIL);
	GRN_TEXT_SET(context, &bulk_sources, sources, n * sizeof(grn_id));
	rc = grn_obj_set_info(context, column, GRN_INFO_SOURCE, &bulk_sources);
	exception = rb_grn_context_to_exception(context, self);
	grn_obj_close(context, &bulk_sources);
    }

    if (!NIL_P(exception))
	rb_exc_raise(exception);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

static VALUE
rb_grn_index_column_set_source (VALUE self, VALUE rb_source)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_source, rb_cArray)))
	rb_source = rb_ary_new3(1, rb_source);

    return rb_grn_index_column_set_sources(self, rb_source);
}

void
rb_grn_init_index_column (VALUE mGrn)
{
    rb_cGrnIndexColumn =
	rb_define_class_under(mGrn, "IndexColumn", rb_cGrnColumn);
    rb_define_alloc_func(rb_cGrnIndexColumn, rb_grn_index_column_alloc);

    rb_define_method(rb_cGrnIndexColumn, "[]=",
		     rb_grn_index_column_array_set, 2);

    rb_define_method(rb_cGrnIndexColumn, "sources",
		     rb_grn_index_column_get_sources, 0);
    rb_define_method(rb_cGrnIndexColumn, "sources=",
		     rb_grn_index_column_set_sources, 1);
    rb_define_method(rb_cGrnIndexColumn, "source=",
		     rb_grn_index_column_set_source, 1);

    rb_grn_init_fix_size_column(mGrn);
}
