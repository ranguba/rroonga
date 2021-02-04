/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sts=4 sw=4 ts=8 noet: */
/*
  Copyright (C) 2018-2021  Sutou Kouhei <kou@clear-code.com>

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

/*
 * Document-class: Groonga::ColumnCache
 *
 * This is a class for accelerating column value read.
 *
 * {Groonga::FixSizeColumn} is only supported for now.
 *
 * @since 7.1.1
 */

VALUE rb_cGrnColumnCache;

static void
rb_grn_column_cache_mark(void *data)
{
    RbGrnColumnCache *rb_grn_column_cache = data;

    if (!rb_grn_column_cache->column_cache)
        return;

    rb_gc_mark(rb_grn_column_cache->rb_column);
}

static void
rb_grn_column_cache_free(void *data)
{
    RbGrnColumnCache *rb_grn_column_cache = data;

    if (!rb_grn_column_cache->column_cache)
        return;

    GRN_OBJ_FIN(rb_grn_column_cache->context,
                &(rb_grn_column_cache->buffer));
    grn_column_cache_close(rb_grn_column_cache->context,
                           rb_grn_column_cache->column_cache);
}

static rb_data_type_t data_type = {
    "Groonga::ColumnCache",
    {
        rb_grn_column_cache_mark,
        rb_grn_column_cache_free,
        NULL,
    },
    NULL,
    NULL,
    RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE
rb_grn_column_cache_allocate (VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &data_type, NULL);
}

static VALUE rb_grn_column_cache_close (VALUE self);

/*
 * Opens a new column cache and passes to the given block. The opened
 * column cache is closed after the given block is finished.
 *
 * @overload open(column) {|column_cache| ...}
 *   @param column [Groonga::Column] The column to be cached.
 *
 *   @yieldparam [Groonga::ColumnCache] The opened column cache.
 *
 *   @return [Object] The object returned by the given block.
 */
static VALUE
rb_grn_column_cache_s_open (VALUE klass, VALUE rb_column)
{
    VALUE rb_column_cache;

    rb_column_cache = rb_funcall(klass, rb_intern("new"), 1, rb_column);
    if (rb_block_given_p()) {
        return rb_ensure(rb_yield, rb_column_cache,
                         rb_grn_column_cache_close, rb_column_cache);
    } else {
        return rb_column_cache;
    }
}

/*
 * @overload initialize(column)
 *   @param column [Groonga::Column] The column to be cached.
 *
 *   @return [Groonga::ColumnCache] Create a new column cache.
 */
static VALUE
rb_grn_column_cache_initialize (VALUE self, VALUE rb_column)
{
    RbGrnColumnCache *rb_grn_column_cache;
    grn_ctx *context;
    grn_obj *column;
    grn_id range_id;

    rb_grn_column_cache = ALLOC(RbGrnColumnCache);
    rb_grn_column_cache->self = self;
    rb_grn_column_cache->context = NULL;
    rb_grn_column_cache->rb_column = rb_column;
    rb_grn_column_cache->column_cache = NULL;
    RTYPEDDATA_DATA(self) = rb_grn_column_cache;

    column = RVAL2GRNCOLUMN(rb_column, &(rb_grn_column_cache->context));
    context = rb_grn_column_cache->context;
    rb_grn_column_cache->column_cache = grn_column_cache_open(context, column);
    if (!rb_grn_column_cache->column_cache) {
        rb_raise(rb_eArgError,
                 "failed to create column cache: %s%s%" PRIsVALUE,
                 context->rc == GRN_SUCCESS ? "" : context->errbuf,
                 context->rc == GRN_SUCCESS ? "" : ": ",
                 rb_column);
    }

    range_id = grn_obj_get_range(context, column);
    GRN_VALUE_FIX_SIZE_INIT(&(rb_grn_column_cache->buffer),
                            GRN_OBJ_DO_SHALLOW_COPY,
                            range_id);
    rb_grn_column_cache->range = grn_ctx_at(context, range_id);
    rb_grn_column_cache->table = grn_ctx_at(context, column->header.domain);

    return Qnil;
}

/*
 * @overload [](id)
 *   @param id [Integer, Groonga::Record] The record ID for the
 *     column value.
 *
 *   @return [Object] The value for the record ID.
 */
static VALUE
rb_grn_column_cache_array_reference (VALUE self, VALUE rb_id)
{
    RbGrnColumnCache *rb_grn_column_cache;
    grn_id id;
    void *value;
    size_t value_size = 0;

    TypedData_Get_Struct(self,
                         RbGrnColumnCache,
                         &data_type,
                         rb_grn_column_cache);

    if (!rb_grn_column_cache->column_cache) {
        return Qnil;
    }

    id = rb_grn_id_from_ruby_object(rb_id,
                                    rb_grn_column_cache->context,
                                    rb_grn_column_cache->table,
                                    self);
    value = grn_column_cache_ref(rb_grn_column_cache->context,
                                 rb_grn_column_cache->column_cache,
                                 id,
                                 &value_size);
    rb_grn_context_check(rb_grn_column_cache->context, self);
    GRN_TEXT_SET_REF(&(rb_grn_column_cache->buffer),
                     value,
                     value_size);

    return GRNBULK2RVAL(rb_grn_column_cache->context,
                        &(rb_grn_column_cache->buffer),
                        rb_grn_column_cache->range,
                        self);
}

/*
 * @overload close
 *   @return [void] Close the column cache.
 */
static VALUE
rb_grn_column_cache_close (VALUE self)
{
    RbGrnColumnCache *rb_grn_column_cache;

    TypedData_Get_Struct(self,
                         RbGrnColumnCache,
                         &data_type,
                         rb_grn_column_cache);

    if (rb_grn_column_cache->column_cache) {
        GRN_OBJ_FIN(rb_grn_column_cache->context,
                    &(rb_grn_column_cache->buffer));
        grn_column_cache_close(rb_grn_column_cache->context,
                               rb_grn_column_cache->column_cache);
        rb_grn_column_cache->column_cache = NULL;
        rb_grn_context_check(rb_grn_column_cache->context, self);
    }

    return Qnil;
}

void
rb_grn_init_column_cache (VALUE mGrn)
{
    rb_cGrnColumnCache =
        rb_define_class_under(mGrn, "ColumnCache", rb_cObject);

    rb_define_alloc_func(rb_cGrnColumnCache, rb_grn_column_cache_allocate);

    rb_define_singleton_method(rb_cGrnColumnCache,
                               "open",
                               rb_grn_column_cache_s_open,
                               1);

    rb_define_method(rb_cGrnColumnCache,
                     "initialize",
                     rb_grn_column_cache_initialize,
                     1);
    rb_define_method(rb_cGrnColumnCache,
                     "[]",
                     rb_grn_column_cache_array_reference,
                     1);
    rb_define_method(rb_cGrnColumnCache,
                     "close",
                     rb_grn_column_cache_close,
                     0);
}
