/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2021  Sutou Kouhei <kou@clear-code.com>
  Copyright (C) 2015-2017  Masafumi Yokoyama <yokoyama@clear-code.com>

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

#ifndef __RB_GRN_H__
#define __RB_GRN_H__

#include <ruby.h>
#include <ruby/encoding.h>

#ifdef HAVE_RUBY_INTERN_H
#  include <ruby/intern.h>
#endif

#ifndef RETURN_ENUMERATOR
#  define RETURN_ENUMERATOR(obj, argc, argv)
#endif

#ifndef NUM2SHORT
#  define NUM2SHORT(object) NUM2INT(object)
#endif

#ifndef NUM2USHORT
#  define NUM2USHORT(object) NUM2UINT(object)
#endif

#ifndef HAVE_RB_SYM2STR
#  define rb_sym2str(symbol) (rb_id2str(SYM2ID(symbol)))
#endif

#ifndef HAVE_RB_TO_SYMBOL
#  define rb_to_symbol(rb_string) (rb_str_intern(rb_string))
#endif

#ifndef HAVE_RB_ARY_NEW_FROM_ARGS
#  define rb_ary_new_from_args(n, ...) rb_ary_new3((n), __VA_ARGS__)
#endif

#ifndef HAVE_RB_ARY_NEW_FROM_VALUES
#  define rb_ary_new_from_values(n, values) rb_ary_new4((n), (values))
#endif

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

#ifdef __WIN32__
#  ifdef RB_GRN_COMPILATION
#    define RB_GRN_VAR extern __declspec(dllexport)
#  else
#    define RB_GRN_VAR extern __declspec(dllimport)
#  endif
#else
#  define RB_GRN_VAR extern
#endif

#ifdef RB_GRN_DEBUG
#  define debug(...) fprintf(stderr, __VA_ARGS__)
#else
#  define debug(...)
#endif

#define RB_GRN_HAVE_FLOAT32 GRN_VERSION_OR_LATER(10, 0, 2)

#define RB_GRN_MAJOR_VERSION 12
#define RB_GRN_MINOR_VERSION 1
#define RB_GRN_MICRO_VERSION 0

#define RB_GRN_OBJECT(object) ((RbGrnObject *)(object))
#define RB_GRN_NAMED_OBJECT(object) ((RbGrnNamedObject *)(object))
#define RB_GRN_TABLE(object) ((RbGrnTable *)(object))
#define RB_GRN_TABLE_KEY_SUPPORT(object) ((RbGrnTableKeySupport *)(object))
#define RB_GRN_DOUBLE_ARRAY_TRIE(object) ((RbGrnDoubleArrayTrie *)(object))
#define RB_GRN_TABLE_CURSOR(object) ((RbGrnTableCursor *)(object))
#define RB_GRN_COLUMN(object) ((RbGrnColumn *)(object))
#define RB_GRN_VARIABLE_SIZE_COLUMN(object) ((RbGrnVariableSizeColumn *)(object))
#define RB_GRN_INDEX_COLUMN(object) ((RbGrnIndexColumn *)(object))
#define RB_GRN_ACCESSOR(object) ((RbGrnAccessor *)(object))
#define RB_GRN_EXPRESSION(object) ((RbGrnExpression *)(object))
#define RB_GRN_SNIPPET(object) ((RbGrnSnippet *)(object))
#define RB_GRN_UNBIND_FUNCTION(function) ((RbGrnUnbindFunction)(function))

typedef void (*RbGrnUnbindFunction) (void *object);

typedef struct _RbGrnContext RbGrnContext;
struct _RbGrnContext
{
    grn_ctx *context;
    grn_ctx context_entity;
    grn_hash *floating_objects;
    VALUE self;
};

typedef struct _RbGrnObject RbGrnObject;
struct _RbGrnObject
{
    VALUE self;
    RbGrnContext *rb_grn_context;
    grn_ctx *context;
    grn_obj *object;
    grn_obj *domain;
    grn_id domain_id;
    grn_obj *range;
    grn_id range_id;
    grn_bool need_close;
    grn_bool have_finalizer;
    grn_bool floating;
};

typedef struct _RbGrnNamedObject RbGrnNamedObject;
struct _RbGrnNamedObject
{
    RbGrnObject parent;
    char *name;
    unsigned name_size;
};

typedef struct _RbGrnTable RbGrnTable;
struct _RbGrnTable
{
    RbGrnObject parent;
    grn_obj *value;
    VALUE columns;
};

typedef struct _RbGrnTableKeySupport RbGrnTableKeySupport;
struct _RbGrnTableKeySupport
{
    RbGrnTable parent;
    grn_obj *key;
};

typedef struct _RbGrnDoubleArrayTrie RbGrnDoubleArrayTrie;
struct _RbGrnDoubleArrayTrie
{
    RbGrnTableKeySupport parent;
    grn_obj *new_key;
};

typedef struct _RbGrnColumn RbGrnColumn;
struct _RbGrnColumn
{
    RbGrnNamedObject parent;
    grn_obj *value;
};

typedef struct _RbGrnDataColumn RbGrnDataColumn;
struct _RbGrnDataColumn
{
    RbGrnColumn parent;
};

typedef struct _RbGrnVariableSizeColumn RbGrnVariableSizeColumn;
struct _RbGrnVariableSizeColumn
{
    RbGrnDataColumn parent;
    grn_obj *element_value;
};

typedef struct _RbGrnIndexColumn RbGrnIndexColumn;
struct _RbGrnIndexColumn
{
    RbGrnDataColumn parent;
    grn_obj *old_value;
    grn_obj *set_value;
    grn_obj *id_query;
    grn_obj *string_query;
};

typedef struct _RbGrnIndexCursor RbGrnIndexCursor;
struct _RbGrnIndexCursor
{
    RbGrnObject parent;
};

typedef struct _RbGrnAccessor RbGrnAccessor;
struct _RbGrnAccessor
{
    RbGrnNamedObject parent;
    grn_obj *value;
};

typedef struct _RbGrnTableCursor RbGrnTableCursor;
struct _RbGrnTableCursor
{
    RbGrnObject parent;
};

typedef struct _RbGrnVariable RbGrnVariable;
struct _RbGrnVariable
{
    RbGrnObject parent;
};

typedef struct _RbGrnExpression RbGrnExpression;
struct _RbGrnExpression
{
    RbGrnObject parent;
    grn_obj *value;
};

typedef struct _RbGrnSnippet RbGrnSnippet;
struct _RbGrnSnippet
{
    RbGrnObject parent;
};

typedef struct _RbGrnPlugin RbGrnPlugin;
struct _RbGrnPlugin
{
    VALUE self;
    grn_ctx *context;
    grn_id id;
};

typedef struct _RbGrnColumnCache RbGrnColumnCache;
struct _RbGrnColumnCache
{
    VALUE self;
    grn_ctx *context;
    VALUE rb_column;
    grn_column_cache *column_cache;
    grn_obj buffer;
    grn_obj *range;
    grn_obj *table;
};

RB_GRN_VAR grn_bool rb_grn_exited;

RB_GRN_VAR VALUE rb_eGrnError;
RB_GRN_VAR VALUE rb_eGrnClosed;
RB_GRN_VAR VALUE rb_eGrnNoSuchColumn;
RB_GRN_VAR VALUE rb_eGrnInvalidArgument;
RB_GRN_VAR VALUE rb_eGrnNoMemoryAvailable;
RB_GRN_VAR VALUE rb_cGrnObject;
RB_GRN_VAR VALUE rb_mGrnEncodingSupport;
RB_GRN_VAR VALUE rb_mGrnFlushable;
RB_GRN_VAR VALUE rb_cGrnDatabase;
RB_GRN_VAR VALUE rb_cGrnTable;
RB_GRN_VAR VALUE rb_mGrnTableKeySupport;
RB_GRN_VAR VALUE rb_cGrnHash;
RB_GRN_VAR VALUE rb_cGrnPatriciaTrie;
RB_GRN_VAR VALUE rb_cGrnDoubleArrayTrie;
RB_GRN_VAR VALUE rb_cGrnArray;
RB_GRN_VAR VALUE rb_cGrnTableCursor;
RB_GRN_VAR VALUE rb_mGrnTableCursorKeySupport;
RB_GRN_VAR VALUE rb_cGrnHashCursor;
RB_GRN_VAR VALUE rb_cGrnPatriciaTrieCursor;
RB_GRN_VAR VALUE rb_cGrnDoubleArrayTrieCursor;
RB_GRN_VAR VALUE rb_cGrnArrayCursor;
RB_GRN_VAR VALUE rb_cGrnType;
RB_GRN_VAR VALUE rb_cGrnProcedure;
RB_GRN_VAR VALUE rb_cGrnColumn;
RB_GRN_VAR VALUE rb_cGrnDataColumn;
RB_GRN_VAR VALUE rb_cGrnFixSizeColumn;
RB_GRN_VAR VALUE rb_cGrnVariableSizeColumn;
RB_GRN_VAR VALUE rb_cGrnIndexColumn;
RB_GRN_VAR VALUE rb_cGrnIndexCursor;
RB_GRN_VAR VALUE rb_cGrnAccessor;
RB_GRN_VAR VALUE rb_cGrnGeoPoint;
RB_GRN_VAR VALUE rb_cGrnTokyoGeoPoint;
RB_GRN_VAR VALUE rb_cGrnWGS84GeoPoint;
RB_GRN_VAR VALUE rb_cGrnRecord;
RB_GRN_VAR VALUE rb_cGrnLogger;
RB_GRN_VAR VALUE rb_cGrnSnippet;
RB_GRN_VAR VALUE rb_cGrnVariable;
RB_GRN_VAR VALUE rb_cGrnOperator;
RB_GRN_VAR VALUE rb_cGrnEqualOperator;
RB_GRN_VAR VALUE rb_cGrnNotEqualOperator;
RB_GRN_VAR VALUE rb_cGrnLessOperator;
RB_GRN_VAR VALUE rb_cGrnGreaterOperator;
RB_GRN_VAR VALUE rb_cGrnLessEqualOperator;
RB_GRN_VAR VALUE rb_cGrnGreaterEqualOperator;
RB_GRN_VAR VALUE rb_cGrnMatchOperator;
RB_GRN_VAR VALUE rb_cGrnPrefixOperator;
RB_GRN_VAR VALUE rb_cGrnRegexpOperator;
RB_GRN_VAR VALUE rb_cGrnExpression;
RB_GRN_VAR VALUE rb_cGrnRecordExpressionBuilder;
RB_GRN_VAR VALUE rb_cGrnColumnExpressionBuilder;
RB_GRN_VAR VALUE rb_cGrnPlugin;
RB_GRN_VAR VALUE rb_cGrnNormalizer;
RB_GRN_VAR VALUE rb_cGrnIndex;
RB_GRN_VAR VALUE rb_mGrnRequestCanceler;
RB_GRN_VAR VALUE rb_mGrnRequestTimer;
RB_GRN_VAR VALUE rb_cGrnRequestTimerID;
RB_GRN_VAR VALUE rb_cGrnColumnCache;

RB_GRN_VAR rb_data_type_t rb_grn_object_data_type;

void           rb_grn_init_utils                    (VALUE mGrn);
void           rb_grn_init_exception                (VALUE mGrn);
void           rb_grn_init_encoding                 (VALUE mGrn);
void           rb_grn_init_encoding_support         (VALUE mGrn);
void           rb_grn_init_flushable                (VALUE mGrn);
void           rb_grn_init_context                  (VALUE mGrn);
void           rb_grn_init_object                   (VALUE mGrn);
void           rb_grn_init_database                 (VALUE mGrn);
void           rb_grn_init_table                    (VALUE mGrn);
void           rb_grn_init_table_key_support        (VALUE mGrn);
void           rb_grn_init_array                    (VALUE mGrn);
void           rb_grn_init_hash                     (VALUE mGrn);
void           rb_grn_init_patricia_trie            (VALUE mGrn);
void           rb_grn_init_double_array_trie        (VALUE mGrn);
void           rb_grn_init_table_cursor             (VALUE mGrn);
void           rb_grn_init_table_cursor_key_support (VALUE mGrn);
void           rb_grn_init_array_cursor             (VALUE mGrn);
void           rb_grn_init_hash_cursor              (VALUE mGrn);
void           rb_grn_init_patricia_trie_cursor     (VALUE mGrn);
void           rb_grn_init_double_array_trie_cursor (VALUE mGrn);
void           rb_grn_init_type                     (VALUE mGrn);
void           rb_grn_init_procedure                (VALUE mGrn);
void           rb_grn_init_procedure_type           (VALUE mGrn);
void           rb_grn_init_column                   (VALUE mGrn);
void           rb_grn_init_data_column              (VALUE mGrn);
void           rb_grn_init_fix_size_column          (VALUE mGrn);
void           rb_grn_init_variable_size_column     (VALUE mGrn);
void           rb_grn_init_index_column             (VALUE mGrn);
void           rb_grn_init_index_cursor             (VALUE mGrn);
void           rb_grn_init_inverted_index_cursor    (VALUE mGrn);
void           rb_grn_init_posting                  (VALUE mGrn);
void           rb_grn_init_accessor                 (VALUE mGrn);
void           rb_grn_init_geo_point                (VALUE mGrn);
void           rb_grn_init_record                   (VALUE mGrn);
void           rb_grn_init_variable                 (VALUE mGrn);
void           rb_grn_init_operator                 (VALUE mGrn);
void           rb_grn_init_equal_operator           (VALUE mGrn);
void           rb_grn_init_not_equal_operator       (VALUE mGrn);
void           rb_grn_init_less_operator            (VALUE mGrn);
void           rb_grn_init_greater_operator         (VALUE mGrn);
void           rb_grn_init_less_equal_operator      (VALUE mGrn);
void           rb_grn_init_greater_equal_operator   (VALUE mGrn);
void           rb_grn_init_match_operator           (VALUE mGrn);
void           rb_grn_init_prefix_operator          (VALUE mGrn);
void           rb_grn_init_regexp_operator          (VALUE mGrn);
void           rb_grn_init_expression               (VALUE mGrn);
void           rb_grn_init_expression_builder       (VALUE mGrn);
void           rb_grn_init_logger                   (VALUE mGrn);
void           rb_grn_init_query_logger             (VALUE mGrn);
void           rb_grn_init_windows_event_logger     (VALUE mGrn);
void           rb_grn_init_snippet                  (VALUE mGrn);
void           rb_grn_init_plugin                   (VALUE mGrn);
void           rb_grn_init_normalizer               (VALUE mGrn);
void           rb_grn_init_thread                   (VALUE mGrn);
void           rb_grn_init_config                   (VALUE mGrn);
void           rb_grn_init_index                    (VALUE mGrn);
void           rb_grn_init_request_canceler         (VALUE mGrn);
void           rb_grn_init_request_timer            (VALUE mGrn);
void           rb_grn_init_request_timer_id         (VALUE mGrn);
void           rb_grn_init_id                       (VALUE mGrn);
void           rb_grn_init_name                     (VALUE mGrn);
void           rb_grn_init_default_cache            (VALUE mGrn);
void           rb_grn_init_column_cache             (VALUE mGrn);

VALUE          rb_grn_rc_to_exception               (grn_rc rc);
void           rb_grn_rc_check                      (grn_rc rc,
                                                     VALUE related_object);

void           rb_grn_context_register_floating_object
                                                    (RbGrnObject *rb_grn_object);
void           rb_grn_context_unregister_floating_object
                                                    (RbGrnObject *rb_grn_object);
void           rb_grn_context_close_floating_objects(RbGrnContext *rb_grn_context);
void           rb_grn_context_reset_floating_objects(RbGrnContext *rb_grn_context);
void           rb_grn_context_mark_grn_id           (grn_ctx *context,
                                                     grn_id   id);
grn_ctx       *rb_grn_context_ensure                (VALUE *context);
VALUE          rb_grn_context_get_default           (void);
VALUE          rb_grn_context_to_exception          (grn_ctx *context,
                                                     VALUE related_object);
void           rb_grn_context_check                 (grn_ctx *context,
                                                     VALUE related_object);
grn_obj       *rb_grn_context_get_backward_compatibility
                                                    (grn_ctx *context,
                                                     const char *name,
                                                     unsigned int name_size);
void           rb_grn_context_object_created        (VALUE rb_context,
                                                     VALUE rb_object);

const char    *rb_grn_inspect                       (VALUE object);
void           rb_grn_scan_options                  (VALUE options, ...)
                                                     RB_GRN_GNUC_NULL_TERMINATED;
grn_bool       rb_grn_equal_option                  (VALUE option,
                                                     const char *key);
grn_bool       rb_grn_equal_string                  (const char *string1,
                                                     const char *string2);
VALUE          rb_grn_convert_to_string             (VALUE object);
VALUE          rb_grn_convert_to_array              (VALUE object);
VALUE          rb_grn_convert_to_path               (VALUE object);
VALUE          rb_grn_check_convert_to_string       (VALUE object);
VALUE          rb_grn_check_convert_to_array        (VALUE object);
VALUE          rb_grn_check_convert_to_hash         (VALUE object);

VALUE          rb_grn_object_alloc                  (VALUE klass);
void           rb_grn_object_bind                   (VALUE self,
                                                     VALUE rb_context,
                                                     RbGrnObject *rb_grn_object,
                                                     grn_ctx *context,
                                                     grn_obj *object);
void           rb_grn_object_free                   (RbGrnObject *rb_grn_object);
void           rb_grn_object_assign                 (VALUE klass,
                                                     VALUE self,
                                                     VALUE rb_context,
                                                     grn_ctx *context,
                                                     grn_obj *object);
void           rb_grn_object_deconstruct            (RbGrnObject *rb_grn_object,
                                                     grn_obj **object,
                                                     grn_ctx **context,
                                                     grn_id *domain_id,
                                                     grn_obj **domain,
                                                     grn_id *range_id,
                                                     grn_obj **range);

VALUE          rb_grn_object_get_id                 (VALUE object);
VALUE          rb_grn_object_array_reference        (VALUE object,
                                                     VALUE rb_id);
VALUE          rb_grn_object_set_raw                (RbGrnObject *rb_grn_object,
                                                     grn_id id,
                                                     VALUE rb_value,
                                                     int flags,
                                                     VALUE related_object);
void           rb_grn_object_close_raw              (RbGrnObject *rb_grn_object);
VALUE          rb_grn_object_close                  (VALUE object);
VALUE          rb_grn_object_closed_p               (VALUE object);
VALUE          rb_grn_object_inspect_object         (VALUE inspected,
                                                     grn_ctx *context,
                                                     grn_obj *object);
VALUE          rb_grn_object_inspect_object_content (VALUE inspected,
                                                     grn_ctx *context,
                                                     grn_obj *object);
VALUE          rb_grn_object_inspect_object_content_name
                                                    (VALUE inspected,
                                                     grn_ctx *context,
                                                     grn_obj *object);
VALUE          rb_grn_object_inspect_header         (VALUE object,
                                                     VALUE inspected);
VALUE          rb_grn_object_inspect_content        (VALUE object,
                                                     VALUE inspected);
VALUE          rb_grn_object_inspect_footer         (VALUE object,
                                                     VALUE inspected);

void           rb_grn_database_finalizer            (grn_ctx *context,
                                                     RbGrnContext *rb_grn_context,
                                                     grn_obj *column,
                                                     RbGrnObject *rb_grn_database);

void           rb_grn_named_object_bind             (RbGrnNamedObject *rb_grn_named_object,
                                                     grn_ctx *context,
                                                     grn_obj *object);
void           rb_grn_named_object_finalizer        (grn_ctx *context,
                                                     grn_obj *column,
                                                     RbGrnNamedObject *rb_grn_named_object);
void           rb_grn_named_object_set_name         (RbGrnNamedObject *rb_grn_named_object,
                                                     const char *name,
                                                     unsigned name_size);

void           rb_grn_table_bind                    (RbGrnTable *rb_grn_table,
                                                     grn_ctx *context,
                                                     grn_obj *table_key_support);
void           rb_grn_table_finalizer               (grn_ctx *context,
                                                     grn_obj *grn_object,
                                                     RbGrnTable *rb_grn_table);
void           rb_grn_table_deconstruct             (RbGrnTable *rb_grn_table,
                                                     grn_obj **table,
                                                     grn_ctx **context,
                                                     grn_id *domain_id,
                                                     grn_obj **domain,
                                                     grn_obj **value,
                                                     grn_id *range_id,
                                                     grn_obj **range,
                                                     VALUE *columns);

VALUE          rb_grn_table_delete_by_id            (VALUE self,
                                                     VALUE rb_id);
VALUE          rb_grn_table_delete_by_expression    (VALUE self);
VALUE          rb_grn_table_array_reference         (VALUE self,
                                                     VALUE rb_id);
VALUE          rb_grn_table_array_set               (VALUE self,
                                                     VALUE rb_id,
                                                     VALUE rb_value);
VALUE          rb_grn_table_get_value               (VALUE self,
                                                     VALUE rb_id);
VALUE          rb_grn_table_set_value               (VALUE self,
                                                     VALUE rb_id,
                                                     VALUE rb_value);
VALUE          rb_grn_table_get_column              (VALUE self,
                                                     VALUE rb_name);
VALUE          rb_grn_table_get_column_surely       (VALUE self,
                                                     VALUE rb_name);
VALUE          rb_grn_table_get_column_value_raw    (VALUE self,
                                                     grn_id id,
                                                     VALUE rb_name);
VALUE          rb_grn_table_get_column_value        (VALUE self,
                                                     VALUE rb_id,
                                                     VALUE rb_name);
VALUE          rb_grn_table_set_column_value_raw    (VALUE self,
                                                     grn_id id,
                                                     VALUE rb_name,
                                                     VALUE rb_value);
VALUE          rb_grn_table_set_column_value        (VALUE self,
                                                     VALUE rb_id,
                                                     VALUE rb_name,
                                                     VALUE rb_value);
VALUE          rb_grn_table_inspect_content         (VALUE object,
                                                     VALUE inspected);

void           rb_grn_table_sort_keys_fill          (grn_ctx *context,
                                                     grn_table_sort_key *sort_keys,
                                                     size_t n_sort_keys,
                                                     VALUE rb_sort_keys,
                                                     VALUE rb_table);

grn_ctx       *rb_grn_table_cursor_ensure_context   (VALUE cursor,
                                                     VALUE *rb_context);
int            rb_grn_table_cursor_order_to_flag    (VALUE rb_order);
int            rb_grn_table_cursor_order_by_to_flag (unsigned char table_type,
                                                     VALUE rb_table,
                                                     VALUE rb_order_by);

void           rb_grn_table_key_support_bind        (RbGrnTableKeySupport *rb_grn_table_key_support,
                                                     grn_ctx *context,
                                                     grn_obj *table_key_support);
void           rb_grn_table_key_support_finalizer   (grn_ctx *context,
                                                     grn_obj *grn_object,
                                                     RbGrnTableKeySupport *rb_grn_table_key_support);
void           rb_grn_table_key_support_deconstruct (RbGrnTableKeySupport *rb_grn_table_key_support,
                                                     grn_obj **table_key_support,
                                                     grn_ctx **context,
                                                     grn_obj **key,
                                                     grn_id *domain_id,
                                                     grn_obj **domain,
                                                     grn_obj **value,
                                                     grn_id *range_id,
                                                     grn_obj **range,
                                                     VALUE *columns);
grn_id         rb_grn_table_key_support_get         (VALUE self,
                                                     VALUE rb_key);

void           rb_grn_double_array_trie_bind        (RbGrnDoubleArrayTrie *rb_grn_double_array_trie,
                                                     grn_ctx *context,
                                                     grn_obj *double_array_trie);
void           rb_grn_double_array_trie_finalizer   (grn_ctx *context,
                                                     grn_obj *grn_object,
                                                     RbGrnDoubleArrayTrie *rb_grn_double_array_trie);

void           rb_grn_column_bind                   (RbGrnColumn *rb_grn_column,
                                                     grn_ctx *context,
                                                     grn_obj *column);
void           rb_grn_column_finalizer              (grn_ctx *context,
                                                     grn_obj *column,
                                                     RbGrnColumn *rb_grn_column);
void           rb_grn_column_deconstruct            (RbGrnColumn *rb_grn_column,
                                                     grn_obj **column,
                                                     grn_ctx **context,
                                                     grn_id *domain_id,
                                                     grn_obj **domain,
                                                     grn_obj **value,
                                                     grn_id *range_id,
                                                     grn_obj **range);

void           rb_grn_variable_size_column_bind     (RbGrnVariableSizeColumn *rb_grn_column,
                                                     grn_ctx *context,
                                                     grn_obj *column);
void           rb_grn_variable_size_column_finalizer(grn_ctx *context,
                                                     grn_obj *column,
                                                     RbGrnVariableSizeColumn *rb_grn_column);

void           rb_grn_index_column_bind             (RbGrnIndexColumn *rb_grn_index_column,
                                                     grn_ctx *context,
                                                     grn_obj *object);
void           rb_grn_index_column_finalizer        (grn_ctx *context,
                                                     grn_obj *grn_object,
                                                     RbGrnIndexColumn *rb_grn_index_column);
void           rb_grn_index_column_deconstruct      (RbGrnIndexColumn *rb_grn_index_column,
                                                     grn_obj **column,
                                                     grn_ctx **context,
                                                     grn_id *domain_id,
                                                     grn_obj **domain,
                                                     grn_obj **value,
                                                     grn_obj **old_value,
                                                     grn_obj **set_value,
                                                     grn_id *range_id,
                                                     grn_obj **range,
                                                     grn_obj **id_query,
                                                     grn_obj **string_query);

void           rb_grn_accessor_bind                 (RbGrnAccessor *rb_grn_accessor,
                                                     grn_ctx *context,
                                                     grn_obj *accessor);
void           rb_grn_accessor_finalizer            (grn_ctx *context,
                                                     grn_obj *accessor,
                                                     RbGrnAccessor *rb_grn_accessor);

void           rb_grn_expression_bind               (RbGrnExpression *rb_grn_expression,
                                                     grn_ctx *context,
                                                     grn_obj *expression);
void           rb_grn_expression_finalizer          (grn_ctx *context,
                                                     grn_obj *grn_object,
                                                     RbGrnExpression *rb_grn_expression);

VALUE          rb_grn_posting_new                   (grn_posting *posting,
                                                     grn_id term_id,
                                                     VALUE rb_table,
                                                     VALUE rb_lexicon);
void           rb_grn_posting_update                (VALUE rb_posting,
                                                     grn_posting *posting,
                                                     grn_id term_id);

VALUE          rb_grn_tokyo_geo_point_new           (int   latitude,
                                                     int   longitude);
VALUE          rb_grn_wgs84_geo_point_new           (int   latitude,
                                                     int   longitude);
VALUE          rb_grn_tokyo_geo_point_new_raw       (VALUE latitude,
                                                     VALUE longitude);
VALUE          rb_grn_wgs84_geo_point_new_raw       (VALUE latitude,
                                                     VALUE longitude);

VALUE          rb_grn_record_new                    (VALUE table,
                                                     grn_id id,
                                                     VALUE values);
VALUE          rb_grn_record_new_added              (VALUE table,
                                                     grn_id id,
                                                     VALUE values);
VALUE          rb_grn_record_new_raw                (VALUE table,
                                                     VALUE id,
                                                     VALUE values);

VALUE          rb_grn_record_expression_builder_new (VALUE table,
                                                     VALUE name);
VALUE          rb_grn_record_expression_builder_build
                                                    (VALUE self);
VALUE          rb_grn_column_expression_builder_new (VALUE column,
                                                     VALUE name,
                                                     VALUE query);
VALUE          rb_grn_column_expression_builder_build
                                                    (VALUE self);

VALUE          rb_grn_index_new                     (VALUE rb_index_column,
                                                     VALUE rb_section);


#define RB_GRN_INTERN(c_string)       (rb_to_symbol(rb_str_new_cstr(c_string)))

#define RVAL2CBOOL(object)            (RTEST(object))
#define CBOOL2RVAL(boolean)           ((boolean) ? Qtrue : Qfalse)

#define RSYMBOL2CSTR(rb_symbol)       (RSTRING_PTR(rb_sym2str(rb_symbol)))

#define RVAL2GRNENCODING(object, context) \
                                      (rb_grn_encoding_from_ruby_object(object, context))
#define GRNENCODING2RVAL(encoding)    (rb_grn_encoding_to_ruby_object(encoding))

#define RVAL2GRNCONTEXT(object)       (rb_grn_context_from_ruby_object(object))
#define GRNCONTEXT2RVAL(context)      (rb_grn_context_to_ruby_object(context))

#define RVAL2GRNOBJECT(rb_object, context) \
                                      (rb_grn_object_from_ruby_object(rb_object, context))
#define GRNOBJECT2RVAL(klass, context, object, owner) \
    (rb_grn_object_to_ruby_object(klass, context, object, owner))
#define GRNOBJECT2RCLASS(object)      (rb_grn_object_to_ruby_class(object))

/* TODO: MORE BETTER NAME!!! PLEASE!!! */
#define RVAL2GRNOBJ(rb_object, context, object) \
    (rb_grn_obj_from_ruby_object(rb_object, context, object))
#define GRNOBJ2RVAL(klass, context, object, related_object)     \
    (rb_grn_obj_to_ruby_object(klass, context, object, related_object))

#define RVAL2GRNDB(object)            (rb_grn_database_from_ruby_object(object))
#define GRNDB2RVAL(context, db, owner) \
    (rb_grn_database_to_ruby_object(context, db, owner))

#define RVAL2GRNTABLE(object, context)(rb_grn_table_from_ruby_object(object, context))
#define GRNTABLE2RVAL(context, table, owner) \
    (rb_grn_table_to_ruby_object(context, table, owner))

#define RVAL2GRNTABLECURSOR(object, context)   (rb_grn_table_cursor_from_ruby_object(object, context))
#define GRNTABLECURSOR2RVAL(klass, context, cursor) \
    (rb_grn_table_cursor_to_ruby_object(klass, context, cursor, GRN_TRUE))
#define GRNTABLECURSOR2RCLASS(object) (rb_grn_table_cursor_to_ruby_class(object))

#define RVAL2GRNCOLUMN(object, context) \
    (rb_grn_column_from_ruby_object(object, context))
#define GRNCOLUMN2RVAL(klass, context, column, owner) \
    (rb_grn_column_to_ruby_object(klass, context, column, owner))

#define GRNINDEXCURSOR2RVAL(context, cursor, rb_table, rb_lexicon) \
    (rb_grn_index_cursor_to_ruby_object(context, cursor, rb_table, rb_lexicon, \
                                        GRN_TRUE))

#define RVAL2GRNACCESSOR(object) \
    (rb_grn_accessor_from_ruby_object(object))
#define GRNACCESSOR2RVAL(context, accessor, owner) \
    (rb_grn_accessor_to_ruby_object(context, accessor, owner))

#define RVAL2GRNOPERATOR(object)      (rb_grn_operator_from_ruby_object(object))
#define RVAL2GRNSETOPERATOR(object)   (rb_grn_set_operator_from_ruby_object(object))

#define RVAL2GRNLOGGER(object)        (rb_grn_logger_from_ruby_object(object))
#define RVAL2GRNQUERYLOGGER(object)   (rb_grn_query_logger_from_ruby_object(object))

#define RVAL2GRNBULK(object, context, bulk) \
    (rb_grn_bulk_from_ruby_object(object, context, bulk))
#define RVAL2GRNBULK_WITH_TYPE(object, context, bulk, type_id, type)    \
    (rb_grn_bulk_from_ruby_object_with_type(object, context, bulk, type_id, type))
#define GRNBULK2RVAL(context, bulk, range, related_object)      \
    (rb_grn_bulk_to_ruby_object(context, bulk, range, related_object))

#define RVAL2GRNPVECTOR(object, context, vector) \
    (rb_grn_pvector_from_ruby_object(object, context, vector))
#define GRNPVECTOR2RVAL(context, vector) \
    (rb_grn_pvector_to_ruby_object(context, vector))

#define RVAL2GRNVECTOR(object, context, vector) \
    (rb_grn_vector_from_ruby_object(object, context, vector))
#define GRNVECTOR2RVAL(context, vector) \
    (rb_grn_vector_to_ruby_object(context, vector))

#define RVAL2GRNUVECTOR(object, context, uvector, related_object)       \
    (rb_grn_uvector_from_ruby_object(object, context, uvector, related_object))
#define GRNUVECTOR2RVAL(context, uvector, range, related_object)        \
    (rb_grn_uvector_to_ruby_object(context, uvector, range, related_object))

#define GRNVALUE2RVAL(context, value, range, related_object) \
    (rb_grn_value_to_ruby_object(context, value, range, related_object))

#define RVAL2GRNID(object, context, table, related_object) \
    (rb_grn_id_from_ruby_object(object, context, table, related_object))

#define GRNKEY2RVAL(context, key, key_size, table, related_object) \
    (rb_grn_key_to_ruby_object(context, key, key_size, table, related_object))
#define RVAL2GRNKEY(object, context, key, domain_id, domain, related_object) \
    (rb_grn_key_from_ruby_object(object, context, key, domain_id,       \
                                 domain, related_object))

#define RVAL2GRNVALUE(object, context, key, domain_id, domain) \
    (rb_grn_value_from_ruby_object(object, context, key, domain_id, domain))

#define GRNVARIABLE2RVAL(context, variable) \
    (rb_grn_variable_to_ruby_object(context, variable))
#define RVAL2GRNVARIABLE(object, context) \
    (rb_grn_variable_from_ruby_object(object, context))

#define GRN_REQUEST_TIMER_ID2RVAL(id) \
    (rb_grn_request_timer_id_to_ruby_object(id))
#define RVAL2GRN_REQUEST_TIMER_ID(rb_id) \
    (rb_grn_request_timer_id_from_ruby_object(rb_id))


grn_encoding   rb_grn_encoding_from_ruby_object     (VALUE object,
                                                     grn_ctx *context);
VALUE          rb_grn_encoding_to_ruby_object       (grn_encoding encoding);
rb_encoding   *rb_grn_encoding_to_ruby_encoding     (grn_encoding encoding);
VALUE          rb_grn_encoding_to_ruby_encoding_object
                                                    (grn_encoding encoding);

RbGrnContext  *rb_grn_context_get_struct            (VALUE rb_context);
grn_ctx       *rb_grn_context_from_ruby_object      (VALUE rb_context);
VALUE          rb_grn_context_to_ruby_object        (grn_ctx *context);
VALUE          rb_grn_context_rb_string_new         (grn_ctx *context,
                                                     const char *string,
                                                     long length);
VALUE          rb_grn_context_rb_string_encode      (grn_ctx *context,
                                                     VALUE rb_string);
void           rb_grn_context_text_set              (grn_ctx *context,
                                                     grn_obj *bulk,
                                                     VALUE rb_string);

grn_obj       *rb_grn_object_from_ruby_object       (VALUE object,
                                                     grn_ctx **context);
VALUE          rb_grn_object_to_ruby_object         (VALUE klass,
                                                     grn_ctx *context,
                                                     grn_obj *object,
                                                     grn_bool owner);
VALUE          rb_grn_object_to_ruby_class          (grn_obj *object);

grn_obj       *rb_grn_database_from_ruby_object     (VALUE object);
VALUE          rb_grn_database_to_ruby_object       (grn_ctx *context,
                                                     grn_obj *db,
                                                     grn_bool owner);

grn_obj       *rb_grn_table_from_ruby_object        (VALUE object,
                                                     grn_ctx **context);
VALUE          rb_grn_table_to_ruby_object          (grn_ctx *context,
                                                     grn_obj *table,
                                                     grn_bool owner);

grn_table_cursor *
               rb_grn_table_cursor_from_ruby_object (VALUE object,
                                                     grn_ctx **context);
VALUE          rb_grn_table_cursor_to_ruby_object   (VALUE klass,
                                                     grn_ctx *context,
                                                     grn_table_cursor *cursor,
                                                     grn_bool owner);
VALUE          rb_grn_table_cursor_to_ruby_class    (grn_table_cursor *cursor);
void           rb_grn_table_cursor_deconstruct      (RbGrnTableCursor *rb_grn_table_cursor,
                                                     grn_table_cursor **cursor,
                                                     grn_ctx **context,
                                                     grn_id *domain_id,
                                                     grn_obj **domain,
                                                     grn_id *range_id,
                                                     grn_obj **range);

grn_obj       *rb_grn_column_from_ruby_object       (VALUE object,
                                                     grn_ctx **context);
VALUE          rb_grn_column_to_ruby_object         (VALUE klass,
                                                     grn_ctx *context,
                                                     grn_obj *column,
                                                     grn_bool owner);
VALUE          rb_grn_index_cursor_to_ruby_object   (grn_ctx *context,
                                                     grn_obj *cursor,
                                                     VALUE rb_table,
                                                     VALUE rb_lexicon,
                                                     grn_bool owner);
VALUE          rb_grn_inverted_index_cursor_to_ruby_object
                                                    (grn_ctx *context,
                                                     grn_ii_cursor *cursor,
                                                     grn_id term_id,
                                                     int flags,
                                                     VALUE rb_table,
                                                     VALUE rb_lexicon);

grn_operator   rb_grn_operator_from_ruby_object     (VALUE object);
grn_operator   rb_grn_set_operator_from_ruby_object (VALUE object);

grn_logger_info *
               rb_grn_logger_from_ruby_object       (VALUE object);

grn_obj       *rb_grn_bulk_from_ruby_object         (VALUE object,
                                                     grn_ctx *context,
                                                     grn_obj *bulk);
grn_obj       *rb_grn_bulk_from_ruby_object_with_type
                                                    (VALUE object,
                                                     grn_ctx *context,
                                                     grn_obj *bulk,
                                                     grn_id type_id,
                                                     grn_obj *type);
VALUE          rb_grn_bulk_to_ruby_object           (grn_ctx *context,
                                                     grn_obj *bulk,
                                                     grn_obj *range,
                                                     VALUE related_object);
grn_obj       *rb_grn_pvector_from_ruby_object      (VALUE object,
                                                     grn_ctx *context,
                                                     grn_obj *pvector);
VALUE          rb_grn_pvector_to_ruby_object        (grn_ctx *context,
                                                     grn_obj *pvector);
grn_obj       *rb_grn_vector_from_ruby_object       (VALUE object,
                                                     grn_ctx *context,
                                                     grn_obj *vector);
VALUE          rb_grn_vector_to_ruby_object         (grn_ctx *context,
                                                     grn_obj *vector);
grn_obj       *rb_grn_uvector_from_ruby_object      (VALUE object,
                                                     grn_ctx *context,
                                                     grn_obj *uvector,
                                                     VALUE related_object);
VALUE          rb_grn_uvector_to_ruby_object        (grn_ctx *context,
                                                     grn_obj *uvector,
                                                     grn_obj *range,
                                                     VALUE related_object);

VALUE          rb_grn_value_to_ruby_object          (grn_ctx *context,
                                                     grn_obj *value,
                                                     grn_obj *range,
                                                     VALUE related_object);

grn_id         rb_grn_id_from_ruby_object           (VALUE object,
                                                     grn_ctx *context,
                                                     grn_obj *table,
                                                     VALUE related_object);

VALUE          rb_grn_key_to_ruby_object            (grn_ctx *context,
                                                     const void *key,
                                                     int key_size,
                                                     grn_obj *table,
                                                     VALUE related_object);
grn_obj       *rb_grn_key_from_ruby_object          (VALUE rb_key,
                                                     grn_ctx *context,
                                                     grn_obj *key,
                                                     grn_id domain_id,
                                                     grn_obj *domain,
                                                     VALUE related_object);
grn_obj       *rb_grn_value_from_ruby_object        (VALUE rb_key,
                                                     grn_ctx *context,
                                                     grn_obj *value,
                                                     grn_id domain_id,
                                                     grn_obj *domain);

VALUE          rb_grn_variable_to_ruby_object       (grn_ctx *context,
                                                     grn_obj *variable);
grn_obj       *rb_grn_variable_from_ruby_object     (VALUE rb_variable,
                                                     grn_ctx **context);

grn_obj       *rb_grn_obj_from_ruby_object          (VALUE rb_object,
                                                     grn_ctx *context,
                                                     grn_obj **obj);
VALUE          rb_grn_obj_to_ruby_object            (VALUE klass,
                                                     grn_ctx *context,
                                                     grn_obj *obj,
                                                     VALUE related_object);

void          *rb_grn_request_timer_id_from_ruby_object
                                                    (VALUE rb_id);
VALUE          rb_grn_request_timer_id_to_ruby_object
                                                    (void *id);

void           rb_grn_snippet_bind                  (RbGrnSnippet *rb_grn_snippet,
                                                     grn_ctx *context,
                                                     grn_obj *snippet);
void           rb_grn_snippet_finalizer             (grn_ctx *context,
                                                     grn_obj *grn_object,
                                                     RbGrnSnippet *rb_grn_snippet);
RB_GRN_END_DECLS

#endif
