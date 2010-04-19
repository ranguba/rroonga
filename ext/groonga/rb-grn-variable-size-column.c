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

#define SELF(object) ((RbGrnVariableSizeColumn *)DATA_PTR(object))

VALUE rb_cGrnVariableSizeColumn;

/*
 * Document-class: Groonga::VariableSizeColumn < Groonga::Column
 *
 * 可変長データ用のカラム。
 */

void
rb_grn_init_variable_size_column (VALUE mGrn)
{
    rb_cGrnVariableSizeColumn =
	rb_define_class_under(mGrn, "VariableSizeColumn", rb_cGrnColumn);
}
