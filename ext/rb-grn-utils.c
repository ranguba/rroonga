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

#include <stdarg.h>

const char *
rb_grn_inspect (VALUE object)
{
    VALUE inspected;

    inspected = rb_funcall(object, rb_intern("inspect"), 0);
    return StringValueCStr(inspected);
}

void
rb_grn_scan_options (VALUE options, ...)
{
    VALUE available_keys;
    const char *key;
    VALUE *value;
    va_list args;

    if (NIL_P(options))
        options = rb_hash_new();
    else
        options = rb_funcall(options, rb_intern("dup"), 0);

    Check_Type(options, T_HASH);

    available_keys = rb_ary_new();
    va_start(args, options);
    key = va_arg(args, const char *);
    while (key) {
        VALUE rb_key;
        value = va_arg(args, VALUE *);

        rb_key = RB_GRN_INTERN(key);
        rb_ary_push(available_keys, rb_key);
        *value = rb_funcall(options, rb_intern("delete"), 1, rb_key);

        key = va_arg(args, const char *);
    }
    va_end(args);

    if (RVAL2CBOOL(rb_funcall(options, rb_intern("empty?"), 0)))
        return;

    rb_raise(rb_eArgError,
             "unexpected key(s) exist: %s: available keys: %s",
             rb_grn_inspect(rb_funcall(options, rb_intern("keys"), 0)),
             rb_grn_inspect(available_keys));
}

rb_grn_boolean
rb_grn_equal_option (VALUE option, const char *key)
{
    VALUE key_string, key_symbol;

    key_string = rb_str_new2(key);
    if (RVAL2CBOOL(rb_funcall(option, rb_intern("=="), 1, key_string)))
	return RB_GRN_TRUE;

    key_symbol = rb_str_intern(key_string);
    if (RVAL2CBOOL(rb_funcall(option, rb_intern("=="), 1, key_symbol)))
	return RB_GRN_TRUE;

    return RB_GRN_FALSE;
}

static VALUE
rb_grn_bulk_to_ruby_object_by_range_id (grn_ctx *context, grn_obj *bulk,
					grn_obj *range, grn_id range_id,
					VALUE rb_range,
					VALUE related_object, VALUE *rb_value)
{
    rb_grn_boolean success = RB_GRN_TRUE;

    switch (range_id) {
      case GRN_DB_VOID:
	*rb_value = rb_str_new(GRN_BULK_HEAD(bulk), GRN_BULK_VSIZE(bulk));
	break;
      case GRN_DB_INT32:
	*rb_value = INT2NUM(*((int *)GRN_BULK_HEAD(bulk)));
	break;
      case GRN_DB_UINT32:
	*rb_value = UINT2NUM(*((int *)GRN_BULK_HEAD(bulk)));
	break;
      case GRN_DB_INT64:
	*rb_value = LL2NUM(*((long long *)GRN_BULK_HEAD(bulk)));
	break;
      case GRN_DB_FLOAT:
	*rb_value = rb_float_new(*((double *)GRN_BULK_HEAD(bulk)));
	break;
      case GRN_DB_TIME:
	{
	    grn_timeval *time_value = (grn_timeval *)GRN_BULK_HEAD(bulk);
	    *rb_value = rb_funcall(rb_cTime, rb_intern("at"), 2,
				   INT2NUM(time_value->tv_sec),
				   INT2NUM(time_value->tv_usec));
	}
	break;
      case GRN_DB_SHORTTEXT:
      case GRN_DB_TEXT:
      case GRN_DB_LONGTEXT:
	*rb_value = rb_str_new(GRN_BULK_HEAD(bulk), GRN_BULK_VSIZE(bulk));
	break;
      default:
	success = RB_GRN_FALSE;
	break;
    }

    return success;
}

static VALUE
rb_grn_bulk_to_ruby_object_by_range_type (grn_ctx *context, grn_obj *bulk,
					  grn_obj *range, grn_id range_id,
					  VALUE rb_range,
					  VALUE related_object, VALUE *rb_value)
{
    rb_grn_boolean success = RB_GRN_TRUE;

    switch (range->header.type) {
      case GRN_TABLE_HASH_KEY:
      case GRN_TABLE_PAT_KEY:
      case GRN_TABLE_NO_KEY:
	{
	    grn_id id;

	    id = *((grn_id *)GRN_BULK_HEAD(bulk));
	    if (id == GRN_ID_NIL)
		*rb_value = Qnil;
	    else
		*rb_value = rb_grn_record_new(rb_range, id);
	}
	break;
      default:
	success = RB_GRN_FALSE;
	break;
    }

    return success;
}

VALUE
rb_grn_bulk_to_ruby_object (grn_ctx *context, grn_obj *bulk,
			    VALUE related_object)
{
    grn_id range_id;
    grn_obj *range;
    VALUE rb_range;
    VALUE rb_value = Qnil;

    if (GRN_BULK_EMPTYP(bulk))
	return Qnil;

    range_id = bulk->header.domain;
    range = grn_ctx_get(context, range_id);
    rb_range = GRNOBJECT2RVAL(Qnil, context, range, RB_GRN_FALSE);

    if (rb_grn_bulk_to_ruby_object_by_range_id(context, bulk,
					       range, range_id, rb_range,
					       related_object, &rb_value))
	return rb_value;

    if (rb_grn_bulk_to_ruby_object_by_range_type(context, bulk,
						 range, range_id, rb_range,
						 related_object, &rb_value))
	return rb_value;

    return rb_str_new(GRN_BULK_HEAD(bulk), GRN_BULK_VSIZE(bulk));
}

grn_obj *
rb_grn_bulk_from_ruby_object (VALUE object, grn_ctx *context, grn_obj *bulk)
{
    const char *string;
    unsigned int size;
    int32_t int32_value;
    int64_t int64_value;
    grn_timeval time_value;
    double double_value;
    grn_id id_value;
    grn_obj_flags flags = 0;

    if (NIL_P(object)) {
	string = NULL;
	size = 0;
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cString))) {
	string = RSTRING_PTR(object);
	size = RSTRING_LEN(object);
	flags |= GRN_OBJ_DO_SHALLOW_COPY;
    } else if (FIXNUM_P(object)) {
	int32_value = NUM2INT(object);
	string = (const char *)&int32_value;
	size = sizeof(int32_value);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cBignum))) {
	int64_value = NUM2LL(object);
	string = (const char *)&int64_value;
	size = sizeof(int64_value);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cFloat))) {
	double_value = NUM2DBL(object);
	string = (const char *)&double_value;
	size = sizeof(double_value);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cTime))) {
	time_value.tv_sec = NUM2INT(rb_funcall(object, rb_intern("to_i"), 0));
	time_value.tv_usec = NUM2INT(rb_funcall(object, rb_intern("usec"), 0));
	string = (const char *)&time_value;
	size = sizeof(time_value);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnObject))) {
	grn_obj *grn_object;

	grn_object = RVAL2GRNOBJECT(object, &context);
	id_value = grn_obj_id(context, grn_object);
	string = (const char *)&id_value;
	size = sizeof(id_value);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnRecord))) {
	id_value = NUM2UINT(rb_funcall(object, rb_intern("id"), 0));
	string = (const char *)&id_value;
	size = sizeof(id_value);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnRecord))) {
	id_value = NUM2UINT(rb_funcall(object, rb_intern("id"), 0));
	string = (const char *)&id_value;
	size = sizeof(id_value);
    } else {
	rb_raise(rb_eTypeError,
		 "bulked object should be one of "
		 "[nil, String, Integer, Float, Time, Groonga::Object]: %s",
		 rb_grn_inspect(object));
    }

    if (bulk) {
	GRN_OBJ_INIT(bulk, GRN_BULK, flags, GRN_ID_NIL);
    } else {
	bulk = grn_obj_open(context, GRN_BULK, 0, flags);
	rb_grn_context_check(context, object);
    }
    GRN_TEXT_SET(context, bulk, string, size);

    return bulk;
}

grn_obj *
rb_grn_bulk_from_ruby_object_with_type (VALUE object, grn_ctx *context,
					grn_obj *bulk, grn_id type)
{
    const char *string;
    unsigned int size;
    int32_t int32_value;
    uint32_t uint32_value;
    int64_t int64_value;
    grn_timeval time_value;
    double double_value;
    grn_id range;
    VALUE rb_type;
    grn_obj_flags flags = 0;
    grn_obj *type_object;

    switch (type) {
      case GRN_DB_INT32:
	int32_value = NUM2INT(object);
	string = (const char *)&int32_value;
	size = sizeof(int32_value);
	break;
      case GRN_DB_UINT32:
	uint32_value = NUM2UINT(object);
	string = (const char *)&uint32_value;
	size = sizeof(uint32_value);
	break;
      case GRN_DB_INT64:
	int64_value = NUM2LL(object);
	string = (const char *)&int64_value;
	size = sizeof(int64_value);
	break;
      case GRN_DB_FLOAT:
	double_value = NUM2DBL(object);
	string = (const char *)&double_value;
	size = sizeof(double_value);
	break;
      case GRN_DB_TIME:
	time_value.tv_sec = NUM2INT(rb_funcall(object, rb_intern("tv_sec"), 0));
	time_value.tv_usec = NUM2INT(rb_funcall(object, rb_intern("tv_usec"), 0));
	string = (const char *)&time_value;
	size = sizeof(time_value);
	break;
      case GRN_DB_SHORTTEXT:
      case GRN_DB_TEXT:
      case GRN_DB_LONGTEXT:
	string = StringValuePtr(object);
	size = RSTRING_LEN(object);
	range = grn_obj_get_range(context, grn_ctx_get(context, type));
	if (size > range)
	    rb_raise(rb_eArgError,
		     "string is too large: expected: %u <= %u",
		     size, range);
	flags |= GRN_OBJ_DO_SHALLOW_COPY;
	break;
      case GRN_DB_VOID:
      case GRN_DB_DELIMIT:
      case GRN_DB_UNIGRAM:
      case GRN_DB_BIGRAM:
      case GRN_DB_TRIGRAM:
      case GRN_DB_MECAB:
	type_object = grn_ctx_get(context, type);
	rb_type = GRNOBJECT2RVAL(Qnil, context, type_object, RB_GRN_FALSE);
	rb_raise(rb_eArgError,
		 "unbulkable type: %s",
		 rb_grn_inspect(rb_type));
	break;
      default:
	return RVAL2GRNBULK(object, context, bulk);
	break;
    }

    if (!bulk) {
	bulk = grn_obj_open(context, GRN_BULK, flags, GRN_ID_NIL);
	rb_grn_context_check(context, object);
    }
    GRN_TEXT_SET(context, bulk, string, size);

    return bulk;
}


/* FIXME: maybe not work */
VALUE
rb_grn_vector_to_ruby_object (grn_ctx *context, grn_obj *vector)
{
    VALUE array;
    unsigned int i, n;

    if (!vector)
	return Qnil;

    n = grn_vector_size(context, vector);
    array = rb_ary_new2(n);
    for (i = 0; i < n; i++) {
	const char *value;
	unsigned int weight, length;
	grn_id domain;

	length = grn_vector_get_element(context, vector, i,
					&value, &weight, &domain);
	rb_ary_push(array,
		    rb_ary_new3(2,
				rb_str_new(value, length), /* FIXME */
				UINT2NUM(weight)));
    }

    return array;
}

grn_obj *
rb_grn_vector_from_ruby_object (VALUE object, grn_ctx *context, grn_obj *vector)
{
    VALUE *values;
    int i, n;

    if (vector)
	GRN_OBJ_INIT(vector, GRN_VECTOR, 0, GRN_ID_NIL);
    else
	vector = grn_obj_open(context, GRN_VECTOR, 0, 0);

    if (NIL_P(object))
	return vector;

    n = RARRAY_LEN(object);
    values = RARRAY_PTR(object);
    for (i = 0; i < n; i++) {
	VALUE rb_value;
	grn_id id;
	void *grn_value;

	rb_value = values[i];
	id = NUM2UINT(rb_value);
	grn_value = &id;
	grn_vector_add_element(context, vector, grn_value, sizeof(id),
			       0, GRN_ID_NIL);
    }

    return vector;
}

VALUE
rb_grn_uvector_to_ruby_object (grn_ctx *context, grn_obj *uvector)
{
    VALUE array;
    grn_id *current, *end;

    if (!uvector)
	return Qnil;

    array = rb_ary_new();
    current = (grn_id *)GRN_BULK_HEAD(uvector);
    end = (grn_id *)GRN_BULK_CURR(uvector);
    while (current < end) {
	rb_ary_push(array, UINT2NUM(*current));
	current++;
    }

    return array;
}

grn_obj *
rb_grn_uvector_from_ruby_object (VALUE object, grn_ctx *context,
				 grn_obj *uvector)
{
    VALUE *values;
    int i, n;

    if (uvector)
	GRN_OBJ_INIT(uvector, GRN_UVECTOR, 0, GRN_ID_NIL);
    else
	uvector = grn_obj_open(context, GRN_UVECTOR, 0, 0);

    if (NIL_P(object))
	return uvector;

    n = RARRAY_LEN(object);
    values = RARRAY_PTR(object);
    for (i = 0; i < n; i++) {
	VALUE value;
	grn_id id;
	void *grn_value;

	value = values[i];
	id = NUM2UINT(value);
	grn_value = &id;
	grn_bulk_write(context, uvector, grn_value, sizeof(id));
    }

    return uvector;
}

VALUE
rb_grn_value_to_ruby_object (grn_ctx *context,
			     grn_obj *value,
			     grn_obj *range,
			     VALUE related_object)
{
    if (!value)
	return Qnil;

    switch (value->header.type) {
      case GRN_VOID:
	return Qnil;
	break;
      case GRN_BULK:
	if (GRN_BULK_EMPTYP(value))
	    return Qnil;
	if (value->header.domain == GRN_ID_NIL && range)
	    value->header.domain = grn_obj_id(context, range);
	return GRNBULK2RVAL(context, value, related_object);
	break;
      default:
	rb_raise(rb_eGrnError,
		 "unsupported value type: 0x%0x: %s",
		 value->header.type, rb_grn_inspect(related_object));
	break;
    }

    if (!range)
	return GRNOBJECT2RVAL(Qnil, context, value, RB_GRN_FALSE);

    return Qnil;
}

grn_id
rb_grn_id_from_ruby_object (VALUE object, grn_ctx *context, grn_obj *table,
			    VALUE related_object)
{
    VALUE rb_id;

    if (NIL_P(object))
	return Qnil;

    if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnRecord))) {
	VALUE rb_table;
	rb_table = rb_funcall(object, rb_intern("table"), 0);
	if (table && RVAL2GRNOBJECT(rb_table, &context) != table) {
	    VALUE rb_expected_table;

	    rb_expected_table =
		GRNOBJECT2RVAL(Qnil, context, table, RB_GRN_FALSE);
	    rb_raise(rb_eGrnError,
		     "wrong table: expected <%s>: actual <%s>",
		     rb_grn_inspect(rb_expected_table),
		     rb_grn_inspect(rb_table));
	}
	rb_id = rb_funcall(object, rb_intern("id"), 0);
    } else {
	rb_id = object;
    }

    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_id, rb_cInteger)))
	rb_raise(rb_eGrnError,
		 "should be unsigned integer or Groogna::Record: <%s>: <%s>",
		 rb_grn_inspect(object),
		 rb_grn_inspect(related_object));

    return NUM2UINT(rb_id);
}

VALUE
rb_grn_key_to_ruby_object (grn_ctx *context, const void *key, int key_size,
			   grn_obj *table, VALUE related_object)
{
    grn_obj bulk;

    GRN_OBJ_INIT(&bulk, GRN_BULK, GRN_OBJ_DO_SHALLOW_COPY, GRN_ID_NIL);
    GRN_TEXT_SET(context, &bulk, key, key_size);
    bulk.header.domain = table->header.domain;

    return GRNBULK2RVAL(context, &bulk, related_object);
}

grn_obj *
rb_grn_key_from_ruby_object (VALUE rb_key, grn_ctx *context,
			     grn_obj *key, grn_obj *domain,
			     VALUE related_object)
{
    grn_id id;

    if (!domain)
	return RVAL2GRNBULK(rb_key, context, key);

    switch (domain->header.type) {
      case GRN_TYPE:
	return RVAL2GRNBULK_WITH_TYPE(rb_key, context, key,
				      grn_obj_id(context, domain));
	break;
      case GRN_TABLE_HASH_KEY:
      case GRN_TABLE_PAT_KEY:
      case GRN_TABLE_NO_KEY:
	id = RVAL2GRNID(rb_key, context, domain, related_object);
	break;
      default:
	if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_key, rb_cInteger)))
	    rb_raise(rb_eGrnError,
		     "should be unsigned integer: <%s>: <%s>",
		     rb_grn_inspect(rb_key),
		     rb_grn_inspect(related_object));

	id = NUM2UINT(rb_key);
	break;
    }

    GRN_TEXT_SET(context, key, &id, sizeof(id));
    return key;
}

void
rb_grn_init_utils (VALUE mGrn)
{
}
