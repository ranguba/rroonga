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

#define SELF(object) ((RbGrnColumn *)DATA_PTR(object))

VALUE rb_cGrnVariableSizeColumn;

/*
 * Document-class: Groonga::VariableSizeColumn < Groonga::Column
 *
 * 可変長データ用のカラム。
 */

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

    rb_define_method(rb_cGrnVariableSizeColumn, "defrag",
		     rb_grn_variable_size_column_defrag, -1);
}
