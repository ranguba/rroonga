/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2016  Kouhei Sutou <kou@clear-code.com>

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

void
rb_grn_table_sort_keys_fill (grn_ctx *context,
                             grn_table_sort_key *sort_keys,
                             size_t n_sort_keys,
                             VALUE rb_sort_keys,
                             VALUE rb_table)
{
    size_t i;

    for (i = 0; i < n_sort_keys; i++) {
        VALUE rb_sort_key;
        VALUE rb_sort_options;
        VALUE rb_key;
        VALUE rb_resolved_key;
        VALUE rb_order;

        rb_sort_key = RARRAY_PTR(rb_sort_keys)[i];
        if (RVAL2CBOOL(rb_obj_is_kind_of(rb_sort_key, rb_cHash))) {
            rb_sort_options = rb_sort_key;
        } else if (RVAL2CBOOL(rb_obj_is_kind_of(rb_sort_key, rb_cArray))) {
            rb_sort_options = rb_hash_new();
            rb_hash_aset(rb_sort_options,
                         RB_GRN_INTERN("key"),
                         rb_ary_entry(rb_sort_key, 0));
            rb_hash_aset(rb_sort_options,
                         RB_GRN_INTERN("order"),
                         rb_ary_entry(rb_sort_key, 1));
        } else {
            rb_sort_options = rb_hash_new();
            rb_hash_aset(rb_sort_options,
                         RB_GRN_INTERN("key"),
                         rb_sort_key);
        }
        rb_grn_scan_options(rb_sort_options,
                            "key", &rb_key,
                            "order", &rb_order,
                            NULL);
        if (RVAL2CBOOL(rb_obj_is_kind_of(rb_key, rb_cString))) {
            rb_resolved_key = rb_grn_table_get_column(rb_table, rb_key);
        } else {
            rb_resolved_key = rb_key;
        }
        sort_keys[i].key = RVAL2GRNOBJECT(rb_resolved_key, &context);
        if (!sort_keys[i].key) {
            rb_raise(rb_eGrnNoSuchColumn,
                     "no such column: <%s>: <%s>",
                     rb_grn_inspect(rb_key),
                     rb_grn_inspect(rb_table));
        }
        if (NIL_P(rb_order) ||
            rb_grn_equal_option(rb_order, "asc") ||
            rb_grn_equal_option(rb_order, "ascending")) {
            sort_keys[i].flags = GRN_TABLE_SORT_ASC;
        } else if (rb_grn_equal_option(rb_order, "desc") ||
                   rb_grn_equal_option(rb_order, "descending")) {
            sort_keys[i].flags = GRN_TABLE_SORT_DESC;
        } else {
            rb_raise(rb_eArgError,
                     "order should be one of "
                     "[nil, :desc, :descending, :asc, :ascending]: %s",
                     rb_grn_inspect(rb_order));
        }
    }

}
