/* -*- coding: utf-8; c-file-style: "ruby" -*- */
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

#define SELF(object) ((RbGrnColumn *)DATA_PTR(object))

VALUE rb_cGrnVariableSizeColumn;

/*
 * Document-class: Groonga::VariableSizeColumn < Groonga::Column
 *
 * 可変長データ用のカラム。
 */

/*
 * Document-method: compressed?
 *
 * call-seq:
 *   column.compressed?        -> boolean
 *   column.compressed?(type)  -> boolean
 *
 * Returns whether the column is compressed or not. If
 * @type@ is specified, it returns whether the column is
 * compressed by @type@ or not.
 *
 * @return [Boolean] whether the column is compressed or not.
 * @param [:zlib, :lzo] type (nil) If @type@ isn't @nil@,
 *   it checks whether specified compressed type is used or
 *   not.
 * @since 1.3.1
 */
static VALUE
rb_grn_variable_size_column_compressed_p (int argc, VALUE *argv, VALUE self)
{
    RbGrnColumn *rb_grn_column;
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj_flags flags;
    VALUE type;
    grn_bool compressed_p = GRN_FALSE;
    grn_bool accept_any_type = GRN_FALSE;
    grn_bool need_zlib_check = GRN_FALSE;
    grn_bool need_lzo_check = GRN_FALSE;

    rb_scan_args(argc, argv, "01", &type);

    if (NIL_P(type)) {
	accept_any_type = GRN_TRUE;
    } else {
	if (rb_grn_equal_option(type, "zlib")) {
	    need_zlib_check = GRN_TRUE;
	} else if (rb_grn_equal_option(type, "lzo")) {
	    need_lzo_check = GRN_TRUE;
	} else {
	    rb_raise(rb_eArgError,
		     "compressed type should be <:zlib> or <:lzo>: <%s>",
		     rb_grn_inspect(type));
	}
    }

    rb_grn_column = SELF(self);
    rb_grn_object_deconstruct(RB_GRN_OBJECT(rb_grn_column), &column, &context,
			      NULL, NULL,
			      NULL, NULL);

    flags = column->header.flags;
    switch (flags & GRN_OBJ_COMPRESS_MASK) {
      case GRN_OBJ_COMPRESS_ZLIB:
	if (accept_any_type || need_zlib_check) {
	    grn_obj support_p;
	    GRN_BOOL_INIT(&support_p, 0);
	    grn_obj_get_info(context, NULL, GRN_INFO_SUPPORT_ZLIB, &support_p);
	    compressed_p = GRN_BOOL_VALUE(&support_p);
	}
	break;
      case GRN_OBJ_COMPRESS_LZO:
	if (accept_any_type || need_lzo_check) {
	    grn_obj support_p;
	    GRN_BOOL_INIT(&support_p, 0);
	    grn_obj_get_info(context, NULL, GRN_INFO_SUPPORT_LZO, &support_p);
	    compressed_p = GRN_BOOL_VALUE(&support_p);
	}
	break;
    }

    return CBOOL2RVAL(compressed_p);
}

/*
 * Document-method: defrag
 *
 * call-seq:
 *   column.defrag(options={}) -> n_segments
 *
 * Defrags the column.
 *
 * @return [Integer] the number of defraged segments
 * @option options [Integer] :threshold (0) the threshold to
 *   determine whether a segment is defraged. Available
 *   values are -4..22. -4 means all segments are defraged.
 *   22 means no segment is defraged.
 * @since 1.2.6
 */
static VALUE
rb_grn_variable_size_column_defrag (int argc, VALUE *argv, VALUE self)
{
    RbGrnColumn *rb_grn_column;
    grn_ctx *context = NULL;
    grn_obj *column;
    int n_segments;
    VALUE options, rb_threshold;
    int threshold = 0;

    rb_scan_args(argc, argv, "01", &options);
    rb_grn_scan_options(options,
			"threshold", &rb_threshold,
			NULL);
    if (!NIL_P(rb_threshold)) {
	threshold = NUM2INT(rb_threshold);
    }

    rb_grn_column = SELF(self);
    rb_grn_object_deconstruct(RB_GRN_OBJECT(rb_grn_column), &column, &context,
			      NULL, NULL,
			      NULL, NULL);
    n_segments = grn_obj_defrag(context, column, threshold);
    rb_grn_context_check(context, self);

    return INT2NUM(n_segments);
}

void
rb_grn_init_variable_size_column (VALUE mGrn)
{
    rb_cGrnVariableSizeColumn =
	rb_define_class_under(mGrn, "VariableSizeColumn", rb_cGrnColumn);

    rb_define_method(rb_cGrnVariableSizeColumn, "compressed?",
		     rb_grn_variable_size_column_compressed_p, -1);
    rb_define_method(rb_cGrnVariableSizeColumn, "defrag",
		     rb_grn_variable_size_column_defrag, -1);
}
