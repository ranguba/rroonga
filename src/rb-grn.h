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

#ifndef __RB_GRN_H__
#define __RB_GRN_H__

#include <ruby.h>

#include <groonga.h>

#if defined(__cplusplus)
#  define RB_GRN_BEGIN_DECLS extern "C" {
#  define RB_GRN_END_DECLS }
#else
#  define RB_GRN_BEGIN_DECLS
#  define RB_GRN_END_DECLS
#endif

RB_GRN_BEGIN_DECLS

#if __GNUC__ >= 4
#  define RB_GRN_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#else
#  define RB_GRN_GNUC_NULL_TERMINATED
#endif

#if defined(RUBY_GRN_PLATFORM_WIN32) && !defined(RB_GRN_PLATFORM_WIN32)
#  define RB_GRN_PLATFORM_WIN32 RUBY_GRN_PLATFORM_WIN32
#endif

#if defined(RUBY_GRN_STATIC_COMPILATION) && !defined(RB_GRN_STATIC_COMPILATION)
#  define RB_GRN_STATIC_COMPILATION RUBY_GRN_STATIC_COMPILATION
#endif

#if defined(RB_GRN_PLATFORM_WIN32) && !defined(RB_GRN_STATIC_COMPILATION)
#  ifdef RB_GRN_COMPILATION
#    define RB_GRN_VAR __declspec(dllexport)
#  else
#    define RB_GRN_VAR extern __declspec(dllimport)
#  endif
#else
#  define RB_GRN_VAR extern
#endif

#define RB_GRN_MAJOR_VERSION 0
#define RB_GRN_MINOR_VERSION 0
#define RB_GRN_MICRO_VERSION 1

typedef int rb_grn_boolean;
#define RB_GRN_FALSE (0)
#define RB_GRN_TRUE (!RB_GRN_FALSE)

#define RB_GRN_QUERY_DEFAULT_MAX_EXPRESSIONS 32

#include <stdint.h>

typedef struct {
  int32_t tv_sec;
  int32_t tv_usec;
} grn_timeval;


RB_GRN_VAR VALUE rb_eGrnError;
RB_GRN_VAR VALUE rb_cGrnObject;
RB_GRN_VAR VALUE rb_cGrnDatabase;
RB_GRN_VAR VALUE rb_cGrnTable;
RB_GRN_VAR VALUE rb_mGrnTableKeySupport;
RB_GRN_VAR VALUE rb_cGrnHash;
RB_GRN_VAR VALUE rb_cGrnPatriciaTrie;
RB_GRN_VAR VALUE rb_cGrnArray;
RB_GRN_VAR VALUE rb_cGrnTableCursor;
RB_GRN_VAR VALUE rb_cGrnHashCursor;
RB_GRN_VAR VALUE rb_cGrnPatriciaTrieCursor;
RB_GRN_VAR VALUE rb_cGrnArrayCursor;
RB_GRN_VAR VALUE rb_cGrnType;
RB_GRN_VAR VALUE rb_cGrnProcedure;
RB_GRN_VAR VALUE rb_cGrnColumn;
RB_GRN_VAR VALUE rb_cGrnFixSizeColumn;
RB_GRN_VAR VALUE rb_cGrnVarSizeColumn;
RB_GRN_VAR VALUE rb_cGrnIndexColumn;
RB_GRN_VAR VALUE rb_cGrnAccessor;
RB_GRN_VAR VALUE rb_cGrnRecord;
RB_GRN_VAR VALUE rb_cGrnQuery;
RB_GRN_VAR VALUE rb_cGrnLogger;
RB_GRN_VAR VALUE rb_cGrnSnippet;

void           rb_grn_init_utils                    (VALUE mGrn);
void           rb_grn_init_exception                (VALUE mGrn);
void           rb_grn_init_encoding                 (VALUE mGrn);
void           rb_grn_init_context                  (VALUE mGrn);
void           rb_grn_init_object                   (VALUE mGrn);
void           rb_grn_init_database                 (VALUE mGrn);
void           rb_grn_init_table                    (VALUE mGrn);
void           rb_grn_init_table_key_support        (VALUE mGrn);
void           rb_grn_init_array                    (VALUE mGrn);
void           rb_grn_init_hash                     (VALUE mGrn);
void           rb_grn_init_patricia_trie            (VALUE mGrn);
void           rb_grn_init_table_cursor             (VALUE mGrn);
void           rb_grn_init_type                     (VALUE mGrn);
void           rb_grn_init_procedure                (VALUE mGrn);
void           rb_grn_init_column                   (VALUE mGrn);
void           rb_grn_init_accessor                 (VALUE mGrn);
void           rb_grn_init_record                   (VALUE mGrn);
void           rb_grn_init_query                    (VALUE mGrn);
void           rb_grn_init_logger                   (VALUE mGrn);
void           rb_grn_init_snippet                  (VALUE mGrn);

VALUE          rb_grn_rc_to_exception               (grn_rc rc);
const char    *rb_grn_rc_to_message                 (grn_rc rc);
void           rb_grn_rc_check                      (grn_rc rc,
						     VALUE related_object);

grn_ctx       *rb_grn_context_ensure                (VALUE context);
VALUE          rb_grn_context_get_default           (void);
VALUE          rb_grn_context_to_exception          (grn_ctx *context,
						     VALUE related_object);
void           rb_grn_context_check                 (grn_ctx *context,
						     VALUE related_object);

const char    *rb_grn_inspect                       (VALUE object);
void           rb_grn_scan_options                  (VALUE options, ...)
                                                     RB_GRN_GNUC_NULL_TERMINATED;
rb_grn_boolean rb_grn_equal_option                  (VALUE option,
						     const char *key);

VALUE          rb_grn_object_alloc                  (VALUE klass);
grn_ctx       *rb_grn_object_ensure_context         (VALUE object,
						     VALUE rb_context);
void           rb_grn_object_initialize             (VALUE self,
						     grn_ctx *context,
						     grn_obj *object);
VALUE          rb_grn_object_array_reference        (VALUE object,
						     VALUE rb_id);
VALUE          rb_grn_object_close                  (VALUE object);
VALUE          rb_grn_object_inspect_object         (VALUE inspected,
						     grn_ctx *context,
						     grn_obj *object);
VALUE          rb_grn_object_inspect_object_content (VALUE inspected,
						     grn_ctx *context,
						     grn_obj *object);
VALUE          rb_grn_object_inspect_header         (VALUE object,
						     VALUE inspected);
VALUE          rb_grn_object_inspect_content        (VALUE object,
						     VALUE inspected);
VALUE          rb_grn_object_inspect_footer         (VALUE object,
						     VALUE inspected);

VALUE          rb_grn_table_s_create                (int argc,
						     VALUE *argv,
						     VALUE klass,
						     grn_obj_flags key_store);
VALUE          rb_grn_table_cursor_close            (VALUE object);

VALUE          rb_grn_record_new                    (VALUE table,
						     grn_id id);


#define RB_GRN_INTERN(string)         (ID2SYM(rb_intern(string)))

#define RVAL2CBOOL(object)            (RTEST(object))
#define CBOOL2RVAL(boolean)           ((boolean) ? Qtrue : Qfalse)

#define RVAL2GRNENCODING(object, context) \
                                      (rb_grn_encoding_from_ruby_object(object, context))
#define GRNENCODING2RVAL(encoding)    (rb_grn_encoding_to_ruby_object(encoding))

#define RVAL2GRNCONTEXT(object)       (rb_grn_context_from_ruby_object(object))
#define GRNCONTEXT2RVAL(context)      (rb_grn_context_to_ruby_object(context))

#define RVAL2GRNOBJECT(rb_object, context) \
                                      (rb_grn_object_from_ruby_object(rb_object, context))
#define GRNOBJECT2RVAL(klass, context, object) \
                                      (rb_grn_object_to_ruby_object(klass, context, object))
#define GRNOBJECT2RCLASS(object)      (rb_grn_object_to_ruby_class(object))

#define RVAL2GRNDB(object)            (rb_grn_database_from_ruby_object(object))
#define GRNDB2RVAL(context, db)       (rb_grn_database_to_ruby_object(context, db))

#define RVAL2GRNTABLE(object)         (rb_grn_table_from_ruby_object(object))
#define GRNTABLE2RVAL(context, table) (rb_grn_table_to_ruby_object(context, table))

#define RVAL2GRNTABLECURSOR(object)   (rb_grn_table_cursor_from_ruby_object(object))
#define GRNTABLECURSOR2RVAL(klass, context, cursor) \
                                      (rb_grn_table_cursor_to_ruby_object(klass, context, cursor))
#define GRNTABLECURSOR2RCLASS(object) (rb_grn_table_cursor_to_ruby_class(object))

#define RVAL2GRNCOLUMN(object)        (rb_grn_column_from_ruby_object(object))
#define GRNCOLUMN2RVAL(klass, context, column) \
                                      (rb_grn_column_to_ruby_object(klass, context, column))

#define RVAL2GRNQUERY(object)         (rb_grn_query_from_ruby_object(object))
#define GRNQUERY2RVAL(context, column)(rb_grn_query_to_ruby_object(context, column))

#define RVAL2GRNSELECTOPERATOR(object)(rb_grn_select_operator_from_ruby_object(object))

#define RVAL2GRNLOGGER(object)        (rb_grn_logger_from_ruby_object(object))

#define RVAL2GRNBULK(context, object) (rb_grn_bulk_from_ruby_object(context, object))
#define RVAL2GRNBULK_WITH_TYPE(context, object, type) \
                                      (rb_grn_bulk_from_ruby_object_with_type(context, object, type))
#define GRNBULK2RVAL(context, bulk, related_object) \
                                      (rb_grn_bulk_to_ruby_object(context, bulk, related_object))

#define RVAL2GRNVECTOR(context, object) \
	                              (rb_grn_vector_from_ruby_object(context, object))
#define GRNVECTOR2RVAL(context, vector) \
	                              (rb_grn_vector_to_ruby_object(context, vector))

#define RVAL2GRNUVECTOR(context, object) \
	                              (rb_grn_uvector_from_ruby_object(context, object))
#define GRNUVECTOR2RVAL(context, uvector) \
	                              (rb_grn_uvector_to_ruby_object(context, uvector))

#define GRNVALUE2RVAL(context, value, range, related_object) \
	                              (rb_grn_value_to_ruby_object(context, value, range, related_object))

#define RVAL2GRNID(object, context, table, related_object) \
                                      (rb_grn_id_from_ruby_object(object, context, table, related_object))


grn_encoding   rb_grn_encoding_from_ruby_object     (VALUE object,
						     grn_ctx *context);
VALUE          rb_grn_encoding_to_ruby_object       (grn_encoding encoding);

grn_ctx       *rb_grn_context_from_ruby_object      (VALUE object);
VALUE          rb_grn_context_to_ruby_object        (grn_ctx *context);

grn_obj       *rb_grn_object_from_ruby_object       (VALUE object,
						     grn_ctx *context);
VALUE          rb_grn_object_to_ruby_object         (VALUE klass,
						     grn_ctx *context,
						     grn_obj *object);
VALUE          rb_grn_object_to_ruby_class          (grn_obj *object);

grn_obj       *rb_grn_database_from_ruby_object     (VALUE object);
VALUE          rb_grn_database_to_ruby_object       (grn_ctx *context,
						     grn_obj *db);

grn_obj       *rb_grn_table_from_ruby_object        (VALUE object);
VALUE          rb_grn_table_to_ruby_object          (grn_ctx *context,
						     grn_obj *table);

grn_table_cursor *
               rb_grn_table_cursor_from_ruby_object (VALUE object);
VALUE          rb_grn_table_cursor_to_ruby_object   (VALUE klass,
						     grn_ctx *context,
						     grn_table_cursor *cursor);
VALUE          rb_grn_table_cursor_to_ruby_class    (grn_table_cursor *cursor);

grn_obj       *rb_grn_column_from_ruby_object       (VALUE object);
VALUE          rb_grn_column_to_ruby_object         (VALUE klass,
						     grn_ctx *context,
						     grn_obj *column);

grn_query     *rb_grn_query_from_ruby_object        (VALUE object);
VALUE          rb_grn_query_to_ruby_object          (grn_ctx *context,
						     grn_query *query);

grn_sel_operator
               rb_grn_select_operator_from_ruby_object
                                                    (VALUE object);

grn_logger_info *
               rb_grn_logger_from_ruby_object       (VALUE object);

grn_obj       *rb_grn_bulk_from_ruby_object         (grn_ctx *context,
						     VALUE object);
grn_obj       *rb_grn_bulk_from_ruby_object_with_type
                                                    (grn_ctx *context,
						     VALUE object,
						     grn_id type);
VALUE          rb_grn_bulk_to_ruby_object           (grn_ctx *context,
						     grn_obj *bulk,
						     VALUE related_object);
grn_obj       *rb_grn_vector_from_ruby_object       (grn_ctx *context,
						     VALUE object);
VALUE          rb_grn_vector_to_ruby_object         (grn_ctx *context,
						     grn_obj *vector);
grn_obj       *rb_grn_uvector_from_ruby_object      (grn_ctx *context,
						     VALUE object);
VALUE          rb_grn_uvector_to_ruby_object        (grn_ctx *context,
						     grn_obj *uvector);

VALUE          rb_grn_value_to_ruby_object          (grn_ctx *context,
						     grn_obj *value,
						     grn_obj *range,
						     VALUE related_object);

grn_id         rb_grn_id_from_ruby_object           (VALUE object,
						     grn_ctx *context,
						     grn_obj *table,
						     VALUE related_object);


RB_GRN_END_DECLS

#endif
