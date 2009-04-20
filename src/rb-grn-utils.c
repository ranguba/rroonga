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

grn_obj *
rb_grn_bulk_from_ruby_object (grn_ctx *context, VALUE object)
{
    grn_obj *bulk;
    grn_rc rc;
    const char *string;
    unsigned int size;
    int32_t int32_value;
    int64_t int64_value;
    grn_timeval time_value;
    double double_value;

    bulk = grn_obj_open(context, GRN_BULK, 0, 0);
    rb_grn_context_check(context, object);

    if (NIL_P(object)) {
	string = NULL;
	size = 0;
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cString))) {
	string = RSTRING_PTR(object);
	size = RSTRING_LEN(object);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cFixnum))) {
	int32_value = NUM2INT(object);
	string = (const char*)&int32_value;
	size = sizeof(int32_value);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cBignum))) {
	int64_value = NUM2LL(object);
	string = (const char*)&int64_value;
	size = sizeof(int64_value);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cFloat))) {
	double_value = NUM2DBL(object);
	string = (const char*)&double_value;
	size = sizeof(double_value);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cTime))) {
	time_value.tv_sec = NUM2INT(rb_funcall(object, rb_intern("to_i"), 0));
	time_value.tv_usec = NUM2INT(rb_funcall(object, rb_intern("usec"), 0));
	string = (const char*)&time_value;
	size = sizeof(time_value);
    } else {
	grn_obj_close(context, bulk);
	rb_raise(rb_eTypeError,
		 "bulked object should be one of "
		 "[nil, String, Integer, Float, Time]: %s",
		 rb_grn_inspect(object));
    }

    rc = grn_bulk_write(context, bulk, string, size);
    if (rc != GRN_SUCCESS) {
	grn_obj_close(context, bulk);
	rb_grn_context_check(context, object);
	rb_grn_rc_check(rc, object);
    }

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

void
rb_grn_init_utils (VALUE mGrn)
{
}
