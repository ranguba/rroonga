/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2012  Kouhei Sutou <kou@clear-code.com>

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

#define SELF(object, context) (RVAL2GRNNORMALIZER(object, context))

VALUE rb_cGrnNormalizer;

/*
 * Document-class: Groonga::Normalizer < Groonga::Object
 *
 * It normalizes string.
 */

/*
 * Normalizes the @string@.
 *
 * @example
 *   # Normalizes "ABC" with the default normalizer
 *   Groonga::Normalizer.normalize("AbC") # => "abc"
 *
 * @overload normalize(string)
 *   @return [String] The normalized string
 *   @param [String] string The original string
 */
static VALUE
rb_grn_normalizer_s_normalize (int argc, VALUE *argv, VALUE klass)
{
    VALUE rb_context = Qnil;
    VALUE rb_encoded_string;
    VALUE rb_normalized_string;
    grn_ctx *context = NULL;
    grn_obj *grn_string;
    grn_obj *normalizer = GRN_NORMALIZER_AUTO;
    int flags = 0;
    const char *normalized_string;
    unsigned int normalized_string_length;

    if (argc != 1 && argc != 2) {
        rb_raise(rb_eArgError, "wrong number of arguments");
    } else if (TYPE(argv[0]) != T_STRING) {
        rb_raise(rb_eArgError, "argument 0 should be a string to be normalized");
    } else if (argc == 1) {
        flags = GRN_STRING_REMOVE_BLANK;
    } else if (TYPE(argv[1]) == T_FIXNUM) {
        flags = FIX2INT(argv[1]);
    } else {
        rb_raise(rb_eArgError, "argument 1 should be a flag defined in Groonga::Normalizer class");
    }

    context = rb_grn_context_ensure(&rb_context);
    rb_encoded_string = rb_grn_context_rb_string_encode(context, argv[0]);
    grn_string = grn_string_open(context,
                                 RSTRING_PTR(rb_encoded_string),
                                 RSTRING_LEN(rb_encoded_string),
                                 normalizer,
                                 flags);
    rb_grn_context_check(context, argv[0]);
    grn_string_get_normalized(context, grn_string,
                              &normalized_string, &normalized_string_length,
                              NULL);
    rb_normalized_string =
        rb_grn_context_rb_string_new(context,
                                     normalized_string,
                                     normalized_string_length);
    grn_obj_close(context, grn_string);

    return rb_normalized_string;
}

void
rb_grn_init_normalizer (VALUE mGrn)
{
    rb_cGrnNormalizer = rb_define_class_under(mGrn, "Normalizer", rb_cObject);

    rb_define_singleton_method(rb_cGrnNormalizer, "normalize",
                               rb_grn_normalizer_s_normalize, -1);
    rb_define_const(rb_cGrnNormalizer, "REMOVE_BLANK",
                    INT2FIX(GRN_STRING_REMOVE_BLANK));
    rb_define_const(rb_cGrnNormalizer, "REMOVE_TOKENIZED_DELIMITER",
                    INT2FIX(GRN_STRING_REMOVE_TOKENIZED_DELIMITER));
}
