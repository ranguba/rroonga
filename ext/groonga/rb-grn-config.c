/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2015-2016  Kouhei Sutou <kou@clear-code.com>
  Copyright (C) 2016  Masafumi Yokoyama <yokoyama@clear-code.com>

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

#define SELF(object) (RVAL2GRNCONTEXT(object))

/*
 * Document-class: Groonga::Config
 *
 * This class manages database global configurations. Each
 * configuration is key and value pair.
 */

/*
 * Creates a new configuration set for `context`.
 *
 * @overload initialize(context=nil)
 *   @param [Groonga::Context, nil] context The context to be configured.
 *     If `context` is `nil`, {Groonga::Context.default} is used.
 *
 * @since 5.0.9
 */
static VALUE
rb_grn_config_initialize (VALUE self, VALUE rb_context)
{
    rb_iv_set(self, "@context", rb_context);

    return Qnil;
}

/*
 * Gets a configuration value for key.
 *
 * @overload config[](key)
 *   @param [String] key The key.
 *   @return [String, nil] The value associated with `key`.
 *
 * @since 5.0.9
 */
static VALUE
rb_grn_config_get (VALUE self, VALUE rb_key)
{
    VALUE rb_context;
    VALUE rb_value;
    grn_ctx *context;
    const char *key;
    int key_size;
    const char *value;
    uint32_t value_size;

    rb_context = rb_iv_get(self, "@context");
    context = rb_grn_context_ensure(&rb_context);

    rb_key = rb_grn_convert_to_string(rb_key);
    key = RSTRING_PTR(rb_key);
    key_size = RSTRING_LEN(rb_key);

    {
        grn_rc rc;
        rc = grn_config_get(context,
                            key, key_size,
                            &value, &value_size);
        rb_grn_context_check(context, self);
        rb_grn_rc_check(rc, self);
    }

    if (value_size == 0) {
        rb_value = Qnil;
    } else {
        rb_value = rb_grn_context_rb_string_new(context, value, value_size);
    }

    return rb_value;
}

/*
 * Sets a configuration key and value pair.
 *
 * @overload config[]=(key, value)
 *   @param [String] key The key.
 *   @param [String] value The value to be assigned.
 *   @return [String] `value`.
 */
static VALUE
rb_grn_config_set (VALUE self, VALUE rb_key, VALUE rb_value)
{
    VALUE rb_value_original = rb_value;
    VALUE rb_context;
    grn_ctx *context;
    const char *key;
    int key_size;
    const char *value;
    int value_size;

    rb_context = rb_iv_get(self, "@context");
    context = rb_grn_context_ensure(&rb_context);

    rb_key = rb_grn_convert_to_string(rb_key);
    key = RSTRING_PTR(rb_key);
    key_size = RSTRING_LEN(rb_key);

    rb_value = rb_grn_convert_to_string(rb_value);
    value = RSTRING_PTR(rb_value);
    value_size = RSTRING_LEN(rb_value);

    {
        grn_rc rc;
        rc = grn_config_set(context,
                            key, key_size,
                            value, value_size);
        rb_grn_context_check(context, self);
        rb_grn_rc_check(rc, self);
    }

    return rb_value_original;
}

/*
 * Deletes a configuration for key.
 *
 * @overload delete(key)
 *   @param [String] key The key.
 *
 * @since 6.0.0
 */
static VALUE
rb_grn_config_delete (VALUE self, VALUE rb_key)
{
    VALUE rb_context;
    grn_ctx *context;
    const char *key;
    int key_size;

    rb_context = rb_iv_get(self, "@context");
    context = rb_grn_context_ensure(&rb_context);

    rb_key = rb_grn_convert_to_string(rb_key);
    key = RSTRING_PTR(rb_key);
    key_size = RSTRING_LEN(rb_key);

    {
        grn_rc rc;
        rc = grn_config_delete(context,
                               key, key_size);
        rb_grn_context_check(context, self);
        rb_grn_rc_check(rc, self);
    }

    return Qnil;
}

typedef struct {
    grn_ctx *context;
    grn_obj *cursor;
} RbGrnConfigEachData;

static VALUE
rb_grn_config_each_body (VALUE user_data)
{
    RbGrnConfigEachData *data = (RbGrnConfigEachData *)user_data;
    grn_ctx *context;
    grn_obj *cursor;

    context = data->context;
    cursor = data->cursor;
    while (grn_config_cursor_next(context, cursor)) {
        const char *key;
        uint32_t key_size;
        VALUE rb_key;
        const char *value;
        uint32_t value_size;
        VALUE rb_value;

        key_size = grn_config_cursor_get_key(context, cursor, &key);
        rb_key = rb_grn_context_rb_string_new(context, key, key_size);
        value_size = grn_config_cursor_get_value(context, cursor, &value);
        rb_value = rb_grn_context_rb_string_new(context, value, value_size);

        rb_yield_values(2, rb_key, rb_value);
    }

    return Qnil;
}

static VALUE
rb_grn_config_each_ensure (VALUE user_data)
{
    RbGrnConfigEachData *data = (RbGrnConfigEachData *)user_data;

    grn_obj_close(data->context, data->cursor);

    return Qnil;
}

/*
 * Passes all key/value of the config to block in order.
 *
 * @example Shows all keys and all values of the config
 *   context.config["rroonga.key1"] = "value1"
 *   context.config["rroonga.key2"] = "value2"
 *   keys = []
 *   values = []
 *   context.config.each do |key, value|
 *     keys << key
 *     values << value
 *   end
 *   p keys   # => ["rroonga.key1", "rroonga.key2"]
 *   p values # => ["value1", "value2"]
 *
 * @overload each
 *   @yield [key, value]
 *   @yieldparam [String] key The configuration key
 *   @yieldparam [String] value The value which is associated with `key`
 *
 * @since 6.0.0
 */
static VALUE
rb_grn_config_each (VALUE self)
{
    VALUE rb_context;
    RbGrnConfigEachData data;

    RETURN_ENUMERATOR(self, 0, NULL);

    rb_context = rb_iv_get(self, "@context");
    data.context = rb_grn_context_ensure(&rb_context);

    data.cursor = grn_config_cursor_open(data.context);
    rb_ensure(rb_grn_config_each_body, (VALUE)&data,
              rb_grn_config_each_ensure, (VALUE)&data);

    return Qnil;
}

void
rb_grn_init_config (VALUE mGrn)
{
    VALUE cGrnConfig;

    cGrnConfig = rb_define_class_under(mGrn, "Config", rb_cObject);
    /* For backward compatibility. */
    rb_define_const(mGrn, "Conf", cGrnConfig);

    rb_include_module(cGrnConfig, rb_mEnumerable);

    rb_define_method(cGrnConfig, "initialize", rb_grn_config_initialize, 1);

    rb_define_method(cGrnConfig, "[]", rb_grn_config_get, 1);
    rb_define_method(cGrnConfig, "[]=", rb_grn_config_set, 2);

    rb_define_method(cGrnConfig, "delete", rb_grn_config_delete, 1);

    rb_define_method(cGrnConfig, "each", rb_grn_config_each, 0);
}
