/* -*- coding: utf-8; c-file-style: "ruby" -*- */
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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
rb_grn_normalizer_s_normalize (VALUE klass, VALUE rb_string)
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

    context = rb_grn_context_ensure(&rb_context);
    rb_encoded_string = rb_grn_context_rb_string_encode(context, rb_string);
    grn_string = grn_string_open(context,
				 RSTRING_PTR(rb_encoded_string),
				 RSTRING_LEN(rb_encoded_string),
				 normalizer,
				 flags);
    rb_grn_context_check(context, rb_string);
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
			       rb_grn_normalizer_s_normalize, 1);
}
