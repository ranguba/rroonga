/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/* vim: set sts=4 sw=4 ts=8 noet: */
/*
  Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>

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

const char *
rb_grn_inspect_type (unsigned char type)
{
  switch (type) {
  case GRN_VOID:
    return "void";
  case GRN_BULK:
    return "bulk";
  case GRN_PTR:
    return "ptr";
  case GRN_UVECTOR:
    return "uvector";
  case GRN_PVECTOR:
    return "pvector";
  case GRN_MSG:
    return "msg";
  case GRN_QUERY:
    return "query";
  case GRN_ACCESSOR:
    return "accessor";
  case GRN_SNIP:
    return "snip";
  case GRN_PATSNIP:
    return "patsnip";
  case GRN_CURSOR_TABLE_HASH_KEY:
    return "cursor-table-hash-key";
  case GRN_CURSOR_TABLE_PAT_KEY:
    return "cursor-table-pat-key";
  case GRN_CURSOR_TABLE_DAT_KEY:
    return "cursor-table-dat-key";
  case GRN_CURSOR_TABLE_NO_KEY:
    return "cursor-table-no-key";
  case GRN_CURSOR_TABLE_VIEW:
    return "cursor-table-view";
  case GRN_CURSOR_COLUMN_INDEX:
    return "cursor-column-index";
  case GRN_TYPE:
    return "type";
  case GRN_PROC:
    return "proc";
  case GRN_EXPR:
    return "expr";
  case GRN_TABLE_HASH_KEY:
    return "table-hash-key";
  case GRN_TABLE_PAT_KEY:
    return "table-pat-key";
  case GRN_TABLE_DAT_KEY:
    return "table-dat-key";
  case GRN_TABLE_NO_KEY:
    return "table-no-key";
  case GRN_TABLE_VIEW:
    return "table-view";
  case GRN_DB:
    return "db";
  case GRN_COLUMN_FIX_SIZE:
    return "column-fix-size";
  case GRN_COLUMN_VAR_SIZE:
    return "column-var-size";
  case GRN_COLUMN_INDEX:
    return "column-index";
  default:
    return "unknown";
  }
}

void
rb_grn_scan_options (VALUE options, ...)
{
    VALUE original_options = options;
    VALUE available_keys;
    const char *key;
    VALUE *value;
    va_list args;

    options = rb_check_convert_type(options, T_HASH, "Hash", "to_hash");
    if (NIL_P(options)) {
        options = rb_hash_new();
    } else if (options == original_options) {
        options = rb_funcall(options, rb_intern("dup"), 0);
    }

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

grn_bool
rb_grn_equal_option (VALUE option, const char *key)
{
    VALUE key_string, key_symbol;

    key_string = rb_str_new2(key);
    if (RVAL2CBOOL(rb_funcall(option, rb_intern("=="), 1, key_string)))
	return GRN_TRUE;

    key_symbol = rb_str_intern(key_string);
    if (RVAL2CBOOL(rb_funcall(option, rb_intern("=="), 1, key_symbol)))
	return GRN_TRUE;

    return GRN_FALSE;
}

static VALUE
rb_grn_bulk_to_ruby_object_by_range_id (grn_ctx *context, grn_obj *bulk,
					grn_id range_id,
					VALUE related_object, VALUE *rb_value)
{
    grn_bool success = GRN_TRUE;

    switch (range_id) {
      case GRN_DB_VOID:
	*rb_value = rb_str_new(GRN_TEXT_VALUE(bulk), GRN_TEXT_LEN(bulk));
	break;
      case GRN_DB_BOOL:
	*rb_value = GRN_BOOL_VALUE(bulk) ? Qtrue : Qfalse;
	break;
      case GRN_DB_INT32:
	*rb_value = INT2NUM(GRN_INT32_VALUE(bulk));
	break;
      case GRN_DB_UINT32:
	*rb_value = UINT2NUM(GRN_UINT32_VALUE(bulk));
	break;
      case GRN_DB_INT64:
	*rb_value = LL2NUM(GRN_INT64_VALUE(bulk));
	break;
      case GRN_DB_UINT64:
	*rb_value = ULL2NUM(GRN_UINT64_VALUE(bulk));
	break;
      case GRN_DB_FLOAT:
	*rb_value = rb_float_new(GRN_FLOAT_VALUE(bulk));
	break;
      case GRN_DB_TIME:
	{
	    int64_t time_value, sec, usec;

	    time_value = GRN_TIME_VALUE(bulk);
	    GRN_TIME_UNPACK(time_value, sec, usec);
	    *rb_value = rb_funcall(rb_cTime, rb_intern("at"), 2,
				   LL2NUM(sec), LL2NUM(usec));
	}
	break;
      case GRN_DB_SHORT_TEXT:
      case GRN_DB_TEXT:
      case GRN_DB_LONG_TEXT:
	*rb_value = rb_grn_context_rb_string_new(context,
						 GRN_TEXT_VALUE(bulk),
						 GRN_TEXT_LEN(bulk));
	break;
      default:
	success = GRN_FALSE;
	break;
    }

    return success;
}

static VALUE
rb_grn_bulk_to_ruby_object_by_range_type (grn_ctx *context, grn_obj *bulk,
					  grn_obj *range, grn_id range_id,
					  VALUE related_object, VALUE *rb_value)
{
    grn_bool success = GRN_TRUE;

    if (!range && range_id != GRN_ID_NIL) {
	range = grn_ctx_at(context, range_id);
    }

    if (!range)
	return GRN_FALSE;

    switch (range->header.type) {
      case GRN_TABLE_HASH_KEY:
      case GRN_TABLE_PAT_KEY:
      case GRN_TABLE_DAT_KEY:
      case GRN_TABLE_NO_KEY:
	{
	    grn_id id;

	    id = *((grn_id *)GRN_BULK_HEAD(bulk));
	    if (id == GRN_ID_NIL) {
		*rb_value = Qnil;
	    } else {
		VALUE rb_range;

		rb_range = GRNOBJECT2RVAL(Qnil, context, range, GRN_FALSE);
		*rb_value = rb_grn_record_new(rb_range, id, Qnil);
	    }
	}
	break;
      case GRN_TYPE:
	if (range->header.flags & GRN_OBJ_KEY_VAR_SIZE) {
	    *rb_value = rb_grn_context_rb_string_new(context,
						     GRN_BULK_HEAD(bulk),
						     GRN_BULK_VSIZE(bulk));
	} else {
	    switch (range->header.flags & GRN_OBJ_KEY_MASK) {
	      case GRN_OBJ_KEY_UINT:
		*rb_value = INT2NUM(GRN_UINT32_VALUE(bulk));
		break;
	      case GRN_OBJ_KEY_INT:
		*rb_value = INT2NUM(GRN_INT32_VALUE(bulk));
		break;
	      case GRN_OBJ_KEY_FLOAT:
		*rb_value = rb_float_new(GRN_FLOAT_VALUE(bulk));
		break;
	      default:
		success = GRN_FALSE;
	    }
	    break;
	}
	break;
      default:
	success = GRN_FALSE;
	break;
    }

    return success;
}

VALUE
rb_grn_bulk_to_ruby_object (grn_ctx *context, grn_obj *bulk,
			    grn_obj *range, VALUE related_object)
{
    grn_id range_id;
    VALUE rb_value = Qnil;

    if (GRN_BULK_EMPTYP(bulk))
	return Qnil;

    range_id = bulk->header.domain;
    if (rb_grn_bulk_to_ruby_object_by_range_id(context, bulk, range_id,
    					       related_object, &rb_value))
    	return rb_value;

    if (rb_grn_bulk_to_ruby_object_by_range_type(context, bulk,
    						 range, range_id,
    						 related_object, &rb_value))
    	return rb_value;

    return rb_grn_context_rb_string_new(context,
					GRN_BULK_HEAD(bulk),
					GRN_BULK_VSIZE(bulk));
}

grn_obj *
rb_grn_bulk_from_ruby_object (VALUE object, grn_ctx *context, grn_obj *bulk)
{
    if (bulk && bulk->header.domain == GRN_DB_TIME)
        return rb_grn_bulk_from_ruby_object_with_type(
            object, context, bulk, bulk->header.domain,
            grn_ctx_at(context, bulk->header.domain));

    if (!bulk) {
	bulk = grn_obj_open(context, GRN_BULK, 0, GRN_ID_NIL);
	rb_grn_context_check(context, object);
    }

    switch (TYPE(object)) {
      case T_NIL:
	grn_obj_reinit(context, bulk, GRN_DB_VOID, 0);
	break;
      case T_SYMBOL:
	object = rb_funcall(object, rb_intern("to_s"), 0);
      case T_STRING:
	grn_obj_reinit(context, bulk, GRN_DB_TEXT, 0);
	rb_grn_context_text_set(context, bulk, object);
	break;
      case T_FIXNUM:
	grn_obj_reinit(context, bulk, GRN_DB_INT32, 0);
	GRN_INT32_SET(context, bulk, NUM2INT(object));
	break;
      case T_BIGNUM:
	{
	    int64_t int64_value;
	    int64_value = NUM2LL(object);
	    if (int64_value <= INT32_MAX) {
		grn_obj_reinit(context, bulk, GRN_DB_INT32, 0);
		GRN_INT32_SET(context, bulk, int64_value);
	    } else {
		grn_obj_reinit(context, bulk, GRN_DB_INT64, 0);
		GRN_INT64_SET(context, bulk, int64_value);
	    }
	}
	break;
      case T_FLOAT:
	grn_obj_reinit(context, bulk, GRN_DB_FLOAT, 0);
	GRN_FLOAT_SET(context, bulk, NUM2DBL(object));
	break;
      case T_TRUE:
	grn_obj_reinit(context, bulk, GRN_DB_BOOL, 0);
	GRN_BOOL_SET(context, bulk, GRN_TRUE);
	break;
      case T_FALSE:
	grn_obj_reinit(context, bulk, GRN_DB_BOOL, 0);
	GRN_BOOL_SET(context, bulk, GRN_FALSE);
	break;
      default:
	if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cTime))) {
	    VALUE sec, usec;
	    int64_t time_value;

	    sec = rb_funcall(object, rb_intern("to_i"), 0);
	    usec = rb_funcall(object, rb_intern("usec"), 0);
	    time_value = GRN_TIME_PACK(NUM2LL(sec), NUM2LL(usec));
	    grn_obj_reinit(context, bulk, GRN_DB_TIME, 0);
	    GRN_TIME_SET(context, bulk, time_value);
	} else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnObject))) {
	    grn_obj *grn_object;
	    grn_id id_value;

	    grn_object = RVAL2GRNOBJECT(object, &context);
	    grn_obj_reinit(context, bulk, grn_object->header.domain, 0);
	    id_value = grn_obj_id(context, grn_object);
	    GRN_RECORD_SET(context, bulk, id_value);
	} else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnRecord))) {
	    grn_obj *table;
	    grn_id id_value;

	    table = RVAL2GRNOBJECT(rb_funcall(object, rb_intern("table"), 0),
				   &context);
	    id_value = NUM2UINT(rb_funcall(object, rb_intern("id"), 0));
	    grn_obj_reinit(context, bulk, grn_obj_id(context, table), 0);
	    GRN_RECORD_SET(context, bulk, id_value);
	} else {
	    rb_raise(rb_eTypeError,
		     "bulked object should be one of "
		     "[nil, true, false, String, Symbol, Integer, Float, Time, "
		     "Groonga::Object, Groonga::Record]: %s",
		     rb_grn_inspect(object));
	}
	break;
    }

    return bulk;
}

grn_obj *
rb_grn_bulk_from_ruby_object_with_type (VALUE object, grn_ctx *context,
					grn_obj *bulk,
					grn_id type_id, grn_obj *type)
{
    const char *string;
    unsigned int size;
    int32_t int32_value;
    uint32_t uint32_value;
    int64_t int64_value;
    uint64_t uint64_value;
    int64_t time_value;
    double double_value;
    grn_id record_id, range;
    VALUE rb_type_object;
    grn_obj_flags flags = 0;
    grn_bool string_p, table_type_p;

    string_p = rb_type(object) == T_STRING;
    table_type_p = (GRN_TABLE_HASH_KEY <= type->header.type &&
		    type->header.type <= GRN_TABLE_VIEW);
    if (string_p && !table_type_p) {
	return RVAL2GRNBULK(object, context, bulk);
    }

    switch (type_id) {
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
      case GRN_DB_UINT64:
	uint64_value = NUM2ULL(object);
	string = (const char *)&uint64_value;
	size = sizeof(uint64_value);
	break;
      case GRN_DB_FLOAT:
	double_value = NUM2DBL(object);
	string = (const char *)&double_value;
	size = sizeof(double_value);
	break;
      case GRN_DB_TIME:
	{
	    VALUE rb_sec, rb_usec;
            int64_t sec;
            int32_t usec;

            switch (TYPE(object)) {
            case T_FIXNUM:
            case T_BIGNUM:
                sec = NUM2LL(object);
                usec = 0;
                break;
            case T_FLOAT:
                rb_sec = rb_funcall(object, rb_intern("to_i"), 0);
                rb_usec = rb_funcall(object, rb_intern("remainder"), 1,
				     INT2NUM(1));

                sec = NUM2LL(rb_sec);
                usec = (int32_t)(NUM2DBL(rb_usec) * 1000000);
                break;
	    case T_NIL:
	        sec = 0;
	        usec = 0;
	        break;
            default:
                sec = NUM2LL(rb_funcall(object, rb_intern("to_i"), 0));
                usec = NUM2INT(rb_funcall(object, rb_intern("usec"), 0));
                break;
            }

	    time_value = GRN_TIME_PACK(sec, usec);
	}
	string = (const char *)&time_value;
	size = sizeof(time_value);
	break;
      case GRN_DB_SHORT_TEXT:
      case GRN_DB_TEXT:
      case GRN_DB_LONG_TEXT:
	string = StringValuePtr(object);
	size = RSTRING_LEN(object);
	range = grn_obj_get_range(context, type);
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
	rb_type_object = GRNOBJECT2RVAL(Qnil, context, type, GRN_FALSE);
	rb_raise(rb_eArgError,
		 "unbulkable type: %s",
		 rb_grn_inspect(rb_type_object));
	break;
      default:
	if (table_type_p &&
	    (NIL_P(object) || (string_p && RSTRING_LEN(object) == 0))) {
	    record_id = GRN_ID_NIL;
	    string = (const char *)&record_id;
	    size = sizeof(record_id);
	    if (bulk && bulk->header.domain != type_id) {
		grn_obj_reinit(context, bulk, type_id, 0);
	    }
	} else {
	    return RVAL2GRNBULK(object, context, bulk);
	}
	break;
    }

    if (!bulk) {
	bulk = grn_obj_open(context, GRN_BULK, flags, GRN_ID_NIL);
	rb_grn_context_check(context, object);
    }
    GRN_TEXT_SET(context, bulk, string, size);

    return bulk;
}


VALUE
rb_grn_vector_to_ruby_object (grn_ctx *context, grn_obj *vector)
{
    VALUE array;
    grn_obj value;
    unsigned int i, n;

    if (!vector)
	return Qnil;

    GRN_VOID_INIT(&value);
    n = grn_vector_size(context, vector);
    array = rb_ary_new2(n);
    for (i = 0; i < n; i++) {
	const char *_value;
	unsigned int weight, length;
	grn_id domain;

	length = grn_vector_get_element(context, vector, i,
					&_value, &weight, &domain);
	grn_obj_reinit(context, &value, domain, 0);
	grn_bulk_write(context, &value, _value, length);
	rb_ary_push(array, GRNOBJ2RVAL(Qnil, context, &value, Qnil));
	/* UINT2NUM(weight); */ /* TODO: How handle weight? */
    }
    GRN_OBJ_FIN(context, &value);

    return array;
}

grn_obj *
rb_grn_vector_from_ruby_object (VALUE object, grn_ctx *context, grn_obj *vector)
{
    VALUE *values;
    grn_obj value;
    int i, n;

    if (vector)
	GRN_OBJ_INIT(vector, GRN_VECTOR, 0, GRN_ID_NIL);
    else
	vector = grn_obj_open(context, GRN_VECTOR, 0, 0);

    if (NIL_P(object))
	return vector;

    GRN_VOID_INIT(&value);
    n = RARRAY_LEN(object);
    values = RARRAY_PTR(object);
    for (i = 0; i < n; i++) {
	grn_obj *_value = &value;
	RVAL2GRNOBJ(values[i], context, &_value);
	grn_vector_add_element(context, vector,
			       GRN_BULK_HEAD(&value),
			       GRN_BULK_VSIZE(&value),
			       0,
			       value.header.domain);
    }
    GRN_OBJ_FIN(context, &value);

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
				 grn_obj *uvector, VALUE related_object)
{
    VALUE *values;
    int i, n;

    if (NIL_P(object))
	return NULL;

    n = RARRAY_LEN(object);
    values = RARRAY_PTR(object);
    for (i = 0; i < n; i++) {
	VALUE value;
	grn_id id;
	void *grn_value;

	value = values[i];
	switch (TYPE(value)) {
	  case T_FIXNUM:
	    id = NUM2UINT(value);
	    break;
	  default:
	    if (rb_respond_to(value, rb_intern("record_raw_id"))) {
		id = NUM2UINT(rb_funcall(value, rb_intern("record_raw_id"), 0));
	    } else {
		grn_obj_unlink(context, uvector);
		rb_raise(rb_eArgError,
			 "uvector value should be one of "
			 "[Fixnum or object that has #record_raw_id]: "
			 "%s (%s): %s",
			 rb_grn_inspect(value),
			 rb_grn_inspect(object),
			 rb_grn_inspect(related_object));
	    }
	    break;
	}
	grn_value = &id;
	grn_bulk_write(context, uvector, grn_value, sizeof(grn_id));
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
	return GRNBULK2RVAL(context, value, range, related_object);
	break;
      case GRN_UVECTOR:
	{
	    VALUE rb_value, rb_range = Qnil;
	    grn_id *uvector, *uvector_end;

	    rb_value = rb_ary_new();
	    if (range)
		rb_range = GRNTABLE2RVAL(context, range, GRN_FALSE);
	    uvector = (grn_id *)GRN_BULK_HEAD(value);
	    uvector_end = (grn_id *)GRN_BULK_CURR(value);
	    for (; uvector < uvector_end; uvector++) {
		VALUE record = Qnil;
		if (*uvector != GRN_ID_NIL)
		    record = rb_grn_record_new(rb_range, *uvector, Qnil);
		rb_ary_push(rb_value, record);
	    }
	    return rb_value;
	}
	break;
      case GRN_VECTOR:
	return GRNVECTOR2RVAL(context, value);
	break;
      default:
	rb_raise(rb_eGrnError,
		 "unsupported value type: %s(%#x): %s",
		 rb_grn_inspect_type(value->header.type),
		 value->header.type,
		 rb_grn_inspect(related_object));
	break;
    }

    if (!range)
	return GRNOBJECT2RVAL(Qnil, context, value, GRN_FALSE);

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
		GRNOBJECT2RVAL(Qnil, context, table, GRN_FALSE);
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

    GRN_OBJ_INIT(&bulk, GRN_BULK, GRN_OBJ_DO_SHALLOW_COPY, table->header.domain);
    GRN_TEXT_SET(context, &bulk, key, key_size);

    return GRNBULK2RVAL(context, &bulk, NULL, related_object);
}

grn_obj *
rb_grn_key_from_ruby_object (VALUE rb_key, grn_ctx *context,
			     grn_obj *key, grn_id domain_id, grn_obj *domain,
			     VALUE related_object)
{
    grn_id id;

    if (!domain)
	return RVAL2GRNBULK(rb_key, context, key);

    switch (domain->header.type) {
      case GRN_TYPE:
	return RVAL2GRNBULK_WITH_TYPE(rb_key, context, key, domain_id, domain);
	break;
      case GRN_TABLE_HASH_KEY:
      case GRN_TABLE_PAT_KEY:
      case GRN_TABLE_DAT_KEY:
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

grn_obj *
rb_grn_obj_from_ruby_object (VALUE rb_object, grn_ctx *context, grn_obj **_obj)
{
    if (RVAL2CBOOL(rb_obj_is_kind_of(rb_object, rb_cGrnObject))) {
	if (*_obj) {
	    grn_obj_unlink(context, *_obj); /* TODO: reduce memory allocation */
	}
	*_obj = RVAL2GRNOBJECT(rb_object, &context);
    } else {
	*_obj = RVAL2GRNBULK(rb_object, context, *_obj);
    }

    return *_obj;
}

VALUE
rb_grn_obj_to_ruby_object (VALUE klass, grn_ctx *context,
			   grn_obj *obj, VALUE related_object)
{
    if (!obj)
	return Qnil;

/*     if (NIL_P(klass)) */
/* 	klass = GRNOBJECT2RCLASS(obj); */

    switch (obj->header.type) {
      case GRN_VOID:
	if (GRN_BULK_VSIZE(obj) > 0)
	    return rb_str_new(GRN_BULK_HEAD(obj), GRN_BULK_VSIZE(obj));
	else
	    return Qnil;
	break;
      case GRN_BULK:
	return GRNBULK2RVAL(context, obj, NULL, related_object);
	break;
      /* case GRN_PTR: */
      /* case GRN_UVECTOR: */
      /* case GRN_PVECTOR: */
      /* case GRN_VECTOR: */
      /* case GRN_MSG: */
      /* case GRN_QUERY: */
      /* case GRN_ACCESSOR: */
      /* case GRN_SNIP: */
      /* case GRN_PATSNIP: */
      /* case GRN_CURSOR_TABLE_HASH_KEY: */
      /* case GRN_CURSOR_TABLE_PAT_KEY: */
      /* case GRN_CURSOR_TABLE_NO_KEY: */
      /* case GRN_CURSOR_COLUMN_INDEX: */
      /* case GRN_TYPE: */
      /* case GRN_PROC: */
      /* case GRN_EXPR: */
      /* case GRN_TABLE_HASH_KEY: */
      /* case GRN_TABLE_PAT_KEY: */
      /* case GRN_TABLE_DAT_KEY: */
      /* case GRN_TABLE_NO_KEY: */
      /* case GRN_DB: */
      /* case GRN_COLUMN_FIX_SIZE: */
      /* case GRN_COLUMN_VAR_SIZE: */
      /* case GRN_COLUMN_INDEX: */
      default:
	rb_raise(rb_eTypeError,
		 "unsupported groonga object: %s(%#x): <%s>",
		 rb_grn_inspect_type(obj->header.type),
		 obj->header.type,
		 rb_grn_inspect(related_object));
	break;
    }

    return Qnil;
}

void
rb_grn_init_utils (VALUE mGrn)
{
}
