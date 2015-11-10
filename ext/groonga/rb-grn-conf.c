/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2015  Kouhei Sutou <kou@clear-code.com>

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
 * Document-class: Groonga::Conf
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
rb_grn_conf_initialize (VALUE self, VALUE rb_context)
{
    rb_iv_set(self, "@context", rb_context);

    return Qnil;
}

/*
 * Gets a configuration value for key.
 *
 * @overload conf[](key)
 *   @param [String] key The key.
 *   @return [String, nil] The value associated with `key`.
 *
 * @since 5.0.9
 */
static VALUE
rb_grn_conf_get (VALUE self, VALUE rb_key)
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
        rc = grn_conf_get(context,
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
 * @overload conf[]=(key, value)
 *   @param [String] key The key.
 *   @param [String] value The value to be assigned.
 *   @return [String] `value`.
 */
static VALUE
rb_grn_conf_set (VALUE self, VALUE rb_key, VALUE rb_value)
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
        rc = grn_conf_set(context,
                          key, key_size,
                          value, value_size);
        rb_grn_context_check(context, self);
        rb_grn_rc_check(rc, self);
    }

    return rb_value_original;
}

void
rb_grn_init_conf (VALUE mGrn)
{
    VALUE cGrnConf;

    cGrnConf = rb_define_class_under(mGrn, "Conf", rb_cObject);

    rb_define_method(cGrnConf, "initialize", rb_grn_conf_initialize, 1);

    rb_define_method(cGrnConf, "[]", rb_grn_conf_get, 1);
    rb_define_method(cGrnConf, "[]=", rb_grn_conf_set, 2);
}
