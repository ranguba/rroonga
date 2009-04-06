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

#include <stdarg.h>

const char *
rb_grn_inspect (VALUE object)
{
    VALUE inspected;

    inspected = rb_funcall(object, rb_intern("inspect"), 0);
    return RSTRING_PTR(inspected);
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

void
rb_grn_init_utils (VALUE mGrn)
{
}
