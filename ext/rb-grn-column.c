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

#define SELF(object, context) (RVAL2GRNCOLUMN(object, context))

VALUE rb_cGrnColumn;
VALUE rb_cGrnFixSizeColumn;
VALUE rb_cGrnVarSizeColumn;
VALUE rb_cGrnIndexColumn;

grn_obj *
rb_grn_column_from_ruby_object (VALUE object, grn_ctx **context)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnColumn))) {
	rb_raise(rb_eTypeError, "not a groonga column");
    }

    return RVAL2GRNOBJECT(object, context);
}

VALUE
rb_grn_column_to_ruby_object (VALUE klass, grn_ctx *context, grn_obj *column,
			      rb_grn_boolean owner)
{
    return GRNOBJECT2RVAL(klass, context, column, owner);
}

static VALUE
rb_grn_fix_size_column_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_id range;
    grn_obj *range_object = NULL;
    grn_rc rc;
    grn_id id;
    grn_obj value;

    column = SELF(self, &context);
    id = NUM2UINT(rb_id);

    range = grn_obj_get_range(context, column);
    if (range != GRN_ID_NIL)
	range_object = grn_ctx_get(context, range);

    if (RVAL2CBOOL(rb_obj_is_kind_of(rb_value, rb_cGrnRecord))) {
	VALUE rb_id, rb_table;
	grn_obj *table;

	if (!range_object)
	    rb_raise(rb_eArgError,
		     "%s isn't associated with any table: %s",
		     rb_grn_inspect(self), rb_grn_inspect(rb_value));

	rb_id = rb_funcall(rb_value, rb_intern("id"), 0);
	rb_table = rb_funcall(rb_value, rb_intern("table"), 0);
	table = RVAL2GRNTABLE(rb_table, &context);
	if (grn_obj_id(context, table) != range)
	    rb_raise(rb_eArgError,
		     "%s isn't associated with passed record's table: %s",
		     rb_grn_inspect(self),
		     rb_grn_inspect(rb_value));

	RVAL2GRNUVECTOR(rb_ary_new3(1, rb_id), context, &value);
    } else if (range_object &&
	       RVAL2CBOOL(rb_obj_is_kind_of(rb_value, rb_cInteger))) {
	switch (range_object->header.type) {
	  case GRN_TABLE_PAT_KEY:
	  case GRN_TABLE_HASH_KEY:
	  case GRN_TABLE_NO_KEY:
	    RVAL2GRNUVECTOR(rb_ary_new3(1, rb_value), context, &value);
	    break;
	  default:
	    RVAL2GRNBULK(rb_value, context, &value);
	    break;
	}
    } else {
	RVAL2GRNBULK(rb_value, context, &value);
    }

    rc = grn_obj_set_value(context, column, id, &value, GRN_OBJ_SET);
    grn_obj_close(context, &value);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

static VALUE
rb_grn_index_column_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_rc rc;
    grn_id id;
    unsigned int section;
    grn_obj old_value, new_value;
    VALUE rb_section, rb_old_value, rb_new_value;

    column = SELF(self, &context);

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

    RVAL2GRNBULK(rb_old_value, context, &old_value);
    RVAL2GRNBULK(rb_new_value, context, &new_value);

    rc = grn_column_index_update(context, column,
				 id, section, &old_value, &new_value);
    grn_obj_close(context, &old_value);
    grn_obj_close(context, &new_value);

    rb_grn_rc_check(rc, self);

    return Qnil;
}

static VALUE
rb_grn_column_get_table (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj *table;

    column = SELF(self, &context);
    table = grn_column_table(context, column);
    rb_grn_context_check(context, self);

    return GRNOBJECT2RVAL(Qnil, context, table, RB_GRN_FALSE);
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

    column = SELF(self, &context);

    grn_obj_get_info(context, column, GRN_INFO_SOURCE, &sources);
    rb_grn_context_check(context, self);

    n = GRN_BULK_VSIZE(&sources) / sizeof(grn_id);
    source_ids = (grn_id *)GRN_BULK_HEAD(&sources);
    rb_sources = rb_ary_new2(n);
    for (i = 0; i < n; i++) {
	grn_obj *source;
	VALUE rb_source;

	source = grn_ctx_get(context, *source_ids);
	rb_source = GRNOBJECT2RVAL(Qnil, context, source, RB_GRN_FALSE);
	rb_ary_push(rb_sources, rb_source);
	source_ids++;
    }

    return rb_sources;
}

static VALUE
rb_grn_index_column_set_sources (VALUE self, VALUE rb_sources)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    int i, n;
    VALUE *rb_source_values;
    grn_id *sources;
    grn_rc rc;

    column = SELF(self, &context);

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
	grn_obj source;
	GRN_OBJ_INIT(&source, GRN_BULK, GRN_OBJ_DO_SHALLOW_COPY);
	GRN_BULK_SET(context, &source, sources, n * sizeof(grn_id));
	rc = grn_obj_set_info(context, column, GRN_INFO_SOURCE, &source);
    }

    rb_grn_context_check(context, self);
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
rb_grn_init_column (VALUE mGrn)
{
    rb_cGrnColumn = rb_define_class_under(mGrn, "Column", rb_cGrnObject);
    rb_cGrnFixSizeColumn =
	rb_define_class_under(mGrn, "FixSizeColumn", rb_cGrnColumn);
    rb_cGrnVarSizeColumn =
	rb_define_class_under(mGrn, "VarSizeColumn", rb_cGrnColumn);
    rb_cGrnIndexColumn =
	rb_define_class_under(mGrn, "IndexColumn", rb_cGrnColumn);

    rb_define_method(rb_cGrnFixSizeColumn, "[]=",
		     rb_grn_fix_size_column_array_set, 2);
    rb_define_method(rb_cGrnIndexColumn, "[]=",
		     rb_grn_index_column_array_set, 2);

    rb_define_method(rb_cGrnColumn, "table", rb_grn_column_get_table, 0);

    rb_define_method(rb_cGrnIndexColumn, "sources",
		     rb_grn_index_column_get_sources, 0);
    rb_define_method(rb_cGrnIndexColumn, "sources=",
		     rb_grn_index_column_set_sources, 1);
    rb_define_method(rb_cGrnIndexColumn, "source=",
		     rb_grn_index_column_set_source, 1);
}
