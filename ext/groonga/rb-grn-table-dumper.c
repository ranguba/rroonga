/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2011  Kouhei Sutou <kou@clear-code.com>

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

/*
 * Document-class: Groonga::TableDumper
 *
 * テーブルの内容をgrn式形式の文字列に変換するクラス。
 */

#include "rb-grn.h"

#define SELF(object) ((RbGrnTableDumper *)DATA_PTR(object))

typedef struct _RbGrnTableDumperCache RbGrnTableDumperCache;
struct _RbGrnTableDumperCache
{
    grn_obj *table;
    VALUE rb_buffer;
    VALUE rb_output;
};

typedef struct _RbGrnTableDumper RbGrnTableDumper;
struct _RbGrnTableDumper
{
    grn_ctx *context;
    grn_obj *buffer;
    grn_obj *target;
    RbGrnTableDumperCache cache;
};

static VALUE rb_cGrnTableDumper;

static void
rb_grn_table_dumper_free (RbGrnTableDumper *dumper)
{
    grn_ctx *context;
    grn_obj *buffer;
    grn_obj *target;

    context = dumper->context;
    buffer = dumper->buffer;
    target = dumper->target;
    debug("table-dumper-free: %p:%p:%p (%p)\n", context, buffer, target, dumper);
    if (!rb_grn_exited && context) {
        grn_obj_unlink(context, buffer);
        grn_obj_unlink(context, target);
    }
    xfree(dumper);
}

static VALUE
rb_grn_table_dumper_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_table_dumper_free, NULL);
}

/*
 * Document-method: new
 *
 * call-seq:
 *   Groonga::TableDumper.new(output)
 *
 * _output_にテーブルの内容を出力するGroonga::TableDumperを
 * 作成する。
 */
static VALUE
rb_grn_table_dumper_initialize (int argc, VALUE *argv, VALUE self)
{
    RbGrnTableDumper *dumper;
    VALUE rb_output, rb_options, rb_context;
    grn_ctx *context;

    rb_scan_args(argc, argv, "11", &rb_output, &rb_options);
    rb_grn_scan_options(rb_options,
			"context", &rb_context,
			NULL);

    context = rb_grn_context_ensure(&rb_context);

    dumper = ALLOC(RbGrnTableDumper);
    DATA_PTR(self) = dumper;

    dumper->context = context;
    dumper->buffer = grn_obj_open(context, GRN_BULK, 0, GRN_DB_TEXT);
    dumper->target = grn_obj_open(context, GRN_VOID, 0, GRN_DB_VOID);
    rb_iv_set(self, "@context", rb_context);
    rb_iv_set(self, "@output", rb_output);
    rb_iv_set(self, "@buffer", rb_str_new(NULL, 0));

    return Qnil;
}

static inline void
dumper_flush (RbGrnTableDumper *dumper)
{
    RbGrnTableDumperCache *cache;

    cache = &(dumper->cache);
    rb_funcall(cache->rb_buffer, rb_intern("clear"), 0);
    rb_str_cat(cache->rb_buffer,
	       GRN_TEXT_VALUE(dumper->buffer),
	       GRN_TEXT_LEN(dumper->buffer));
    rb_io_write(cache->rb_output, cache->rb_buffer);
    GRN_BULK_REWIND(dumper->buffer);
}

/*
 * Document-method: dump
 *
 * call-seq:
 *   dumper.dump(table)
 *
 * _table_の内容を@outputに出力する。
 */
static VALUE
rb_grn_table_dumper_dump (VALUE self, VALUE rb_table)
{
    RbGrnTableDumper *dumper;
    RbGrnTableDumperCache *cache;
    grn_ctx *context, *table_context;
    grn_obj *buffer;
    int name_size;

    dumper = SELF(self);
    cache = &(dumper->cache);
    cache->table = RVAL2GRNTABLE(rb_table, &table_context);
    cache->rb_output = rb_iv_get(self, "@output");
    cache->rb_buffer = rb_iv_get(self, "@buffer");

    context = dumper->context;
    buffer = dumper->buffer;

    GRN_TEXT_PUTS(context, buffer, "load --table ");
    name_size = grn_obj_name(context, cache->table, NULL, 0);
    grn_bulk_space(context, buffer, name_size);
    name_size = grn_obj_name(context, cache->table,
			     GRN_BULK_CURR(buffer) - name_size, name_size);
    GRN_TEXT_PUTS(context, buffer, "\n");
    GRN_TEXT_PUTS(context, buffer, "[\n");
    dumper_flush(dumper);

    GRN_TEXT_PUTS(context, buffer, "]\n");
    dumper_flush(dumper);

    cache->table = NULL;
    cache->rb_output = Qnil;
    cache->rb_buffer = Qnil;

    return self;
}

void
rb_grn_init_table_dumper (VALUE mGrn)
{
    rb_cGrnTableDumper = rb_define_class_under(mGrn, "TableDumper", rb_cObject);
    rb_define_alloc_func(rb_cGrnTableDumper, rb_grn_table_dumper_alloc);

    rb_define_method(rb_cGrnTableDumper, "initialize",
		     rb_grn_table_dumper_initialize, -1);

    rb_define_method(rb_cGrnTableDumper, "dump", rb_grn_table_dumper_dump, 1);
}
