/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2022  Sutou Kouhei <kou@clear-code.com>
  Copyright (C) 2014-2016  Masafumi Yokoyama <yokoyama@clear-code.com>
  Copyright (C) 2019  Horimoto Yasuhiro <horimoto@clear-code.com>

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

/*
 * Document-class: Groonga::Object
 *
 * rroongaが提供するクラスのベースとなるクラス。
 * {Groonga::Context} と {Groonga::Logger} 以外はGroonga::Objectを継
 * 承している。
 */

#include "rb-grn.h"

#define SELF(object) ((RbGrnObject *)RTYPEDDATA_DATA(object))

VALUE rb_cGrnObject;

static void
rb_grn_object_dfree (void *pointer)
{
    rb_grn_object_free(pointer);
}

rb_data_type_t rb_grn_object_data_type = {
    "Groonga::Object",
    {
        NULL,
        rb_grn_object_dfree,
        NULL,
    },
    NULL,
    NULL,
    RUBY_TYPED_FREE_IMMEDIATELY
};

grn_obj *
rb_grn_object_from_ruby_object (VALUE object, grn_ctx **context)
{
    RbGrnObject *rb_grn_object;

    if (NIL_P(object))
        return NULL;

    if (context && *context) {
        grn_obj *grn_object;
        if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cString))) {
            const char *name;
            unsigned int name_size;

            name = StringValuePtr(object);
            name_size = RSTRING_LEN(object);
            grn_object = rb_grn_context_get_backward_compatibility(*context,
                                                                   name,
                                                                   name_size);
            rb_grn_context_check(*context, object);
            if (!grn_object)
                rb_raise(rb_eArgError,
                         "unregistered Groonga object: name: <%s>",
                         rb_grn_inspect(object));
            return grn_object;
        } else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cInteger))) {
            grn_object = grn_ctx_at(*context, NUM2UINT(object));
            rb_grn_context_check(*context, object);
            if (!grn_object)
                rb_raise(rb_eArgError,
                         "unregistered Groonga object: ID: <%s>",
                         rb_grn_inspect(object));
            return grn_object;
        }
    }

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnObject))) {
        rb_raise(rb_eTypeError, "not a Groonga object: <%s>",
                 rb_grn_inspect(object));
    }

    TypedData_Get_Struct(object,
                         RbGrnObject,
                         &rb_grn_object_data_type,
                         rb_grn_object);
    if (!rb_grn_object)
        rb_raise(rb_eGrnError, "Groonga object is NULL");

    if (!rb_grn_object->object) {
        rb_raise(rb_eGrnClosed,
                 "can't access already closed Groonga object: %s",
                 rb_grn_inspect(CLASS_OF(rb_grn_object->self)));
    }

    if (context && !*context)
        *context = rb_grn_object->context;

    return rb_grn_object->object;
}

static void
rb_grn_object_unbind (RbGrnObject *rb_grn_object)
{
    debug("unbind: %p:%p:%p %s(%#x)\n",
          rb_grn_object->context, rb_grn_object->object, rb_grn_object,
          grn_obj_type_to_string(rb_grn_object->object->header.type),
          rb_grn_object->object->header.type);

    rb_grn_object->rb_grn_context = NULL;
    rb_grn_object->context = NULL;
    rb_grn_object->object = NULL;
}

static void
rb_grn_object_run_finalizer (grn_ctx *context, grn_obj *grn_object,
                             RbGrnObject *rb_grn_object)
{
    RbGrnContext *rb_grn_context = NULL;

    if (rb_grn_exited)
        return;

    grn_obj_set_finalizer(context, grn_object, NULL);

    debug("finalize: %p:%p:%p:%p:%p:%p %s(%#x)\n",
          context, grn_object, rb_grn_object,
          rb_grn_object->context, rb_grn_object->object,
          rb_grn_object->rb_grn_context,
          grn_obj_type_to_string(grn_object->header.type),
          grn_object->header.type);

    rb_grn_context = rb_grn_object->rb_grn_context;
    rb_grn_object->have_finalizer = GRN_FALSE;

    switch (grn_object->header.type) {
    case GRN_DB:
        rb_grn_database_finalizer(context, rb_grn_context,
                                  grn_object, rb_grn_object);
        break;
    case GRN_TYPE:
    case GRN_PROC:
    case GRN_CURSOR_TABLE_HASH_KEY:
    case GRN_CURSOR_TABLE_PAT_KEY:
    case GRN_CURSOR_TABLE_DAT_KEY:
    case GRN_CURSOR_TABLE_NO_KEY:
        break;
    case GRN_TABLE_HASH_KEY:
    case GRN_TABLE_PAT_KEY:
        rb_grn_table_key_support_finalizer(context, grn_object,
                                           RB_GRN_TABLE_KEY_SUPPORT(rb_grn_object));
        break;
    case GRN_TABLE_DAT_KEY:
        rb_grn_double_array_trie_finalizer(context, grn_object,
                                           RB_GRN_DOUBLE_ARRAY_TRIE(rb_grn_object));
        break;
    case GRN_TABLE_NO_KEY:
        rb_grn_table_finalizer(context, grn_object,
                               RB_GRN_TABLE(rb_grn_object));
        break;
    case GRN_CURSOR_COLUMN_INDEX:
        break;
    case GRN_COLUMN_FIX_SIZE:
    case GRN_COLUMN_VAR_SIZE:
        rb_grn_column_finalizer(context, grn_object,
                                RB_GRN_COLUMN(rb_grn_object));
        break;
    case GRN_COLUMN_INDEX:
        rb_grn_index_column_finalizer(context, grn_object,
                                      RB_GRN_INDEX_COLUMN(rb_grn_object));
        break;
    case GRN_ACCESSOR:
        rb_grn_accessor_finalizer(context, grn_object,
                                  RB_GRN_ACCESSOR(rb_grn_object));
        break;
    case GRN_EXPR:
        rb_grn_expression_finalizer(context, grn_object,
                                    RB_GRN_EXPRESSION(rb_grn_object));
        break;
    case GRN_SNIP:
        rb_grn_snippet_finalizer(context, grn_object,
                                 RB_GRN_SNIPPET(rb_grn_object));
        break;
    default:
        rb_raise(rb_eTypeError,
                 "unsupported Groonga object type for finalizer: %s(%#x)",
                 grn_obj_type_to_string(grn_object->header.type),
                 grn_object->header.type);
        break;
    }

    rb_grn_object_unbind(rb_grn_object);
}

static grn_obj *
rb_grn_object_finalizer (grn_ctx *context, int n_args, grn_obj **grn_objects,
                         grn_user_data *user_data)
{
    RbGrnObject *rb_grn_object;
    grn_obj *grn_object = *grn_objects;

    if (rb_grn_exited)
        return NULL;

    rb_grn_object = user_data->ptr;

    grn_obj_user_data(context, grn_object)->ptr = NULL;
    rb_grn_object_run_finalizer(context, grn_object, rb_grn_object);

    return NULL;
}

void
rb_grn_object_free (RbGrnObject *rb_grn_object)
{
    grn_ctx *context;
    grn_obj *grn_object;

    context = rb_grn_object->context;
    grn_object = rb_grn_object->object;
    debug("rb-free: %p:%p:%p; %d:%d\n", context, grn_object, rb_grn_object,
          rb_grn_object->have_finalizer, rb_grn_object->need_close);
    if (!rb_grn_exited && context && grn_object &&
        (rb_grn_object->have_finalizer || rb_grn_object->need_close)) {
        grn_user_data *user_data = NULL;

        if (rb_grn_object->have_finalizer) {
            user_data = grn_obj_user_data(context, grn_object);
        }
        debug("type: %s(%#x); need_close: %d; user_data: %p; ptr: %p\n",
              grn_obj_type_to_string(grn_object->header.type),
              grn_object->header.type,
              rb_grn_object->need_close,
              user_data,
              user_data ? user_data->ptr : NULL);
        if (rb_grn_object->have_finalizer) {
            if (user_data && user_data->ptr) {
                rb_grn_object_finalizer(context, 1, &grn_object, user_data);
            } else {
                rb_grn_object_run_finalizer(context, grn_object, rb_grn_object);
            }
        } else {
            rb_grn_object_unbind(rb_grn_object);
        }
        if (rb_grn_object->need_close) {
            grn_obj_unlink(context, grn_object);
        }
    }
    xfree(rb_grn_object);
}

VALUE
rb_grn_object_to_ruby_class (grn_obj *object)
{
    VALUE klass = Qnil;

    switch (object->header.type) {
      case GRN_DB:
        klass = rb_cGrnDatabase;
        break;
      case GRN_TABLE_HASH_KEY:
        klass = rb_cGrnHash;
        break;
      case GRN_TABLE_PAT_KEY:
        klass = rb_cGrnPatriciaTrie;
        break;
      case GRN_TABLE_DAT_KEY:
        klass = rb_cGrnDoubleArrayTrie;
        break;
      case GRN_TABLE_NO_KEY:
        klass = rb_cGrnArray;
        break;
      case GRN_TYPE:
        klass = rb_cGrnType;
        break;
      case GRN_ACCESSOR:
        klass = rb_cGrnAccessor;
        break;
      case GRN_SNIP:
        klass = rb_cGrnSnippet;
        break;
      case GRN_PROC:
        klass = rb_cGrnProcedure;
        break;
      case GRN_COLUMN_FIX_SIZE:
        klass = rb_cGrnFixSizeColumn;
        break;
      case GRN_COLUMN_VAR_SIZE:
        klass = rb_cGrnVariableSizeColumn;
        break;
      case GRN_COLUMN_INDEX:
        klass = rb_cGrnIndexColumn;
        break;
      case GRN_EXPR:
        klass = rb_cGrnExpression;
        break;
      case GRN_CURSOR_TABLE_HASH_KEY:
        klass = rb_cGrnHashCursor;
        break;
      case GRN_CURSOR_TABLE_PAT_KEY:
        klass = rb_cGrnPatriciaTrieCursor;
        break;
      case GRN_CURSOR_TABLE_DAT_KEY:
        klass = rb_cGrnDoubleArrayTrieCursor;
        break;
      case GRN_CURSOR_TABLE_NO_KEY:
        klass = rb_cGrnArrayCursor;
        break;
      default:
        rb_raise(rb_eTypeError,
                 "unsupported Groonga object type for class detection: 0x%x",
                 object->header.type);
        break;
    }

    return klass;
}

VALUE
rb_grn_object_to_ruby_object (VALUE klass, grn_ctx *context, grn_obj *object,
                              grn_bool owner)
{
    RbGrnContext *rb_grn_context;
    VALUE rb_object, rb_context = Qnil;
    grn_user_data *user_data;

    if (!object)
        return Qnil;

    user_data = grn_obj_user_data(context, object);
    if (user_data && user_data->ptr) {
        return RB_GRN_OBJECT(user_data->ptr)->self;
    }

    if (NIL_P(klass))
        klass = GRNOBJECT2RCLASS(object);

    rb_grn_context = GRN_CTX_USER_DATA(context)->ptr;
    if (rb_grn_context)
        rb_context = rb_grn_context->self;
    rb_object = rb_obj_alloc(klass);
    rb_grn_object_assign(klass, rb_object, rb_context, context, object);

    switch (object->header.type) {
    case GRN_ACCESSOR:
    case GRN_SNIP:
    case GRN_TABLE_NO_KEY:
    case GRN_TABLE_HASH_KEY:
    case GRN_TABLE_PAT_KEY:
    case GRN_TABLE_DAT_KEY:
        rb_grn_context_object_created(rb_context, rb_object);
        break;
    default:
        break;
    }

    return rb_object;
}

VALUE
rb_grn_object_alloc (VALUE klass)
{
    return TypedData_Wrap_Struct(klass,
                                 &rb_grn_object_data_type,
                                 NULL);
}

static void
rb_grn_object_bind_common (VALUE klass, VALUE self, VALUE rb_context,
                           RbGrnObject *rb_grn_object,
                           grn_ctx *context, grn_obj *object)
{
    grn_user_data *user_data;

    debug("bind: %p:%p:%p %s(%#x)\n",
          context, object, rb_grn_object,
          grn_obj_type_to_string(object->header.type),
          object->header.type);

    rb_grn_object->rb_grn_context = rb_grn_context_get_struct(rb_context);
    rb_grn_object->context = context;
    rb_grn_object->object = object;
    rb_grn_object->self = self;
    rb_grn_object->need_close = GRN_TRUE;
    rb_grn_object->have_finalizer = GRN_FALSE;
    rb_grn_object->floating = GRN_FALSE;

    user_data = grn_obj_user_data(context, object);
    if (user_data) {
        debug("set-finalizer: %p:%p:%p %s(%#x)\n",
              context, object, rb_grn_object,
              grn_obj_type_to_string(object->header.type),
              object->header.type);
        user_data->ptr = rb_grn_object;
        grn_obj_set_finalizer(context, object, rb_grn_object_finalizer);
        rb_grn_object->have_finalizer = GRN_TRUE;
    } else if (object->header.type == GRN_ACCESSOR) {
        debug("set-finalizer(implicit): %p:%p:%p %s(%#x)\n",
              context, object, rb_grn_object,
              grn_obj_type_to_string(object->header.type),
              object->header.type);
        rb_grn_object->have_finalizer = GRN_TRUE;
    }

    switch (object->header.type) {
      case GRN_PROC:
      case GRN_TYPE:
        rb_grn_object->need_close = GRN_FALSE;
        break;
      default:
        if (klass == rb_cGrnVariable)
            rb_grn_object->need_close = GRN_FALSE;
        break;
    }

    rb_grn_object->domain_id = GRN_ID_NIL;
    if (object)
        rb_grn_object->domain_id = object->header.domain;
    if (rb_grn_object->domain_id == GRN_ID_NIL)
        rb_grn_object->domain = NULL;
    else
        rb_grn_object->domain = grn_ctx_at(context, rb_grn_object->domain_id);

    rb_grn_object->range_id = GRN_ID_NIL;
    if (object && object->header.type != GRN_TYPE)
        rb_grn_object->range_id = grn_obj_get_range(context, object);
    if (rb_grn_object->range_id == GRN_ID_NIL)
        rb_grn_object->range = NULL;
    else
        rb_grn_object->range = grn_ctx_at(context, rb_grn_object->range_id);

    RTYPEDDATA_DATA(self) = rb_grn_object;
}

void
rb_grn_object_assign (VALUE klass, VALUE self, VALUE rb_context,
                      grn_ctx *context, grn_obj *object)
{
    void *rb_grn_object;

    if (!object)
        return;

    if (NIL_P(klass))
        klass = rb_obj_class(self);

    if (klass == rb_cGrnDatabase ||
        (RVAL2CBOOL(rb_obj_is_kind_of(self, rb_cGrnType))) ||
        klass == rb_cGrnHashCursor ||
        klass == rb_cGrnPatriciaTrieCursor ||
        klass == rb_cGrnDoubleArrayTrieCursor ||
        klass == rb_cGrnArrayCursor ||
        klass == rb_cGrnIndexCursor ||
        klass == rb_cGrnProcedure ||
        klass == rb_cGrnVariable) {
        rb_grn_object = ALLOC(RbGrnObject);
        rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
                                  context, object);
    } else if (klass == rb_cGrnDoubleArrayTrie) {
        rb_grn_object = ALLOC(RbGrnDoubleArrayTrie);
        rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
                                  context, object);
        rb_grn_double_array_trie_bind(RB_GRN_DOUBLE_ARRAY_TRIE(rb_grn_object),
                                      context, object);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(self, rb_mGrnTableKeySupport))) {
        rb_grn_object = ALLOC(RbGrnTableKeySupport);
        rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
                                  context, object);
        rb_grn_table_key_support_bind(RB_GRN_TABLE_KEY_SUPPORT(rb_grn_object),
                                      context, object);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(self, rb_cGrnTable))) {
        rb_grn_object = ALLOC(RbGrnTable);
        rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
                                  context, object);
        rb_grn_table_bind(RB_GRN_TABLE(rb_grn_object), context, object);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(self, rb_cGrnIndexColumn))) {
        rb_grn_object = ALLOC(RbGrnIndexColumn);
        rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
                                  context, object);
        rb_grn_index_column_bind(RB_GRN_INDEX_COLUMN(rb_grn_object),
                                 context, object);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(self, rb_cGrnVariableSizeColumn))) {
        rb_grn_object = ALLOC(RbGrnVariableSizeColumn);
        rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
                                  context, object);
        rb_grn_variable_size_column_bind(RB_GRN_VARIABLE_SIZE_COLUMN(rb_grn_object),
                                         context,
                                         object);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(self, rb_cGrnColumn))) {
        rb_grn_object = ALLOC(RbGrnColumn);
        rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
                                  context, object);
        rb_grn_column_bind(RB_GRN_COLUMN(rb_grn_object), context, object);
    } else if (klass == rb_cGrnAccessor) {
        rb_grn_object = ALLOC(RbGrnAccessor);
        rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
                                  context, object);
        rb_grn_accessor_bind(RB_GRN_ACCESSOR(rb_grn_object), context, object);
    } else if (klass == rb_cGrnExpression) {
        rb_grn_object = ALLOC(RbGrnExpression);
        rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
                                  context, object);
        rb_grn_expression_bind(RB_GRN_EXPRESSION(rb_grn_object),
                               context, object);
    } else if (klass == rb_cGrnSnippet) {
        rb_grn_object = ALLOC(RbGrnSnippet);
        rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
                                  context, object);
        rb_grn_snippet_bind(RB_GRN_SNIPPET(rb_grn_object),
                            context, object);
    } else {
        rb_raise(rb_eTypeError,
                 "unsupported Groonga object type for assignment: %s(%#x)",
                 grn_obj_type_to_string(object->header.type),
                 object->header.type);
    }

    rb_iv_set(self, "@context", rb_context);

    debug("assign: %p:%p:%p %s(%#x)\n",
          context, object, rb_grn_object,
          grn_obj_type_to_string(object->header.type), object->header.type);
}

void
rb_grn_named_object_bind (RbGrnNamedObject *rb_grn_named_object,
                          grn_ctx *context, grn_obj *object)
{
    rb_grn_named_object->name = NULL;
    rb_grn_named_object->name_size = 0;
}

void
rb_grn_named_object_finalizer (grn_ctx *context, grn_obj *grn_object,
                               RbGrnNamedObject *rb_grn_named_object)
{
    if (rb_grn_named_object->name)
        xfree(rb_grn_named_object->name);
    rb_grn_named_object->name = NULL;
    rb_grn_named_object->name_size = 0;
}

void
rb_grn_named_object_set_name (RbGrnNamedObject *rb_grn_named_object,
                              const char *name, unsigned name_size)
{
    if (rb_grn_named_object->name) {
        xfree(rb_grn_named_object->name);
        rb_grn_named_object->name = NULL;
    }
    if (name_size > 0) {
        rb_grn_named_object->name = ALLOC_N(char, name_size + 1);
        memcpy(rb_grn_named_object->name, name, name_size);
        rb_grn_named_object->name[name_size] = '\0';
        debug("set-name: %p:%p:%p %s(%#x): <%.*s>\n",
              RB_GRN_OBJECT(rb_grn_named_object)->context,
              RB_GRN_OBJECT(rb_grn_named_object)->object,
              rb_grn_named_object,
              grn_obj_type_to_string(RB_GRN_OBJECT(rb_grn_named_object)->object->header.type),
              RB_GRN_OBJECT(rb_grn_named_object)->object->header.type,
              name_size, name);
    }
    rb_grn_named_object->name_size = name_size;
}


void
rb_grn_object_deconstruct (RbGrnObject *rb_grn_object,
                           grn_obj **object,
                           grn_ctx **context,
                           grn_id *domain_id,
                           grn_obj **domain,
                           grn_id *range_id,
                           grn_obj **range)
{
    if (!rb_grn_object)
        return;

    if (object)
        *object = rb_grn_object->object;
    if (context)
        *context = rb_grn_object->context;
    if (domain_id)
        *domain_id = rb_grn_object->domain_id;
    if (domain)
        *domain = rb_grn_object->domain;
    if (range_id)
        *range_id = rb_grn_object->range_id;
    if (range)
        *range = rb_grn_object->range;
}

void
rb_grn_object_close_raw (RbGrnObject *rb_grn_object)
{
    grn_obj *object;
    grn_ctx *context;

    rb_grn_object_deconstruct(rb_grn_object, &object, &context,
                              NULL, NULL, NULL, NULL);
    debug("object:close: %p:%p:%s\n",
          context,
          object,
          object ? grn_obj_type_to_string(object->header.type) : "(null)");
    if (object && context) {
        if (rb_grn_object->have_finalizer) {
            rb_grn_object_run_finalizer(context, object, rb_grn_object);
        } else {
            rb_grn_object_unbind(rb_grn_object);
        }
        grn_obj_close(context, object);
    }
    debug("object:close: %p:%p: done\n", context, object);
}

/*
 * _object_ が使用しているリソースを開放する。これ以降 _object_ を
 * 使うことはできない。
 *
 * @overload close
 */
VALUE
rb_grn_object_close (VALUE self)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = SELF(self);
    rb_grn_object_close_raw(rb_grn_object);

    return Qnil;
}

/*
 * _object_ のリファレンスカウンタを1減少する。
 *
 * @overload unlink
 */
VALUE
rb_grn_object_unlink (VALUE self)
{
    RbGrnObject *rb_grn_object;
    grn_obj *object;
    grn_ctx *context;

    rb_grn_object = SELF(self);
    rb_grn_object_deconstruct(rb_grn_object, &object, &context,
                              NULL, NULL, NULL, NULL);
    if (object && context) {
        if (!(rb_grn_object->object->header.flags & GRN_OBJ_PERSISTENT)) {
            rb_grn_object_run_finalizer(context, object, rb_grn_object);
            rb_grn_object_unbind(rb_grn_object);
        }
        grn_obj_unlink(context, object);
    }

    return Qnil;
}

/*
 * _object_ が開放済みの場合は +true+ を返し、そうでない場合は
 * +false+ を返す。
 *
 * @overload closed?
 */
VALUE
rb_grn_object_closed_p (VALUE self)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = SELF(self);
    if (rb_grn_object->context && rb_grn_object->object) {
        return Qfalse;
    } else {
        return Qtrue;
    }
}

VALUE
rb_grn_object_inspect_object (VALUE inspected, grn_ctx *context, grn_obj *object)
{
    VALUE rb_object;

    rb_object = GRNOBJECT2RVAL(Qnil, context, object, GRN_FALSE);
    rb_str_concat(inspected, rb_inspect(rb_object));

    return inspected;
}

VALUE
rb_grn_object_inspect_header (VALUE self, VALUE inspected)
{
    rb_str_cat2(inspected, "#<");
    rb_str_concat(inspected, rb_inspect(rb_obj_class(self)));

    return inspected;
}

static VALUE
rb_grn_object_inspect_content_id_with_label (VALUE inspected,
                                             grn_ctx *context, grn_obj *object)
{
    grn_id id;

    rb_str_cat2(inspected, "id: <");
    id = grn_obj_id(context, object);
    if (id == GRN_ID_NIL)
        rb_str_cat2(inspected, "nil");
    else
        rb_str_concat(inspected, rb_obj_as_string(UINT2NUM(id)));
    rb_str_cat2(inspected, ">");

    return inspected;
}

VALUE
rb_grn_object_inspect_object_content_name (VALUE inspected,
                                           grn_ctx *context, grn_obj *object)
{
    char name[GRN_TABLE_MAX_KEY_SIZE];
    int name_size;

    name_size = grn_obj_name(context, object, name, GRN_TABLE_MAX_KEY_SIZE);
    if (name_size == 0) {
        rb_str_cat2(inspected, "(anonymous)");
    } else {
        rb_str_cat2(inspected, "<");
        rb_str_cat(inspected, name, name_size);
        rb_str_cat2(inspected, ">");
    }

    return inspected;
}

static VALUE
rb_grn_object_inspect_content_name_with_label (VALUE inspected,
                                               grn_ctx *context, grn_obj *object)
{

    rb_str_cat2(inspected, "name: ");
    rb_grn_object_inspect_object_content_name(inspected, context, object);
    return inspected;
}

static VALUE
rb_grn_object_inspect_content_path_with_label (VALUE inspected,
                                               grn_ctx *context, grn_obj *object)
{
    const char *path;

    rb_str_cat2(inspected, "path: ");
    path = grn_obj_path(context, object);
    if (path) {
        rb_str_cat2(inspected, "<");
        rb_str_cat2(inspected, path);
        rb_str_cat2(inspected, ">");
    } else {
        rb_str_cat2(inspected, "(temporary)");
    }

    return inspected;
}

static VALUE
rb_grn_object_inspect_content_domain_with_label (VALUE inspected,
                                                 grn_ctx *context,
                                                 grn_obj *object)
{
    grn_id domain;

    rb_str_cat2(inspected, "domain: ");
    domain = object->header.domain;
    if (domain == GRN_ID_NIL) {
        rb_str_cat2(inspected, "(nil)");
    } else {
        grn_obj *domain_object;

        domain_object = grn_ctx_at(context, domain);
        if (domain_object) {
            if (domain_object == object) {
                rb_str_cat2(inspected, "(self)");
            } else {
                rb_grn_object_inspect_object_content_name(inspected,
                                                          context,
                                                          domain_object);
            }
        } else {
            rb_str_cat2(inspected, "(");
            rb_str_concat(inspected, rb_obj_as_string(UINT2NUM(domain)));
            rb_str_cat2(inspected, ")");
        }
    }

    return inspected;
}

static VALUE
rb_grn_object_inspect_content_range_with_label (VALUE inspected,
                                                grn_ctx *context,
                                                grn_obj *object)
{
    grn_id range;

    rb_str_cat2(inspected, "range: ");

    range = grn_obj_get_range(context, object);
    switch (object->header.type) {
      case GRN_TYPE:
        rb_str_cat2(inspected, "<");
        rb_str_concat(inspected, rb_inspect(UINT2NUM(range)));
        rb_str_cat2(inspected, ">");
        break;
      default:
        if (range == GRN_ID_NIL) {
            rb_str_cat2(inspected, "(nil)");
        } else {
            grn_obj *range_object;

            range_object = grn_ctx_at(context, range);
            if (range_object) {
                if (range_object == object) {
                    rb_str_cat2(inspected, "(self)");
                } else {
                    rb_grn_object_inspect_object_content_name(inspected,
                                                              context,
                                                              range_object);
                }
            } else {
                rb_str_cat2(inspected, "(");
                rb_str_concat(inspected, rb_obj_as_string(UINT2NUM(range)));
                rb_str_cat2(inspected, ")");
            }
        }
    }

    return inspected;
}

static VALUE
rb_grn_object_inspect_content_flags_with_label (VALUE inspected,
                                                grn_ctx *context,
                                                grn_obj *object)
{
    grn_obj_flags flags;
    grn_column_flags column_flags = 0;
    VALUE inspected_flags;

    rb_str_cat2(inspected, "flags: ");

    flags = object->header.flags;
    switch (object->header.type) {
    case GRN_COLUMN_FIX_SIZE:
    case GRN_COLUMN_VAR_SIZE:
    case GRN_COLUMN_INDEX:
        column_flags = grn_column_get_flags(context, object);
        break;
    default:
        break;
    }

    inspected_flags = rb_ary_new();

    if (0) {
        if (flags & GRN_OBJ_TABLE_HASH_KEY)
            rb_ary_push(inspected_flags, rb_str_new_cstr("TABLE_HASH_KEY"));
        if (flags & GRN_OBJ_TABLE_PAT_KEY)
            rb_ary_push(inspected_flags, rb_str_new_cstr("TABLE_PAT_KEY"));
        if (flags & GRN_OBJ_TABLE_DAT_KEY)
            rb_ary_push(inspected_flags, rb_str_new_cstr("TABLE_DAT_KEY"));
        if (flags & GRN_OBJ_TABLE_NO_KEY)
            rb_ary_push(inspected_flags, rb_str_new_cstr("TABLE_NO_KEY"));
    }

    switch (object->header.type) {
    case GRN_TYPE:
        if (flags & GRN_OBJ_KEY_VAR_SIZE) {
            rb_ary_push(inspected_flags, rb_str_new_cstr("KEY_VAR_SIZE"));
        } else {
            switch (flags & GRN_OBJ_KEY_MASK) {
            case GRN_OBJ_KEY_UINT:
                rb_ary_push(inspected_flags, rb_str_new_cstr("KEY_UINT"));
                break;
            case GRN_OBJ_KEY_INT:
                rb_ary_push(inspected_flags, rb_str_new_cstr("KEY_INT"));
                break;
            case GRN_OBJ_KEY_FLOAT:
                rb_ary_push(inspected_flags, rb_str_new_cstr("KEY_FLOAT"));
                break;
            case GRN_OBJ_KEY_GEO_POINT:
                rb_ary_push(inspected_flags, rb_str_new_cstr("KEY_GEO_POINT"));
                break;
            default:
                break;
            }
        }
    default:
        break;
    }

    switch (object->header.type) {
    case GRN_TABLE_HASH_KEY:
    case GRN_TABLE_PAT_KEY:
    case GRN_TABLE_DAT_KEY:
        if (flags & GRN_OBJ_KEY_WITH_SIS)
            rb_ary_push(inspected_flags, rb_str_new_cstr("KEY_WITH_SIS"));
        if (flags & GRN_OBJ_KEY_NORMALIZE)
            rb_ary_push(inspected_flags, rb_str_new_cstr("KEY_NORMALIZE"));
        break;
    default:
        break;
    }

    switch (object->header.type) {
    case GRN_COLUMN_FIX_SIZE:
    case GRN_COLUMN_VAR_SIZE:
        if (column_flags & GRN_OBJ_COLUMN_SCALAR) {
            rb_ary_push(inspected_flags, rb_str_new_cstr("COLUMN_SCALAR"));
        } else if (column_flags & GRN_OBJ_COLUMN_VECTOR) {
            rb_ary_push(inspected_flags, rb_str_new_cstr("COLUMN_VECTOR"));
            if (column_flags & GRN_OBJ_WITH_WEIGHT)
                rb_ary_push(inspected_flags, rb_str_new_cstr("WITH_WEIGHT"));
            if (column_flags & GRN_OBJ_WEIGHT_FLOAT32)
                rb_ary_push(inspected_flags, rb_str_new_cstr("WEIGHT_FLOAT32"));
        }
        switch (column_flags & GRN_OBJ_MISSING_MASK) {
        case GRN_OBJ_MISSING_IGNORE:
            rb_ary_push(inspected_flags, rb_str_new_cstr("MISSING_IGNORE"));
            break;
        case GRN_OBJ_MISSING_NIL:
            rb_ary_push(inspected_flags, rb_str_new_cstr("MISSING_NIL"));
            break;
        default:
            break;
        }
        switch (column_flags & GRN_OBJ_INVALID_MASK) {
        case GRN_OBJ_INVALID_WARN:
            rb_ary_push(inspected_flags, rb_str_new_cstr("INVALID_WARN"));
            break;
        case GRN_OBJ_INVALID_IGNORE:
            rb_ary_push(inspected_flags, rb_str_new_cstr("INVALID_IGNORE"));
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    switch (object->header.type) {
    case GRN_COLUMN_FIX_SIZE:
    case GRN_COLUMN_VAR_SIZE:
        switch (column_flags & GRN_OBJ_COMPRESS_MASK) {
        case GRN_OBJ_COMPRESS_ZLIB:
            rb_ary_push(inspected_flags, rb_str_new_cstr("COMPRESS_ZLIB"));
            break;
        case GRN_OBJ_COMPRESS_LZ4:
            rb_ary_push(inspected_flags, rb_str_new_cstr("COMPRESS_LZ4"));
            break;
        case GRN_OBJ_COMPRESS_ZSTD:
            rb_ary_push(inspected_flags, rb_str_new_cstr("COMPRESS_ZSTD"));
            break;
        default:
            break;
        }
        break;
    case GRN_COLUMN_INDEX:
        if (column_flags & GRN_OBJ_WITH_SECTION)
            rb_ary_push(inspected_flags, rb_str_new_cstr("WITH_SECTION"));
        if (column_flags & GRN_OBJ_WITH_WEIGHT)
            rb_ary_push(inspected_flags, rb_str_new_cstr("WITH_WEIGHT"));
        if (column_flags & GRN_OBJ_WITH_POSITION)
            rb_ary_push(inspected_flags, rb_str_new_cstr("WITH_POSITION"));
        if (column_flags & GRN_OBJ_INDEX_SMALL)
            rb_ary_push(inspected_flags, rb_str_new_cstr("SMALL"));
        if (column_flags & GRN_OBJ_INDEX_MEDIUM)
            rb_ary_push(inspected_flags, rb_str_new_cstr("MEDIUM"));
        if (column_flags & GRN_OBJ_INDEX_LARGE)
            rb_ary_push(inspected_flags, rb_str_new_cstr("LARGE"));
        break;
    default:
        break;
    }

    switch (object->header.type) {
    case GRN_TABLE_NO_KEY:
    case GRN_TABLE_HASH_KEY:
    case GRN_TABLE_PAT_KEY:
    case GRN_TABLE_DAT_KEY:
        if (flags & GRN_OBJ_RING_BUFFER)
            rb_ary_push(inspected_flags, rb_str_new_cstr("RING_BUFFER"));

        if (flags & GRN_OBJ_WITH_SUBREC) {
            rb_ary_push(inspected_flags, rb_str_new_cstr("WITH_SUBREC"));

            if (flags & GRN_OBJ_UNIT_DOCUMENT_SECTION)
                rb_ary_push(inspected_flags,
                            rb_str_new_cstr("UNIT_DOCUMENT_SECTION"));
            if (flags & GRN_OBJ_UNIT_DOCUMENT_POSITION)
                rb_ary_push(inspected_flags,
                            rb_str_new_cstr("UNIT_DOCUMENT_POSITION"));

            if (flags & GRN_OBJ_UNIT_SECTION_POSITION)
                rb_ary_push(inspected_flags,
                            rb_str_new_cstr("UNIT_SECTION_POSITION"));

            if (flags & GRN_OBJ_UNIT_USERDEF_DOCUMENT)
                rb_ary_push(inspected_flags,
                            rb_str_new_cstr("UNIT_USERDEF_DOCUMENT"));
            if (flags & GRN_OBJ_UNIT_USERDEF_SECTION)
                rb_ary_push(inspected_flags,
                            rb_str_new_cstr("UNIT_USERDEF_SECTION"));
            if (flags & GRN_OBJ_UNIT_USERDEF_POSITION)
                rb_ary_push(inspected_flags,
                            rb_str_new_cstr("UNIT_USERDEF_POSITION"));
        }
        break;
    default:
        break;
    }

    rb_str_cat2(inspected, "<");
    rb_str_concat(inspected, rb_ary_join(inspected_flags, rb_str_new_cstr("|")));
    rb_str_cat2(inspected, ">");

    return inspected;
}

VALUE
rb_grn_object_inspect_object_content (VALUE inspected,
                                      grn_ctx *context, grn_obj *object)
{
    rb_grn_object_inspect_content_id_with_label(inspected, context, object);
    rb_str_cat2(inspected, ", ");
    rb_grn_object_inspect_content_name_with_label(inspected, context, object);
    rb_str_cat2(inspected, ", ");
    rb_grn_object_inspect_content_path_with_label(inspected, context, object);
    rb_str_cat2(inspected, ", ");
    rb_grn_object_inspect_content_domain_with_label(inspected, context, object);
    rb_str_cat2(inspected, ", ");
    rb_grn_object_inspect_content_range_with_label(inspected, context, object);
    rb_str_cat2(inspected, ", ");
    rb_grn_object_inspect_content_flags_with_label(inspected, context, object);

    return inspected;
}

VALUE
rb_grn_object_inspect_content (VALUE self, VALUE inspected)
{
    RbGrnObject *rb_grn_object;
    grn_ctx *context;
    grn_obj *object;

    rb_grn_object = SELF(self);
    if (!rb_grn_object)
        return inspected;

    context = rb_grn_object->context;
    object = rb_grn_object->object;

    rb_str_cat2(inspected, " ");
    if (rb_grn_exited) {
        rb_str_cat2(inspected, "(finished)");
    } else if (object) {
        rb_grn_object_inspect_object_content(inspected, context, object);
    } else {
        rb_str_cat2(inspected, "(closed)");
    }

    return inspected;
}

VALUE
rb_grn_object_inspect_footer (VALUE self, VALUE inspected)
{
    rb_str_cat2(inspected, ">");

    return inspected;
}

/*
 * _object_ の詳細を示した文字列を返す。デバッグ用。
 *
 * @overload inspect
 *   @return [詳細情報]
 */
static VALUE
rb_grn_object_inspect (VALUE self)
{
    VALUE inspected;

    inspected = rb_str_new_cstr("");
    rb_grn_object_inspect_header(self, inspected);
    rb_grn_object_inspect_content(self, inspected);
    rb_grn_object_inspect_footer(self, inspected);

    return inspected;
}

/*
 * _object_ のIDを返す。
 * _object_ が {#closed?} なときやIDがない場合は +nil+ を返す。
 *
 * @overload id
 *   @return [ID or nil]
 */
VALUE
rb_grn_object_get_id (VALUE self)
{
    RbGrnObject *rb_grn_object;
    grn_id id;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
        return Qnil;

    id = grn_obj_id(rb_grn_object->context, rb_grn_object->object);
    if (id == GRN_ID_NIL)
        return Qnil;
    else
        return UINT2NUM(id);
}

/*
 * _object_ に対応するファイルパスを返す。一時 _object_
 * なら +nil+ を返す。
 *
 * @overload path
 *   @return [ファイルパス or nil]
 */
static VALUE
rb_grn_object_get_path (VALUE self)
{
    RbGrnObject *rb_grn_object;
    const char *path;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
        return Qnil;

    path = grn_obj_path(rb_grn_object->context, rb_grn_object->object);

    if (!path)
        return Qnil;
    else
        return rb_str_new_cstr(path);
}

/*
 * _object_ が一時オブジェクトなら +true+ 、永続オブジェクトな
 * ら +false+ を返す。
 *
 * @overload temporary?
 */
static VALUE
rb_grn_object_temporary_p (VALUE self)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
        return Qnil;

    return CBOOL2RVAL(!(rb_grn_object->object->header.flags & GRN_OBJ_PERSISTENT));
}

/*
 * _object_ が永続オブジェクトなら +true+ 、一時オブジェクトな
 * ら +false+ を返す。
 *
 * @overload persistent?
 */
static VALUE
rb_grn_object_persistent_p (VALUE self)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
        return Qnil;

    return CBOOL2RVAL(rb_grn_object->object->header.flags & GRN_OBJ_PERSISTENT);
}

/*
 * _object_ の属している {Groonga::Object} を返す。例えば、
 * {Groonga::Column} は {Groonga::Table} を返す。属している
 * {Groonga::Object} がない場合は +nil+ を返す。
 *
 * @overload domain
 *   @return [Groonga::Object or nil]
 */
static VALUE
rb_grn_object_get_domain (VALUE self)
{
    RbGrnObject *rb_grn_object;
    grn_ctx *context;
    grn_obj *object;
    grn_id domain;

    rb_grn_object = SELF(self);
    object = rb_grn_object->object;
    if (!object)
        return Qnil;

    context = rb_grn_object->context;
    domain = object->header.domain;
    if (domain == GRN_ID_NIL) {
        return Qnil;
    } else {
        grn_obj *domain_object;

        domain_object = grn_ctx_at(context, domain);
        if (domain_object)
            return GRNOBJECT2RVAL(Qnil, context, domain_object, GRN_FALSE);
        else
            return UINT2NUM(domain);
    }
}

/*
 * _object_ の名前を返す。無名オブジェクトの場合は +nil+ を返す。
 *
 * @overload name
 *   @return [名前 or nil]
 */
static VALUE
rb_grn_object_get_name (VALUE self)
{
    RbGrnObject *rb_grn_object;
    VALUE rb_name;
    char *name;
    int name_size;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
        return Qnil;

    name_size = grn_obj_name(rb_grn_object->context, rb_grn_object->object,
                             NULL, 0);
    if (name_size == 0)
        return Qnil;

    name = xmalloc(name_size);
    grn_obj_name(rb_grn_object->context, rb_grn_object->object,
                 name, name_size);
    rb_name = rb_str_new(name, name_size);
    xfree(name);

    return rb_name;
}

/*
 * _object_ の値がとりうる範囲を示した {Groonga::Object} を返す。
 * 例えば、 {Groonga::Column} の場合は
 * {Groonga::Table#define_column} で指定された {Groonga::Type} や
 * {Groonga::Table} を返す。
 * 範囲が指定されていないオブジェクトの場合は +nil+ を返す。
 *
 * @overload range
 *   @return [Groonga::Object or nil]
 */
static VALUE
rb_grn_object_get_range (VALUE self)
{
    RbGrnObject *rb_grn_object;
    grn_ctx *context;
    grn_obj *object;
    grn_id range;

    rb_grn_object = SELF(self);
    object = rb_grn_object->object;
    if (!object)
        return Qnil;

    context = rb_grn_object->context;
    range = grn_obj_get_range(context, object);
    if (range == GRN_ID_NIL) {
        return Qnil;
    } else {
        grn_obj *range_object;

        range_object = grn_ctx_at(context, range);
        if (range_object)
            return GRNOBJECT2RVAL(Qnil, context, range_object, GRN_FALSE);
        else
            return UINT2NUM(range);
    }
}

/*
 * _object_ と _other_ が同じGroongaのオブジェクトなら +true+ を返
 * し、そうでなければ +false+ を返す。
 *
 * @overload ==(other)
 *   @return [Boolean]
 */
static VALUE
rb_grn_object_equal (VALUE self, VALUE other)
{
    RbGrnObject *self_rb_grn_object, *other_rb_grn_object;

    if (self == other)
        return Qtrue;

    if (!RVAL2CBOOL(rb_funcall(rb_obj_class(self), rb_intern("=="), 1,
                               rb_obj_class(other))))
        return Qfalse;

    self_rb_grn_object = SELF(self);
    other_rb_grn_object = SELF(other);

    return self_rb_grn_object->object == other_rb_grn_object->object;
}

/*
 * _object_ の _id_ に対応する値を返す。
 *
 * @overload [](id)
 *   @return [値]
 */
VALUE
rb_grn_object_array_reference (VALUE self, VALUE rb_id)
{
    VALUE exception;
    RbGrnObject *rb_grn_object;
    grn_id id, range_id;
    grn_ctx *context;
    grn_obj *object;
    grn_obj *range;
    unsigned char range_type;
    grn_obj value;
    VALUE rb_value = Qnil;

    rb_grn_object = SELF(self);
    context = rb_grn_object->context;
    object = rb_grn_object->object;
    if (!object)
        return Qnil;

    id = NUM2UINT(rb_id);
    range_id = grn_obj_get_range(context, object);
    range = grn_ctx_at(context, range_id);
    range_type = range ? range->header.type : GRN_VOID;
    switch (object->header.type) {
      case GRN_TABLE_HASH_KEY:
      case GRN_TABLE_PAT_KEY:
      case GRN_TABLE_DAT_KEY:
      case GRN_TABLE_NO_KEY:
        GRN_OBJ_INIT(&value, GRN_BULK, 0, GRN_ID_NIL);
        break;
      case GRN_TYPE:
      case GRN_ACCESSOR: /* FIXME */
        GRN_OBJ_INIT(&value, GRN_BULK, 0, range_id);
        break;
      case GRN_COLUMN_VAR_SIZE:
      case GRN_COLUMN_FIX_SIZE:
          {
              grn_column_flags column_flags;
              column_flags = grn_column_get_flags(context, object);
              switch (column_flags & GRN_OBJ_COLUMN_TYPE_MASK) {
              case GRN_OBJ_COLUMN_VECTOR:
                  GRN_OBJ_INIT(&value, GRN_VECTOR, 0, range_id);
                  break;
              case GRN_OBJ_COLUMN_SCALAR:
                  GRN_OBJ_INIT(&value, GRN_BULK, 0, range_id);
                  break;
              default:
                  rb_raise(rb_eGrnError, "unsupported column type: %u: %s",
                           range_type, rb_grn_inspect(self));
                  break;
              }
          }
          break;
      case GRN_COLUMN_INDEX:
        GRN_UINT32_INIT(&value, 0);
        break;
      default:
        rb_raise(rb_eGrnError,
                 "unsupported type: %s", rb_grn_inspect(self));
        break;
    }

    grn_obj_get_value(context, object, id, &value);
    exception = rb_grn_context_to_exception(context, self);
    if (NIL_P(exception))
        rb_value = GRNVALUE2RVAL(context, &value, range, self);
    grn_obj_unlink(context, &value);
    if (!NIL_P(exception))
        rb_exc_raise(exception);

    return rb_value;
}

static bool
rb_uvector_value_p (RbGrnObject *rb_grn_object, VALUE rb_value)
{
    VALUE first_element;

    switch (rb_grn_object->range->header.type) {
      case GRN_TYPE:
        if (!(rb_grn_object->range->header.flags & GRN_OBJ_KEY_VAR_SIZE)) {
            return true;
        }
        break;
      case GRN_TABLE_HASH_KEY:
      case GRN_TABLE_PAT_KEY:
      case GRN_TABLE_DAT_KEY:
      case GRN_TABLE_NO_KEY:
        first_element = rb_ary_entry(rb_value, 0);
        if (rb_respond_to(first_element, rb_intern("record_raw_id"))) {
            return GRN_TRUE;
        }
        break;
      default:
        break;
    }

    return false;
}

typedef struct {
    RbGrnObject *rb_grn_object;
    grn_id id;
    grn_obj value;
    VALUE rb_value;
    int flags;
    VALUE related_object;
} SetRawData;

static VALUE
rb_grn_object_set_raw_body (VALUE user_data)
{
    SetRawData *data = (SetRawData *)user_data;
    RbGrnObject *rb_grn_object;
    grn_ctx *context;
    grn_obj *value;
    grn_rc rc;
    VALUE rb_value, rb_values;
    VALUE related_object;

    rb_grn_object = data->rb_grn_object;
    context = rb_grn_object->context;
    value = &(data->value);
    rb_value = data->rb_value;
    rb_values = rb_grn_check_convert_to_array(rb_value);
    related_object = data->related_object;
    if (NIL_P(rb_values)) {
        if (NIL_P(rb_value)) {
            GRN_OBJ_INIT(value, GRN_BULK, 0, GRN_ID_NIL);
        } else {
            GRN_OBJ_INIT(value, GRN_BULK, 0, GRN_ID_NIL);
            RVAL2GRNBULK(rb_value, context, value);
        }
    } else {
        if (rb_uvector_value_p(rb_grn_object, rb_values)) {
            GRN_OBJ_INIT(value, GRN_UVECTOR, 0, rb_grn_object->range_id);
            RVAL2GRNUVECTOR(rb_values, context, value, related_object);
        } else {
            GRN_OBJ_INIT(value, GRN_VECTOR, 0, GRN_ID_NIL);
            RVAL2GRNVECTOR(rb_values, context, value);
        }
    }
    rc = grn_obj_set_value(context, rb_grn_object->object, data->id,
                           value, data->flags);
    rb_grn_context_check(context, related_object);
    rb_grn_rc_check(rc, related_object);

    return Qnil;
}

static VALUE
rb_grn_object_set_raw_ensure (VALUE user_data)
{
    SetRawData *data = (SetRawData *)user_data;

    grn_obj_unlink(data->rb_grn_object->context, &(data->value));

    return Qnil;
}

VALUE
rb_grn_object_set_raw (RbGrnObject *rb_grn_object, grn_id id,
                       VALUE rb_value, int flags, VALUE related_object)
{
    SetRawData data;

    data.rb_grn_object = rb_grn_object;
    data.id = id;
    GRN_VOID_INIT(&(data.value));
    data.rb_value = rb_value;
    data.flags = flags;
    data.related_object = related_object;

    return rb_ensure(rb_grn_object_set_raw_body, (VALUE)(&data),
                     rb_grn_object_set_raw_ensure, (VALUE)(&data));
}

static VALUE
rb_grn_object_set (VALUE self, VALUE rb_id, VALUE rb_value, int flags)
{
    RbGrnObject *rb_grn_object;
    grn_id id;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
        return Qnil;

    id = NUM2UINT(rb_id);

    return rb_grn_object_set_raw(rb_grn_object, id, rb_value, flags, self);
}

/*
 * _object_ の _id_ に対応する値を設定する。既存の値は上書きさ
 * れる。
 *
 * @overload []=(id, value)
 */
static VALUE
rb_grn_object_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    return rb_grn_object_set(self, rb_id, rb_value, GRN_OBJ_SET);
}

/*
 * _object_ の _id_ に対応する値の最後に _value_ を追加する。
 *
 * @overload append(id, value)
 */
static VALUE
rb_grn_object_append_value (VALUE self, VALUE rb_id, VALUE rb_value)
{
    return rb_grn_object_set(self, rb_id, rb_value, GRN_OBJ_APPEND);
}

/*
 * _object_ の _id_ に対応する値の最初に _value_ を追加する。
 *
 * @overload prepend(id, value)
 */
static VALUE
rb_grn_object_prepend_value (VALUE self, VALUE rb_id, VALUE rb_value)
{
    return rb_grn_object_set(self, rb_id, rb_value, GRN_OBJ_PREPEND);
}

/*
 * Free the `object`. If the `object` is persistent object, all files
 * related with the object are removed.
 *
 * @overload remove(options={})
 *
 *   @param [::Hash] options The optional parameters.
 *
 *   @option options [Boolean] :dependent (false)
 *     If it's `true`, all tables and columns that depend on the
 *     `object`.
 */
static VALUE
rb_grn_object_remove (int argc, VALUE *argv, VALUE self)
{
    RbGrnObject *rb_grn_object;
    grn_ctx *context;
    VALUE rb_options;
    VALUE rb_dependent_p;
    bool dependent_p = false;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
        return Qnil;

    rb_scan_args(argc, argv, "01", &rb_options);
    rb_grn_scan_options(rb_options,
                        "dependent", &rb_dependent_p,
                        NULL);

    if (!NIL_P(rb_dependent_p)) {
        dependent_p = RVAL2CBOOL(rb_dependent_p);
    }

    context = rb_grn_object->context;
    if (dependent_p) {
        grn_obj_remove_dependent(context, rb_grn_object->object);
    } else {
        grn_obj_remove(context, rb_grn_object->object);
    }
    rb_grn_context_check(context, self);

    rb_iv_set(self, "@context", Qnil);

    return Qnil;
}

static VALUE
rb_grn_object_builtin_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool builtin = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        builtin = grn_obj_is_builtin(context, object);
    }

    return CBOOL2RVAL(builtin);
}

/*
 * Checks whether the object is table or not.
 *
 * @overload table?
 *   @return [Boolean] `true` if the object is table, `false` otherwise.
 *
 * @since 5.0.1
 */
static VALUE
rb_grn_object_table_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool table_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        table_p = grn_obj_is_table(context, object);
    }

    return CBOOL2RVAL(table_p);
}

/*
 * Checks whether the object is column or not.
 *
 * @overload column?
 *   @return [Boolean] `true` if the object is column, `false` otherwise.
 *
 * @since 6.0.5
 */
static VALUE
rb_grn_object_column_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool column_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        column_p = grn_obj_is_column(context, object);
    }

    return CBOOL2RVAL(column_p);
}

/*
 * Checks whether the object is reference column or not.
 *
 * @overload reference_column?
 *   @return [Boolean] `true` if the object is reference column,
 *     `false` otherwise.
 *
 * @since 6.0.5
 */
static VALUE
rb_grn_object_reference_column_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool reference_column_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        reference_column_p = grn_obj_is_reference_column(context, object);
    }

    return CBOOL2RVAL(reference_column_p);
}

/*
 * Checks whether the object is index column or not.
 *
 * @overload index_column?
 *   @return [Boolean] `true` if the object is index column,
 *     `false` otherwise.
 *
 * @since 6.0.5
 */
static VALUE
rb_grn_object_index_column_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool index_column_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        index_column_p = grn_obj_is_index_column(context, object);
    }

    return CBOOL2RVAL(index_column_p);
}

/*
 * Checks whether the object is procedure or not.
 *
 * @overload procedure?
 *   @return [Boolean] `true` if the object is procedure, `false` otherwise.
 *
 * @since 5.0.1
 */
static VALUE
rb_grn_object_procedure_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool procedure_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        procedure_p = grn_obj_is_proc(context, object);
    }

    return CBOOL2RVAL(procedure_p);
}

/*
 * Checks whether the object is function procedure or not.
 *
 * @overload function_procedure?
 *   @return [Boolean] `true` if the object is function procedure,
 *     `false` otherwise.
 *
 * @since 5.0.1
 */
static VALUE
rb_grn_object_function_procedure_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool function_procedure_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        function_procedure_p = grn_obj_is_function_proc(context, object);
    }

    return CBOOL2RVAL(function_procedure_p);
}

/*
 * Checks whether the object is selector procedure or not.
 *
 * @overload selector_procedure?
 *   @return [Boolean] `true` if the object is selector procedure,
 *     `false` otherwise.
 *
 * @since 5.0.1
 */
static VALUE
rb_grn_object_selector_procedure_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool selector_procedure_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        selector_procedure_p = grn_obj_is_selector_proc(context, object);
    }

    return CBOOL2RVAL(selector_procedure_p);
}

/*
 * Checks whether the object is selector only procedure or not.
 *
 * @overload selector_only_procedure?
 *   @return [Boolean] `true` if the object is selector only procedure,
 *     `false` otherwise.
 *
 * @since 5.0.5
 */
static VALUE
rb_grn_object_selector_only_procedure_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool selector_only_procedure_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        selector_only_procedure_p = grn_obj_is_selector_only_proc(context, object);
    }

    return CBOOL2RVAL(selector_only_procedure_p);
}

/*
 * Checks whether the object is scorer procedure or not.
 *
 * @overload scorer_procedure?
 *   @return [Boolean] `true` if the object is scorer procedure,
 *     `false` otherwise.
 *
 * @since 5.0.1
 */
static VALUE
rb_grn_object_scorer_procedure_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool scorer_procedure_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        scorer_procedure_p = grn_obj_is_scorer_proc(context, object);
    }

    return CBOOL2RVAL(scorer_procedure_p);
}

/*
 * Checks whether the object is windows function procedure or not.
 *
 * @overload window_function_procedure?
 *   @return [Boolean] `true` if the object is window function procedure,
 *     `false` otherwise.
 *
 * @since 6.0.4
 */
static VALUE
rb_grn_object_window_function_procedure_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool window_function_procedure_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        window_function_procedure_p =
            grn_obj_is_window_function_proc(context, object);
    }

    return CBOOL2RVAL(window_function_procedure_p);
}

/*
 * Checks whether the object is accessor or not.
 *
 * @overload accessor?
 *   @return [Boolean] `true` if the object is accessor,
 *     `false` otherwise.
 *
 * @since 5.1.1
 */
static VALUE
rb_grn_object_accessor_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool accessor_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        accessor_p = grn_obj_is_accessor(context, object);
    }

    return CBOOL2RVAL(accessor_p);
}

/*
 * Checks whether the object is an accessor for `_key` or not.
 *
 * @example `true` case: `column("_key")` is an accessor and it's an accessor for `_key`
 *   Groonga::Schema.create_table("Users", :key_type => :short_text)
 *   users = Groonga["Users"]
 *   users.column("_key").key_accessor? # => true
 *
 * @example `false` case: `column("_id")` is an accessor but it's not an accessor for `_key`
 *   Groonga::Schema.create_table("Users", :key_type => :short_text)
 *   users = Groonga["Users"]
 *   users.column("_id").key_accessor? # => false
 *
 * @overload key_accessor?
 *   @return [Boolean] `true` if the object is key accessor,
 *     `false` otherwise.
 *
 * @since 5.1.1
 */
static VALUE
rb_grn_object_key_accessor_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool key_accessor_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        key_accessor_p = grn_obj_is_key_accessor(context, object);
    }

    return CBOOL2RVAL(key_accessor_p);
}

/*
 * Checks whether the object is an accessor for `_id` or not.
 *
 * @example `true` case: `column("_id")` is an accessor and it's an accessor for `_id`
 *   Groonga::Schema.create_table("Users", :key_type => :short_text)
 *   users = Groonga["Users"]
 *   users.column("_id").id_accessor? # => true
 *
 * @example `false` case: `column("_key")` is an accessor but it's not an accessor for `_id`
 *   Groonga::Schema.create_table("Users", :key_type => :short_text)
 *   users = Groonga["Users"]
 *   users.column("_key").id_accessor? # => false
 *
 * @overload id_accessor?
 *   @return [Boolean] `true` if the object is an accessor for `_id`,
 *     `false` otherwise.
 *
 * @since 9.0.4
 */
static VALUE
rb_grn_object_id_accessor_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool id_accessor_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        id_accessor_p = grn_obj_is_id_accessor(context, object);
    }

    return CBOOL2RVAL(id_accessor_p);
}

/*
 * Checks whether the object is an accessor for `_value` or not.
 *
 * @example `true` case: `column("_value")` is an accessor and it's an accessor for `_value`
 *   Groonga::Schema.create_table("Users",
 *                                :key_type => :short_text,
 *                                :value_type => "UInt32")
 *   users = Groonga["Users"]
 *   users.column("_value").value_accessor? # => true
 *
 * @example `false` case: `column("_key")` is an accessor but it's not an accessor for `_value`
 *   Groonga::Schema.create_table("Users", :key_type => :short_text)
 *   users = Groonga["Users"]
 *   users.column("_key").value_accessor? # => false
 *
 * @overload value_accessor?
 *   @return [Boolean] `true` if the object is an accessor for `_value`,
 *     `false` otherwise.
 *
 * @since 9.0.4
 */
static VALUE
rb_grn_object_value_accessor_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool value_accessor_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        value_accessor_p = grn_obj_is_value_accessor(context, object);
    }

    return CBOOL2RVAL(value_accessor_p);
}

/*
 * Checks whether the object is an accessor for `_socre` or not.
 *
 * @example `true` case: `column("_score")` is an accessor and it's an accessor for `_score`
 *   Groonga::Schema.create_table("Users", :key_type => :short_text)
 *   users = Groonga["Users"]
 *   users.select.column("_score").socre_accessor? # => true
 *
 * @example `false` case: `column("_key")` is an accessor but it's not an accessor for `_score`
 *   Groonga::Schema.create_table("Users", :key_type => :short_text)
 *   users = Groonga["Users"]
 *   users.column("_key").score_accessor? # => false
 *
 * @overload score_accessor?
 *   @return [Boolean] `true` if the object is an accessor for `_score`,
 *     `false` otherwise.
 *
 * @since 9.0.4
 */
static VALUE
rb_grn_object_score_accessor_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool score_accessor_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        score_accessor_p = grn_obj_is_score_accessor(context, object);
    }

    return CBOOL2RVAL(score_accessor_p);
}

/*
 * Checks whether the object is an accessor for `_nsubrecs` or not.
 *
 * @example `true` case: `column("_nsubrecs")` is an accessor and it's an accessor for `_nsubrecs`
 *   users = Groonga::Hash.create(:name => "Users",
 *                                :key_type => "ShortText")
 *   grouped_users = users.group("_key")
 *   grouped_users.column("_nsubrecs").n_sub_records_accessor? # => true
 *
 * @example `false` case: `column("_key")` is an accessor but it's not an accessor for `_nsubrecs`
 *   Groonga::Schema.create_table("Users", :key_type => :short_text)
 *   users = Groonga["Users"]
 *   users.column("_key").n_sub_records_accessor? # => false
 *
 * @overload n_sub_records_accessor?
 *   @return [Boolean] `true` if the object is an accessor for `_nsubrecs`,
 *     `false` otherwise.
 *
 * @since 9.0.4
 */
VALUE
rb_grn_object_n_sub_records_accessor_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool n_sub_records_accessor_p = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        n_sub_records_accessor_p = grn_obj_is_nsubrecs_accessor(context, object);
    }

    return CBOOL2RVAL(n_sub_records_accessor_p);
}

/*
 * Update the last modified time of the `object`. It's meaningful only
 * for persistent database, table and column.
 *
 * @overload touch(time=nil)
 *   @param [Time, nil] time (nil) The last modified time.
 *     If `time` is `nil`, the current time is used.
 *   @return [void]
 *
 * @since 6.0.5
 */
static VALUE
rb_grn_object_touch (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    VALUE rb_time;
    grn_timeval time_buffer;
    grn_timeval *time = NULL;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);
    if (!context || !object) {
        return Qnil;
    }

    rb_scan_args(argc, argv, "01", &rb_time);
    if (!NIL_P(rb_time)) {
        time_buffer.tv_sec = NUM2LONG(rb_funcall(rb_time, rb_intern("sec"), 0));
        time_buffer.tv_nsec = NUM2INT(rb_funcall(rb_time, rb_intern("nsec"), 0));
        time = &time_buffer;
    }

    grn_obj_touch(context, object, time);
    rb_grn_context_check(context, self);

    return Qnil;
}

/*
 * @overload last_modified
 *   @return [Time] The last modified time of the object.
 *
 * @since 6.0.5
 */
static VALUE
rb_grn_object_get_last_modified (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    uint32_t last_modified;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);
    last_modified = grn_obj_get_last_modified(context, object);
    return rb_funcall(rb_cTime, rb_intern("at"), 1, UINT2NUM(last_modified));
}


/*
 * @overload dirty?
 *   @return [Boolean] `true` if the object isn't flushed after the last change.
 *
 * @since 6.0.5
 */
static VALUE
rb_grn_object_dirty_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool is_dirty = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        is_dirty = grn_obj_is_dirty(context, object);
    }

    return CBOOL2RVAL(is_dirty);
}

/*
 * @overload corrupt?
 *   @return [Boolean] `true` if the object is corrupt, `false` otherwise.
 *
 * @since 7.1.1
 */
static VALUE
rb_grn_object_corrupt_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool is_corrupt = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        is_corrupt = grn_obj_is_corrupt(context, object);
    }

    return CBOOL2RVAL(is_corrupt);
}

/*
 * @overload lexicon?
 *   @return [Boolean] `true` if the object is lexicon, `false` otherwise.
 *
 * @since 9.0.4
 */
static VALUE
rb_grn_object_lexicon_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool is_lexicon = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);

    if (context && object) {
        is_lexicon = grn_obj_is_lexicon(context, object);
    }

    return CBOOL2RVAL(is_lexicon);
}

/*
 * @overload bulk?
 *   @return [Boolean] `true` if the object is bulk, `false` otherwise.
 *
 * @since 9.0.4
 */
static VALUE
rb_grn_object_bulk_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    bool is_bulk = false;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);
    if (context && object) {
        is_bulk = grn_obj_is_bulk(context, object);
    }
    return CBOOL2RVAL(is_bulk);
}

/*
 * @overload disk_usage
 *   @return [Integer] The number of bytes used by this object in disk.
 *
 * @since 7.1.1
 */
static VALUE
rb_grn_object_get_disk_usage (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    size_t disk_usage;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);
    disk_usage = grn_obj_get_disk_usage(context, object);
    return UINT2NUM(disk_usage);
}

void
rb_grn_init_object (VALUE mGrn)
{
    rb_cGrnObject = rb_define_class_under(mGrn, "Object", rb_cObject);
    rb_define_alloc_func(rb_cGrnObject, rb_grn_object_alloc);

    rb_define_attr(rb_cGrnObject, "context", GRN_TRUE, GRN_FALSE);

    rb_define_method(rb_cGrnObject, "inspect", rb_grn_object_inspect, 0);

    rb_define_method(rb_cGrnObject, "id", rb_grn_object_get_id, 0);
    rb_define_method(rb_cGrnObject, "domain", rb_grn_object_get_domain, 0);
    rb_define_method(rb_cGrnObject, "name", rb_grn_object_get_name, 0);
    rb_define_method(rb_cGrnObject, "range", rb_grn_object_get_range, 0);
    rb_define_method(rb_cGrnObject, "path", rb_grn_object_get_path, 0);

    rb_define_method(rb_cGrnObject, "temporary?", rb_grn_object_temporary_p, 0);
    rb_define_method(rb_cGrnObject, "persistent?",
                     rb_grn_object_persistent_p, 0);

    rb_define_method(rb_cGrnObject, "==", rb_grn_object_equal, 1);

    rb_define_method(rb_cGrnObject, "close", rb_grn_object_close, 0);
    rb_define_method(rb_cGrnObject, "closed?", rb_grn_object_closed_p, 0);

    rb_define_method(rb_cGrnObject, "unlink", rb_grn_object_unlink, 0);

    rb_define_method(rb_cGrnObject, "[]", rb_grn_object_array_reference, 1);
    rb_define_method(rb_cGrnObject, "[]=", rb_grn_object_array_set, 2);
    rb_define_method(rb_cGrnObject, "append", rb_grn_object_append_value, 2);
    rb_define_method(rb_cGrnObject, "prepend", rb_grn_object_prepend_value, 2);

    rb_define_method(rb_cGrnObject, "remove", rb_grn_object_remove, -1);

    rb_define_method(rb_cGrnObject, "builtin?", rb_grn_object_builtin_p, 0);
    rb_define_method(rb_cGrnObject, "table?", rb_grn_object_table_p, 0);
    rb_define_method(rb_cGrnObject, "column?", rb_grn_object_column_p, 0);
    rb_define_method(rb_cGrnObject, "reference_column?",
                     rb_grn_object_reference_column_p, 0);
    rb_define_method(rb_cGrnObject, "index_column?",
                     rb_grn_object_index_column_p, 0);
    rb_define_method(rb_cGrnObject, "procedure?", rb_grn_object_procedure_p, 0);
    rb_define_method(rb_cGrnObject, "function_procedure?",
                     rb_grn_object_function_procedure_p, 0);
    rb_define_method(rb_cGrnObject, "selector_procedure?",
                     rb_grn_object_selector_procedure_p, 0);
    rb_define_method(rb_cGrnObject, "selector_only_procedure?",
                     rb_grn_object_selector_only_procedure_p, 0);
    rb_define_method(rb_cGrnObject, "scorer_procedure?",
                     rb_grn_object_scorer_procedure_p, 0);
    rb_define_method(rb_cGrnObject, "window_function_procedure?",
                     rb_grn_object_window_function_procedure_p, 0);
    rb_define_method(rb_cGrnObject, "accessor?",
                     rb_grn_object_accessor_p, 0);
    rb_define_method(rb_cGrnObject, "key_accessor?",
                     rb_grn_object_key_accessor_p, 0);
    rb_define_method(rb_cGrnObject, "id_accessor?",
                     rb_grn_object_id_accessor_p, 0);
    rb_define_method(rb_cGrnObject, "value_accessor?",
                     rb_grn_object_value_accessor_p, 0);
    rb_define_method(rb_cGrnObject, "score_accessor?",
                     rb_grn_object_score_accessor_p, 0);
    rb_define_method(rb_cGrnObject, "n_sub_records_accessor?",
                     rb_grn_object_n_sub_records_accessor_p, 0);

    rb_define_method(rb_cGrnObject, "touch", rb_grn_object_touch, -1);
    rb_define_method(rb_cGrnObject, "last_modified",
                     rb_grn_object_get_last_modified, 0);
    rb_define_method(rb_cGrnObject, "dirty?",
                     rb_grn_object_dirty_p, 0);
    rb_define_method(rb_cGrnObject, "corrupt?",
                     rb_grn_object_corrupt_p, 0);
    rb_define_method(rb_cGrnObject, "lexicon?",
                     rb_grn_object_lexicon_p, 0);
    rb_define_method(rb_cGrnObject, "bulk?",
                     rb_grn_object_bulk_p, 0);

    rb_define_method(rb_cGrnObject, "disk_usage",
                     rb_grn_object_get_disk_usage, 0);
}
