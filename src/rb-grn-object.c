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

#define SELF(object) (rb_rb_grn_object_from_ruby_object(object))

VALUE rb_cGrnObject;

typedef struct _RbGrnObject RbGrnObject;
struct _RbGrnObject
{
    grn_ctx *context;
    grn_obj *object;
};

static RbGrnObject *
rb_rb_grn_object_from_ruby_object (VALUE object)
{
    RbGrnObject *rb_grn_object;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnObject))) {
	rb_raise(rb_eTypeError, "not a groonga object");
    }

    Data_Get_Struct(object, RbGrnObject, rb_grn_object);
    if (!rb_grn_object)
	rb_raise(rb_eGrnError, "groonga object is NULL");

    return rb_grn_object;
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

    return rb_rb_grn_object_from_ruby_object(object)->object;
}

static void
rb_rb_grn_object_free (void *object)
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
    RbGrnObject *rb_grn_object;

    if (!object)
        return Qnil;

    rb_grn_object = ALLOC(RbGrnObject);
    rb_grn_object->context = context;
    rb_grn_object->object = object;

    if (NIL_P(klass))
        klass = GRNOBJECT2RCLASS(object);

    return Data_Wrap_Struct(klass, NULL, rb_rb_grn_object_free, rb_grn_object);
}

VALUE
rb_grn_object_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_rb_grn_object_free, NULL);
}

grn_ctx *
rb_grn_object_ensure_context (VALUE object, VALUE rb_context)
{
    if (NIL_P(rb_context)) {
	RbGrnObject *rb_grn_object;

	rb_grn_object = SELF(object);
	if (rb_grn_object && rb_grn_object->context)
	    return rb_grn_object->context;
    }

    return rb_grn_context_ensure(rb_context);
}

void
rb_grn_object_initialize (VALUE self, grn_ctx *context, grn_obj *object)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = ALLOC(RbGrnObject);
    DATA_PTR(self) = rb_grn_object;
    rb_grn_object->context = context;
    rb_grn_object->object = object;
}

VALUE
rb_grn_object_close (VALUE self)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = SELF(self);
    if (rb_grn_object->context && rb_grn_object->object) {
        GRN_OBJ_FIN(rb_grn_object->context, rb_grn_object->object);
        rb_grn_object->context = NULL;
        rb_grn_object->object = NULL;
    }
    return Qnil;
}

static VALUE
rb_grn_object_closed_p (VALUE self)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = SELF(self);
    if (rb_grn_object->context && rb_grn_object->object)
        return Qfalse;
    else
        return Qtrue;
}

static VALUE
rb_grn_object_get_domain (VALUE self)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = SELF(self);
    if (rb_grn_object->object)
        return UINT2NUM(rb_grn_object->object->header.domain);
    else
        return Qnil;
}

static VALUE
rb_grn_object_get_name (VALUE self)
{
    RbGrnObject *rb_grn_object;
    VALUE rb_name;
    int name_size;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
	return Qnil;

    name_size = grn_obj_name(rb_grn_object->context, rb_grn_object->object,
			     NULL, 0);
    if (name_size == 0)
	return Qnil;

    rb_name = rb_str_buf_new(name_size);
    RSTRING_LEN(rb_name) = name_size;
    grn_obj_name(rb_grn_object->context, rb_grn_object->object,
		 RSTRING_PTR(rb_name), name_size);
    return rb_name;
}

static VALUE
rb_grn_object_equal (VALUE self, VALUE other)
{
    RbGrnObject *self_rb_grn_object, *other_rb_grn_object;

    if (self == other)
        return Qtrue;

    if (!RVAL2CBOOL(rb_funcall(rb_obj_class(self), rb_intern("=="), 1,
                               rb_obj_class(other))))
        return Qfalse;

    self_rb_grn_object = SELF(self);
    other_rb_grn_object = SELF(other);

    return self_rb_grn_object->object == other_rb_grn_object->object;
}

static VALUE
rb_grn_object_array_reference (VALUE self, VALUE rb_id)
{
    RbGrnObject *rb_grn_object;
    grn_id id;
    grn_obj *value;
    VALUE rb_value = Qnil;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
	return Qnil;

    id = NUM2UINT(rb_id);
    value = grn_obj_get_value(rb_grn_object->context, rb_grn_object->object,
			      id, NULL);
    if (!value) {
	rb_grn_context_check(rb_grn_object->context);
	return Qnil;
    }

    if (!GRN_BULK_EMPTYP(value)) {
	rb_value = rb_str_new(GRN_BULK_HEAD(value), GRN_BULK_VSIZE(value));
	grn_obj_close(rb_grn_object->context, value);
    }
    rb_grn_context_check(rb_grn_object->context);

    return rb_value;
}

static VALUE
rb_grn_object_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    RbGrnObject *rb_grn_object;
    grn_id id;
    grn_obj *value;
    grn_rc rc;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
	return Qnil;

    id = NUM2UINT(rb_id);
    value = RVAL2GRNBULK(rb_grn_object->context, rb_value);
    rc = grn_obj_set_value(rb_grn_object->context, rb_grn_object->object, id,
			   value, GRN_OBJ_SET);
    grn_obj_close(rb_grn_object->context, value);
    rb_grn_check_rc(rc);

    return Qnil;
}

static VALUE
rb_grn_object_search (int argc, VALUE *argv, VALUE self)
{
    RbGrnObject *rb_grn_object;
    grn_ctx *context;
    grn_obj *query;
    grn_obj *result;
    grn_sel_operator operator;
    grn_search_optarg search_options;
    rb_grn_boolean search_options_is_set = RB_GRN_FALSE;
    rb_grn_boolean query_is_created = RB_GRN_FALSE;
    grn_rc rc;
    VALUE rb_query, options, rb_result, rb_operator;
    VALUE rb_exact, rb_left_common_prefix, rb_suffix, rb_prefix, rb_partial;
    VALUE rb_near, rb_near2, rb_similar, rb_term_extract;
    VALUE rb_similarity_threshold, rb_max_interval, rb_weight_vector;
    VALUE rb_procedure, rb_max_size;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
	return Qnil;

    context = rb_grn_object->context;

    rb_scan_args(argc, argv, "11", &rb_query, &options);

    if (CBOOL2RVAL(rb_obj_is_kind_of(rb_query, rb_cGrnQuery))) {
	grn_query *_query;
	_query = RVAL2GRNQUERY(rb_query);
	query = (grn_obj *)_query;
    } else if (CBOOL2RVAL(rb_obj_is_kind_of(rb_query, rb_cString))) {
	query_is_created = RB_GRN_TRUE;
	query = grn_obj_open(context, GRN_BULK, 0, 0);
	rc = grn_bulk_write(context, query,
			    StringValuePtr(rb_query), RSTRING_LEN(rb_query));
	rb_grn_context_check(context);
	rb_grn_check_rc(rc);
    } else {
	rb_raise(rb_eArgError,
		 "query should be Groonga::Query or query string: <%s>",
		 rb_grn_inspect(rb_query));
    }

    rb_grn_scan_options(options,
			"result", &rb_result,
			"operator", &rb_operator,
			"exact", &rb_exact,
			"left_common_prefix", &rb_left_common_prefix,
			"suffix", &rb_suffix,
			"prefix", &rb_prefix,
			"partial", &rb_partial,
			"near", &rb_near,
			"near2", &rb_near2,
			"similar", &rb_similar,
			"term_exact", &rb_term_extract,
			"similarity_threshold", &rb_similarity_threshold,
			"max_interval", &rb_max_interval,
			"weight_vector", &rb_weight_vector,
			"procedure", &rb_procedure,
			"max_size", &rb_max_size,
			NULL);

    if (NIL_P(rb_result)) {
	result = grn_table_create(context, NULL, 0, NULL,
				  GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
				  NULL /* FIXME: column range type */,
				  0, GRN_ENC_NONE);
	rb_grn_context_check(context);
    } else {
	result = RVAL2GRNOBJECT(rb_result, context);
    }

    operator = RVAL2GRNSELECTOPERATOR(rb_operator);

    search_options.flags = 0;
    if (!NIL_P(rb_exact)) {
	search_options_is_set = RB_GRN_TRUE;
	if (RVAL2CBOOL(rb_exact))
	    search_options.flags |= GRN_SEARCH_EXACT;
    }
    if (!NIL_P(rb_left_common_prefix)) {
	search_options_is_set = RB_GRN_TRUE;
	if (RVAL2CBOOL(rb_left_common_prefix))
	    search_options.flags |= GRN_SEARCH_LCP;
    }
    if (!NIL_P(rb_suffix)) {
	search_options_is_set = RB_GRN_TRUE;
	if (RVAL2CBOOL(rb_suffix))
	    search_options.flags |= GRN_SEARCH_SUFFIX;
    }
    if (!NIL_P(rb_prefix)) {
	search_options_is_set = RB_GRN_TRUE;
	if (RVAL2CBOOL(rb_prefix))
	    search_options.flags |= GRN_SEARCH_PREFIX;
    }
    if (!NIL_P(rb_partial)) {
	search_options_is_set = RB_GRN_TRUE;
	if (RVAL2CBOOL(rb_partial))
	    search_options.flags |= GRN_SEARCH_PARTIAL;
    }
    if (!NIL_P(rb_near)) {
	search_options_is_set = RB_GRN_TRUE;
	if (RVAL2CBOOL(rb_near))
	    search_options.flags |= GRN_SEARCH_NEAR;
    }
    if (!NIL_P(rb_near2)) {
	search_options_is_set = RB_GRN_TRUE;
	if (RVAL2CBOOL(rb_near2))
	    search_options.flags |= GRN_SEARCH_NEAR2;
    }
    if (!NIL_P(rb_similar)) {
	search_options_is_set = RB_GRN_TRUE;
	if (RVAL2CBOOL(rb_similar))
	    search_options.flags |= GRN_SEARCH_SIMILAR;
    }
    if (!NIL_P(rb_term_extract)) {
	search_options_is_set = RB_GRN_TRUE;
	if (RVAL2CBOOL(rb_term_extract))
	    search_options.flags |= GRN_SEARCH_TERM_EXTRACT;
    }

    search_options.similarity_threshold = 30; /* FIXME */
    if (!NIL_P(rb_similarity_threshold)) {
	search_options_is_set = RB_GRN_TRUE;
	search_options.similarity_threshold = NUM2INT(rb_similarity_threshold);
    }

    search_options.max_interval = 5; /* FIXME */
    if (!NIL_P(rb_max_interval)) {
	search_options_is_set = RB_GRN_TRUE;
	search_options.max_interval = NUM2INT(rb_max_interval);
    }

    search_options.weight_vector = NULL; /* FIXME */
    if (!NIL_P(rb_weight_vector)) {
	search_options_is_set = RB_GRN_TRUE;
	/* FIXME */
	/* search_options.weight_vector = RVAL2INTVECTOR(rb_weight_vector); */
    }

    search_options.proc = NULL;
    if (!NIL_P(rb_procedure)) {
	search_options_is_set = RB_GRN_TRUE;
	search_options.proc = RVAL2GRNOBJECT(rb_procedure, context);
    }

    search_options.max_size = 100; /* FIXME */
    if (!NIL_P(rb_max_size)) {
	search_options_is_set = RB_GRN_TRUE;
	search_options.max_size = INT2NUM(rb_max_size);
    }

    rc = grn_obj_search(context,
			rb_grn_object->object,
			query,
			result,
			operator,
			search_options_is_set ? &search_options : NULL);
    if (query_is_created)
	grn_obj_close(context, query);
    rb_grn_check_rc(rc);

    if (NIL_P(rb_result))
	return GRNOBJECT2RVAL(Qnil, context, result);
    else
	return rb_result;
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

    rb_define_method(rb_cGrnObject, "search", rb_grn_object_search, -1);
}
