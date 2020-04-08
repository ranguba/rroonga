/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sts=4 sw=4 ts=8 noet: */
/*
  Copyright (C) 2009-2020  Sutou Kouhei <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
    VALUE original_options = options;
    VALUE available_keys;
    const char *key;
    VALUE *value;
    va_list args;

    options = rb_grn_check_convert_to_hash(options);
    if (NIL_P(options)) {
        if (NIL_P(original_options)) {
            options = rb_hash_new();
        } else {
            rb_raise(rb_eArgError,
                     "options must be Hash: %s",
                     rb_grn_inspect(original_options));
        }
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
        if (NIL_P(*value)) {
            rb_key = rb_str_new_cstr(key);
            *value = rb_funcall(options, rb_intern("delete"), 1, rb_key);
        }

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

    key_string = rb_str_new_cstr(key);
    if (RVAL2CBOOL(rb_funcall(option, rb_intern("=="), 1, key_string)))
        return GRN_TRUE;

    key_symbol = rb_str_intern(key_string);
    if (RVAL2CBOOL(rb_funcall(option, rb_intern("=="), 1, key_symbol)))
        return GRN_TRUE;

    return GRN_FALSE;
}

grn_bool
rb_grn_equal_string (const char *string1, const char *string2)
{
    if (string1 == string2) {
        return GRN_TRUE;
    }

    if (!string1 || !string2) {
        return GRN_FALSE;
    }

    return strcmp(string1, string2) == 0;
}

VALUE
rb_grn_convert_to_string (VALUE object)
{
    if (rb_type(object) == T_SYMBOL) {
        return rb_sym2str(object);
    } else {
        return rb_convert_type(object, RUBY_T_STRING, "String", "to_str");
    }
}

VALUE
rb_grn_convert_to_array (VALUE object)
{
    return rb_convert_type(object, RUBY_T_ARRAY, "Array", "to_ary");
}

VALUE
rb_grn_convert_to_path (VALUE object)
{
    VALUE path;

    path = rb_grn_check_convert_to_string(object);
    if (NIL_P(path)) {
        ID to_path;
        CONST_ID(to_path, "to_path");
        path = rb_check_funcall(object, to_path, 0, 0);
        if (path == Qundef) {
            path = object;
        }
        path = rb_grn_convert_to_string(path);
    }

    return path;
}

VALUE
rb_grn_check_convert_to_string (VALUE object)
{
    return rb_check_string_type(object);
}

VALUE
rb_grn_check_convert_to_array (VALUE object)
{
    return rb_check_array_type(object);
}

VALUE
rb_grn_check_convert_to_hash (VALUE object)
{
    return rb_check_convert_type(object, RUBY_T_HASH, "Hash", "to_hash");
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
    case GRN_DB_INT8:
        *rb_value = INT2NUM(GRN_INT8_VALUE(bulk));
        break;
    case GRN_DB_UINT8:
        *rb_value = UINT2NUM(GRN_UINT8_VALUE(bulk));
        break;
    case GRN_DB_INT16:
        *rb_value = INT2NUM(GRN_INT16_VALUE(bulk));
        break;
    case GRN_DB_UINT16:
        *rb_value = UINT2NUM(GRN_UINT16_VALUE(bulk));
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
#if RB_GRN_HAVE_FLOAT32
    case GRN_DB_FLOAT32:
        *rb_value = rb_float_new(GRN_FLOAT32_VALUE(bulk));
        break;
#endif
    case GRN_DB_FLOAT:
        *rb_value = rb_float_new(GRN_FLOAT_VALUE(bulk));
        break;
    case GRN_DB_TIME: {
        int64_t time_value, sec, usec;

        time_value = GRN_TIME_VALUE(bulk);
        GRN_TIME_UNPACK(time_value, sec, usec);
        *rb_value = rb_funcall(rb_cTime, rb_intern("at"), 2,
                               LL2NUM(sec), LL2NUM(usec));
        break;
    }
    case GRN_DB_SHORT_TEXT:
    case GRN_DB_TEXT:
    case GRN_DB_LONG_TEXT:
        *rb_value = rb_grn_context_rb_string_new(context,
                                                 GRN_TEXT_VALUE(bulk),
                                                 GRN_TEXT_LEN(bulk));
        break;
    case GRN_DB_TOKYO_GEO_POINT: {
        int latitude, longitude;

        GRN_GEO_POINT_VALUE(bulk, latitude, longitude);
        *rb_value = rb_grn_tokyo_geo_point_new(latitude, longitude);
        break;
    }
    case GRN_DB_WGS84_GEO_POINT: {
        int latitude, longitude;

        GRN_GEO_POINT_VALUE(bulk, latitude, longitude);
        *rb_value = rb_grn_wgs84_geo_point_new(latitude, longitude);
        break;
    }
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
    case GRN_TABLE_NO_KEY: {
        grn_id id;

        id = *((grn_id *)GRN_BULK_HEAD(bulk));
        if (id == GRN_ID_NIL) {
            *rb_value = Qnil;
        } else {
            VALUE rb_range;

            rb_range = GRNOBJECT2RVAL(Qnil, context, range, GRN_FALSE);
            *rb_value = rb_grn_record_new(rb_range, id, Qnil);
        }
        break;
    }
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
    int object_type;

    if (bulk && bulk->header.domain == GRN_DB_TIME)
        return RVAL2GRNBULK_WITH_TYPE(object, context, bulk,
                                      bulk->header.domain,
                                      grn_ctx_at(context, bulk->header.domain));

    if (!bulk) {
        bulk = grn_obj_open(context, GRN_BULK, 0, GRN_ID_NIL);
        rb_grn_context_check(context, object);
    }

    object_type = TYPE(object);
    switch (object_type) {
    case T_NIL:
        grn_obj_reinit(context, bulk, GRN_DB_VOID, 0);
        break;
    case T_SYMBOL:
    case T_REGEXP:
    case T_STRING:
        switch (object_type) {
        case T_SYMBOL:
            object = rb_funcall(object, rb_intern("to_s"), 0);
            break;
        case T_REGEXP:
            object = rb_funcall(object, rb_intern("source"), 0);
            break;
        default:
            break;
        }
        grn_obj_reinit(context, bulk, GRN_DB_TEXT, 0);
        rb_grn_context_text_set(context, bulk, object);
        break;
    case T_FIXNUM:
    case T_BIGNUM: {
        int64_t int64_value;
        int64_value = NUM2LL(object);
        if (int64_value <= INT32_MAX) {
            grn_obj_reinit(context, bulk, GRN_DB_INT32, 0);
            GRN_INT32_SET(context, bulk, int64_value);
        } else {
            grn_obj_reinit(context, bulk, GRN_DB_INT64, 0);
            GRN_INT64_SET(context, bulk, int64_value);
        }
        break;
    }
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
    union {
        int8_t int8_value;
        uint8_t uint8_value;
        int16_t int16_value;
        uint16_t uint16_value;
        int32_t int32_value;
        uint32_t uint32_value;
        int64_t int64_value;
        uint64_t uint64_value;
        int64_t time_value;
        float float_value;
        double double_value;
        grn_geo_point geo_point_value;
        grn_id record_id;
    } value;
    grn_id range;
    VALUE rb_type_object;
    grn_obj_flags flags = 0;
    grn_bool string_p, table_type_p;

    string_p = rb_type(object) == T_STRING;
    table_type_p = (GRN_TABLE_HASH_KEY <= type->header.type &&
                    type->header.type <= GRN_TABLE_NO_KEY);

    switch (type_id) {
    case GRN_DB_INT8:
        if (string_p) {
            object = rb_Integer(object);
        }
        value.int8_value = NUM2SHORT(object);
        string = (const char *)&(value.int8_value);
        size = sizeof(value.int8_value);
        break;
    case GRN_DB_UINT8:
        if (string_p) {
            object = rb_Integer(object);
        }
        value.uint8_value = NUM2USHORT(object);
        string = (const char *)&(value.uint8_value);
        size = sizeof(value.uint8_value);
        break;
    case GRN_DB_INT16:
        if (string_p) {
            object = rb_Integer(object);
        }
        value.int16_value = NUM2SHORT(object);
        string = (const char *)&(value.int16_value);
        size = sizeof(value.int16_value);
        break;
    case GRN_DB_UINT16:
        if (string_p) {
            object = rb_Integer(object);
        }
        value.uint16_value = NUM2USHORT(object);
        string = (const char *)&(value.uint16_value);
        size = sizeof(value.uint16_value);
        break;
    case GRN_DB_INT32:
        if (string_p) {
            object = rb_Integer(object);
        }
        value.int32_value = NUM2INT(object);
        string = (const char *)&(value.int32_value);
        size = sizeof(value.int32_value);
        break;
    case GRN_DB_UINT32:
        if (string_p) {
            object = rb_Integer(object);
        }
        value.uint32_value = NUM2UINT(object);
        string = (const char *)&(value.uint32_value);
        size = sizeof(value.uint32_value);
        break;
    case GRN_DB_INT64:
        if (string_p) {
            object = rb_Integer(object);
        }
        value.int64_value = NUM2LL(object);
        string = (const char *)&(value.int64_value);
        size = sizeof(value.int64_value);
        break;
    case GRN_DB_UINT64:
        if (string_p) {
            object = rb_Integer(object);
        }
        value.uint64_value = NUM2ULL(object);
        string = (const char *)&(value.uint64_value);
        size = sizeof(value.uint64_value);
        break;
#if RB_GRN_HAVE_FLOAT32
    case GRN_DB_FLOAT32:
        if (string_p) {
            object = rb_Float(object);
        }
        value.float_value = NUM2DBL(object);
        string = (const char *)&(value.float_value);
        size = sizeof(value.float_value);
        break;
#endif
    case GRN_DB_FLOAT:
        if (string_p) {
            object = rb_Float(object);
        }
        value.double_value = NUM2DBL(object);
        string = (const char *)&(value.double_value);
        size = sizeof(value.double_value);
        break;
    case GRN_DB_TIME: {
        VALUE rb_sec, rb_usec;
        int64_t sec;
        int32_t usec;

        if (string_p) {
            ID id_parse;
            CONST_ID(id_parse, "parse");
            object = rb_funcall(rb_cTime, id_parse, 1, object);
        }

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

        value.time_value = GRN_TIME_PACK(sec, usec);
        string = (const char *)&(value.time_value);
        size = sizeof(value.time_value);
        break;
    }
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
    case GRN_DB_TOKYO_GEO_POINT:
    case GRN_DB_WGS84_GEO_POINT: {
        VALUE rb_geo_point;
        VALUE rb_latitude, rb_longitude;
        if (type_id == GRN_DB_TOKYO_GEO_POINT) {
            rb_geo_point = rb_funcall(rb_cGrnTokyoGeoPoint,
                                      rb_intern("new"), 1, object);
        } else {
            rb_geo_point = rb_funcall(rb_cGrnWGS84GeoPoint,
                                      rb_intern("new"), 1, object);
        }
        rb_geo_point = rb_funcall(rb_geo_point, rb_intern("to_msec"), 0);
        rb_latitude  = rb_funcall(rb_geo_point, rb_intern("latitude"), 0);
        rb_longitude = rb_funcall(rb_geo_point, rb_intern("longitude"), 0);
        value.geo_point_value.latitude = NUM2INT(rb_latitude);
        value.geo_point_value.longitude = NUM2INT(rb_longitude);
        string = (const char *)&(value.geo_point_value);
        size = sizeof(value.geo_point_value);
        break;
    }
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
            value.record_id = GRN_ID_NIL;
            string = (const char *)&(value.record_id);
            size = sizeof(value.record_id);
            if (bulk && bulk->header.domain != type_id) {
                grn_obj_reinit(context, bulk, type_id, 0);
            }
        } else {
            return RVAL2GRNBULK(object, context, bulk);
        }
        break;
    }

    if (!bulk) {
        bulk = grn_obj_open(context, GRN_BULK, flags, type_id);
        rb_grn_context_check(context, object);
    }
    if (bulk->header.domain != type_id) {
        grn_obj_reinit(context, bulk, type_id, 0);
    }
    GRN_TEXT_SET(context, bulk, string, size);

    return bulk;
}

VALUE
rb_grn_pvector_to_ruby_object (grn_ctx *context, grn_obj *pvector)
{
    VALUE array;
    unsigned int i, n;

    if (!pvector)
        return Qnil;

    n = GRN_BULK_VSIZE(pvector) / sizeof(grn_obj *);
    array = rb_ary_new2(n);
    for (i = 0; i < n; i++) {
        grn_obj *object = GRN_PTR_VALUE_AT(pvector, i);

        rb_ary_push(array, GRNOBJECT2RVAL(Qnil, context, object, GRN_FALSE));
    }

    return array;
}

grn_obj *
rb_grn_pvector_from_ruby_object (VALUE object,
                                 grn_ctx *context,
                                 grn_obj *pvector)
{
    int i, n;
    VALUE array;

    if (NIL_P(object))
        return pvector;

    array = rb_grn_convert_to_array(object);

    n = RARRAY_LEN(array);
    for (i = 0; i < n; i++) {
        VALUE rb_value = RARRAY_PTR(array)[i];
        grn_obj *value;

        value = RVAL2GRNOBJECT(rb_value, &context);
        GRN_PTR_PUT(context, pvector, value);
    }

    return pvector;
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
    }
    GRN_OBJ_FIN(context, &value);

    return array;
}

static void
rb_grn_add_vector_element (VALUE rb_element, grn_ctx *context, grn_obj *vector,
                           grn_obj *value_buffer)
{
    unsigned int weight = 0;
    if (RVAL2CBOOL(rb_obj_is_kind_of(rb_element, rb_cHash))) {
        VALUE rb_value;
        VALUE rb_weight;
        rb_value = rb_hash_aref(rb_element, RB_GRN_INTERN("value"));
        rb_weight = rb_hash_aref(rb_element, RB_GRN_INTERN("weight"));
        RVAL2GRNOBJ(rb_value, context, &value_buffer);
        if (!NIL_P(rb_weight)) {
            weight = NUM2UINT(rb_weight);
        }
    } else {
        RVAL2GRNOBJ(rb_element, context, &value_buffer);
    }
    grn_vector_add_element(context, vector,
                           GRN_BULK_HEAD(value_buffer),
                           GRN_BULK_VSIZE(value_buffer),
                           weight,
                           value_buffer->header.domain);
}

typedef struct {
    VALUE array;
    grn_ctx *context;
    grn_obj *vector;
    grn_obj value_buffer;
} VectorFromRubyData;

static VALUE
rb_grn_vector_from_ruby_object_body (VALUE user_data)
{
    VectorFromRubyData *data = (VectorFromRubyData *)user_data;
    VALUE *rb_values;
    grn_ctx *context;
    grn_obj *vector;
    grn_obj *value_buffer;
    int i, n;

    n = RARRAY_LEN(data->array);
    rb_values = RARRAY_PTR(data->array);
    context = data->context;
    vector = data->vector;
    value_buffer = &(data->value_buffer);
    for (i = 0; i < n; i++) {
        rb_grn_add_vector_element(rb_values[i], context, vector, value_buffer);
    }

    return Qnil;
}

static VALUE
rb_grn_vector_from_ruby_object_ensure (VALUE user_data)
{
    VectorFromRubyData *data = (VectorFromRubyData *)user_data;

    GRN_OBJ_FIN(data->context, &(data->value_buffer));

    return Qnil;
}

grn_obj *
rb_grn_vector_from_ruby_object (VALUE object, grn_ctx *context, grn_obj *vector)
{
    VectorFromRubyData data;

    if (NIL_P(object))
        return vector;

    data.array = rb_grn_convert_to_array(object);
    data.context = context;
    data.vector = vector;
    GRN_OBJ_INIT(&(data.value_buffer),
                 GRN_BULK,
                 0,
                 vector->header.domain);
    rb_ensure(rb_grn_vector_from_ruby_object_body,   (VALUE)(&data),
              rb_grn_vector_from_ruby_object_ensure, (VALUE)(&data));

    return vector;
}

VALUE
rb_grn_uvector_to_ruby_object (grn_ctx *context, grn_obj *uvector,
                               grn_obj *range, VALUE related_object)
{
    VALUE array = Qnil;

    if (!uvector)
        return Qnil;

    if (!range) {
        rb_raise(rb_eTypeError,
                 "unknown range uvector can't be converted: <%s>",
                 rb_grn_inspect(related_object));
    }

    switch (range->header.type) {
    case GRN_TYPE: {
        const char *current, *end;
        grn_id range_id;
        grn_obj value;
        int value_size;
        value_size = grn_obj_get_range(context, range);
        array = rb_ary_new();
        current = GRN_BULK_HEAD(uvector);
        end = GRN_BULK_CURR(uvector);
        range_id = grn_obj_id(context, range);
        GRN_OBJ_INIT(&value, GRN_BULK, GRN_OBJ_DO_SHALLOW_COPY, range_id);
        while (current < end) {
            VALUE rb_value;
            GRN_TEXT_SET(context, &value, current, value_size);
            rb_value = GRNBULK2RVAL(context, &value, range, related_object);
            rb_ary_push(array, rb_value);
            current += value_size;
        }
        GRN_OBJ_FIN(context, &value);
        break;
    }
    case GRN_TABLE_HASH_KEY:
    case GRN_TABLE_PAT_KEY:
    case GRN_TABLE_DAT_KEY:
    case GRN_TABLE_NO_KEY: {
        grn_id *current, *end;
        VALUE rb_range = Qnil;
        array = rb_ary_new();
        rb_range = GRNTABLE2RVAL(context, range, GRN_FALSE);
        current = (grn_id *)GRN_BULK_HEAD(uvector);
        end = (grn_id *)GRN_BULK_CURR(uvector);
        while (current < end) {
            VALUE record = Qnil;
            if (*current != GRN_ID_NIL) {
                record = rb_grn_record_new(rb_range, *current, Qnil);
            }
            rb_ary_push(array, record);
            current++;
        }
        break;
    }
    default:
        rb_raise(rb_eTypeError,
                 "unknown range uvector can't be converted: %s(%#x): <%s>",
                 grn_obj_type_to_string(range->header.type),
                 range->header.type,
                 rb_grn_inspect(related_object));
        break;
    }

    return array;
}

typedef struct  {
    VALUE object;
    grn_ctx *context;
    grn_obj *uvector;
    VALUE related_object;

    grn_obj element_buffer;
    grn_obj *domain;
    grn_bool succeeded;
} UVectorFromRubyData;

static void
rb_grn_uvector_from_ruby_object_type (UVectorFromRubyData *data)
{
    VALUE object;
    grn_ctx *context;
    grn_obj *uvector;
    grn_obj *type;
    VALUE *rb_values;
    int i, n;
    grn_obj *grn_value;
    int value_size;

    object = data->object;
    context = data->context;
    uvector = data->uvector;
    type = data->domain;
    grn_value = &(data->element_buffer);

    n = RARRAY_LEN(object);
    rb_values = RARRAY_PTR(object);
    value_size = grn_obj_get_range(context, type);
    for (i = 0; i < n; i++) {
        GRN_BULK_REWIND(grn_value);
        RVAL2GRNBULK(rb_values[i], context, grn_value);
        grn_bulk_write(context, uvector, GRN_BULK_HEAD(grn_value), value_size);
    }

    data->succeeded = GRN_TRUE;
}

static void
rb_grn_uvector_from_ruby_object_reference (UVectorFromRubyData *data)
{
    VALUE object;
    grn_ctx *context;
    grn_obj *uvector;
    VALUE related_object;
    VALUE *rb_values;
    int i, n;

    object = data->object;
    context = data->context;
    uvector = data->uvector;
    related_object = data->related_object;

    n = RARRAY_LEN(object);
    rb_values = RARRAY_PTR(object);
    for (i = 0; i < n; i++) {
        VALUE rb_value;
        grn_id id;
        void *grn_value;
        ID id_record_raw_id;

        rb_value = rb_values[i];
        switch (TYPE(rb_value)) {
        case T_FIXNUM:
            id = NUM2UINT(rb_value);
            break;
        default:
            CONST_ID(id_record_raw_id, "record_raw_id");
            if (rb_respond_to(rb_value, id_record_raw_id)) {
                id = NUM2UINT(rb_funcall(rb_value, id_record_raw_id, 0));
            } else {
                rb_raise(rb_eArgError,
                         "uvector value should be one of "
                         "[Fixnum or object that has #record_raw_id]: "
                         "%s (%s): %s",
                         rb_grn_inspect(rb_value),
                         rb_grn_inspect(object),
                         rb_grn_inspect(related_object));
            }
            break;
        }
        grn_value = &id;
        grn_bulk_write(context, uvector, grn_value, sizeof(grn_id));
    }

    data->succeeded = GRN_TRUE;
}

static VALUE
rb_grn_uvector_from_ruby_object_body (VALUE user_data)
{
    UVectorFromRubyData *data = (UVectorFromRubyData *)user_data;
    grn_obj *domain;

    domain = data->domain;
    switch (domain->header.type) {
    case GRN_TYPE:
        rb_grn_uvector_from_ruby_object_type(data);
        break;
    case GRN_TABLE_HASH_KEY:
    case GRN_TABLE_PAT_KEY:
    case GRN_TABLE_DAT_KEY:
    case GRN_TABLE_NO_KEY:
        rb_grn_uvector_from_ruby_object_reference(data);
        break;
    default:
        rb_raise(rb_eTypeError,
                 "can't convert to unknown domain uvector: %s(%#x): <%s>",
                 grn_obj_type_to_string(domain->header.type),
                 domain->header.type,
                 rb_grn_inspect(data->related_object));
        break;
    }

    return Qnil;
}

static VALUE
rb_grn_uvector_from_ruby_object_ensure (VALUE user_data)
{
    UVectorFromRubyData *data = (UVectorFromRubyData *)user_data;

    if (data->domain) {
        grn_obj_unlink(data->context, data->domain);
    }
    GRN_OBJ_FIN(data->context, &(data->element_buffer));

    return Qnil;
}

grn_obj *
rb_grn_uvector_from_ruby_object (VALUE object, grn_ctx *context,
                                 grn_obj *uvector, VALUE related_object)
{
    UVectorFromRubyData data;

    if (NIL_P(object))
        return NULL;

    data.domain = grn_ctx_at(context, uvector->header.domain);
    if (!data.domain) {
        rb_raise(rb_eArgError,
                 "unknown domain uvector can't be converted: <%s>",
                 rb_grn_inspect(related_object));
    }

    GRN_OBJ_INIT(&(data.element_buffer), GRN_BULK, 0, uvector->header.domain);

    data.object = object;
    data.context = context;
    data.uvector = uvector;
    data.related_object = related_object;
    data.succeeded = GRN_FALSE;

    rb_ensure(rb_grn_uvector_from_ruby_object_body, (VALUE)(&data),
              rb_grn_uvector_from_ruby_object_ensure, (VALUE)(&data));

    if (!data.succeeded) {
        return NULL;
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
        return GRNUVECTOR2RVAL(context, value, range, related_object);
        break;
    case GRN_VECTOR:
        return GRNVECTOR2RVAL(context, value);
        break;
    default:
        rb_raise(rb_eGrnError,
                 "unsupported value type: %s(%#x): %s",
                 grn_obj_type_to_string(value->header.type),
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
                            VALUE rb_related_object)
{
    VALUE rb_id;

    if (NIL_P(object))
        return Qnil;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnRecord)) &&
        !RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cInteger))) {
        VALUE rb_record;
        if (table) {
            VALUE rb_table;
            rb_table = GRNOBJECT2RVAL(Qnil, context, table, GRN_FALSE);
            if (!NIL_P(rb_table) &&
                RVAL2CBOOL(rb_obj_is_kind_of(rb_table,
                                             rb_mGrnTableKeySupport))) {
                rb_record = rb_funcall(rb_table, rb_intern("[]"), 1, object);
                if (NIL_P(rb_record)) {
                    rb_raise(rb_eArgError,
                             "nonexistent key: %" PRIsVALUE
                             ": %" PRIsVALUE
                             ": %" PRIsVALUE,
                             object,
                             rb_table,
                             rb_related_object);
                }
                object = rb_record;
            }
        }
    }

    if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnRecord))) {
        VALUE rb_table;
        rb_table = rb_funcall(object, rb_intern("table"), 0);
        if (table && RVAL2GRNOBJECT(rb_table, &context) != table) {
            VALUE rb_expected_table;

            rb_expected_table =
                GRNOBJECT2RVAL(Qnil, context, table, GRN_FALSE);
            rb_raise(rb_eGrnError,
                     "wrong table: expected %" PRIsVALUE
                     ": actual %" PRIsVALUE
                     ": %" PRIsVALUE,
                     rb_expected_table,
                     rb_table,
                     rb_related_object);
        }
        rb_id = rb_funcall(object, rb_intern("id"), 0);
    } else {
        rb_id = object;
    }

    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_id, rb_cInteger)))
        rb_raise(rb_eGrnError,
                 "should be unsigned integer or Groogna::Record: %" PRIsVALUE
                 ": %" PRIsVALUE,
                 object,
                 rb_related_object);

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
rb_grn_value_from_ruby_object (VALUE object, grn_ctx *context,
                               grn_obj *value, grn_id type_id, grn_obj *type)
{
    grn_bool string_p, table_type_p;

    string_p = rb_type(object) == T_STRING;
    table_type_p = (GRN_TABLE_HASH_KEY <= type->header.type &&
                    type->header.type <= GRN_TABLE_NO_KEY);
    if (!string_p) {
        return RVAL2GRNBULK_WITH_TYPE(object, context, value, type_id, type);
    }

    if (table_type_p && RSTRING_LEN(object) == 0) {
        if (value) {
            if (value->header.domain != type_id) {
                grn_obj_reinit(context, value, type_id, 0);
            }
        } else {
            value = grn_obj_open(context, GRN_BULK, 0, type_id);
            rb_grn_context_check(context, object);
        }
        GRN_RECORD_SET(context, value, GRN_ID_NIL);
        return value;
    }

    return RVAL2GRNBULK(object, context, value);
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
/*      klass = GRNOBJECT2RCLASS(obj); */

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
    case GRN_PVECTOR:
        return GRNPVECTOR2RVAL(context, obj);
        break;
    case GRN_VECTOR:
        return GRNVECTOR2RVAL(context, obj);
        break;
    /* case GRN_MSG: */
    /* case GRN_QUERY: */
    /* case GRN_ACCESSOR: */
    /* case GRN_SNIP: */
    /* case GRN_PATSNIP: */
    /* case GRN_CURSOR_TABLE_HASH_KEY: */
    /* case GRN_CURSOR_TABLE_PAT_KEY: */
    /* case GRN_CURSOR_TABLE_NO_KEY: */
    /* case GRN_CURSOR_COLUMN_INDEX: */
    /* case GRN_CURSOR_CONFIG: */
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
                 grn_obj_type_to_string(obj->header.type),
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
