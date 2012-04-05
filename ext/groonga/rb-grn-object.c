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

/*
 * Document-class: Groonga::Object
 *
 * rroongaが提供するクラスのベースとなるクラス。
 * Groonga::ContextとGroonga::Logger以外はGroonga::Objectを継
 * 承している。
 */

#include "rb-grn.h"

#define SELF(object) ((RbGrnObject *)DATA_PTR(object))

VALUE rb_cGrnObject;

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
	    if (!grn_object)
		rb_raise(rb_eArgError,
			 "unregistered groonga object: name: <%s>",
			 rb_grn_inspect(object));
	    return grn_object;
	} else if (RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cInteger))) {
	    grn_object = grn_ctx_at(*context, NUM2UINT(object));
	    if (!grn_object)
		rb_raise(rb_eArgError,
			 "unregistered groonga object: ID: <%s>",
			 rb_grn_inspect(object));
	    return grn_object;
	}
    }

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnObject))) {
	rb_raise(rb_eTypeError, "not a groonga object: <%s>",
		 rb_grn_inspect(object));
    }

    Data_Get_Struct(object, RbGrnObject, rb_grn_object);
    if (!rb_grn_object)
	rb_raise(rb_eGrnError, "groonga object is NULL");

    if (context && !*context)
	*context = rb_grn_object->context;

    return rb_grn_object->object;
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
	  rb_grn_inspect_type(grn_object->header.type),
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
      case GRN_CURSOR_TABLE_VIEW:
	break;
      case GRN_TABLE_HASH_KEY:
      case GRN_TABLE_PAT_KEY:
      case GRN_TABLE_DAT_KEY:
	rb_grn_table_key_support_finalizer(context, grn_object,
					   RB_GRN_TABLE_KEY_SUPPORT(rb_grn_object));
	break;
      case GRN_TABLE_NO_KEY:
      case GRN_TABLE_VIEW:
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
		 "unsupported groonga object type for finalizer: %s(%#x)",
		 rb_grn_inspect_type(grn_object->header.type),
		 grn_object->header.type);
	break;
    }

    rb_grn_object->rb_grn_context = NULL;
    rb_grn_object->context = NULL;
    rb_grn_object->object = NULL;
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
	      rb_grn_inspect_type(grn_object->header.type),
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
      case GRN_TABLE_VIEW:
	klass = rb_cGrnView;
	break;
      case GRN_TYPE:
	klass = rb_cGrnType;
	break;
      case GRN_ACCESSOR:
	klass = rb_cGrnAccessor;
	break;
      case GRN_ACCESSOR_VIEW:
	klass = rb_cGrnViewAccessor;
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
      case GRN_CURSOR_TABLE_VIEW:
	klass = rb_cGrnViewCursor;
	break;
      default:
	rb_raise(rb_eTypeError,
		 "unsupported groonga object type for class detection: 0x%x",
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

    return rb_object;
}

VALUE
rb_grn_object_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_grn_object_free, NULL);
}

static void
rb_grn_object_bind_common (VALUE klass, VALUE self, VALUE rb_context,
			   RbGrnObject *rb_grn_object,
			   grn_ctx *context, grn_obj *object)
{
    grn_user_data *user_data;
    RbGrnContext *rb_grn_context;

    debug("bind: %p:%p:%p %s(%#x)\n",
	  context, object, rb_grn_object,
	  rb_grn_inspect_type(object->header.type),
	  object->header.type);

    Data_Get_Struct(rb_context, RbGrnContext, rb_grn_context);
    rb_grn_object->rb_grn_context = rb_grn_context;
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
	      rb_grn_inspect_type(object->header.type),
	      object->header.type);
	user_data->ptr = rb_grn_object;
	grn_obj_set_finalizer(context, object, rb_grn_object_finalizer);
	rb_grn_object->have_finalizer = GRN_TRUE;
    } else if (object->header.type == GRN_ACCESSOR) {
	debug("set-finalizer(implicit): %p:%p:%p %s(%#x)\n",
	      context, object, rb_grn_object,
	      rb_grn_inspect_type(object->header.type),
	      object->header.type);
	 /* TODO: We want to call finalizer for GRN_ACCESSOR. */
	rb_grn_object->have_finalizer = GRN_FALSE;
    }

    switch (object->header.type) {
      case GRN_PROC:
      case GRN_TYPE:
      case GRN_ACCESSOR: /* TODO: We want to close GRN_ACCESSOR. */
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

    DATA_PTR(self) = rb_grn_object;
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
	klass == rb_cGrnViewCursor ||
	klass == rb_cGrnIndexCursor ||
	klass == rb_cGrnProcedure ||
	klass == rb_cGrnVariable) {
	rb_grn_object = ALLOC(RbGrnObject);
	rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
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
    } else if (klass == rb_cGrnViewAccessor) {
	rb_grn_object = ALLOC(RbGrnNamedObject);
	rb_grn_object_bind_common(klass, self, rb_context, rb_grn_object,
				  context, object);
	rb_grn_named_object_bind(RB_GRN_NAMED_OBJECT(rb_grn_object),
				 context, object);
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
		 "unsupported groonga object type for assignment: %s(%#x)",
		 rb_grn_inspect_type(object->header.type),
		 object->header.type);
    }

    rb_iv_set(self, "@context", rb_context);

    debug("assign: %p:%p:%p %s(%#x)\n",
	  context, object, rb_grn_object,
	  rb_grn_inspect_type(object->header.type), object->header.type);
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
	RbGrnObject *rb_grn_object;
	rb_grn_named_object->name = ALLOC_N(char, name_size + 1);
	memcpy(rb_grn_named_object->name, name, name_size);
	rb_grn_named_object->name[name_size] = '\0';
	rb_grn_object = RB_GRN_OBJECT(rb_grn_named_object);
	debug("set-name: %p:%p:%p %s(%#x): <%.*s>\n",
	      rb_grn_object->context,
	      rb_grn_object->object,
	      rb_grn_named_object,
	      rb_grn_inspect_type(rb_grn_object->object->header.type),
	      rb_grn_object->object->header.type,
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

    if (!rb_grn_object->object) {
	rb_raise(rb_eGrnClosed,
		 "can't access already closed groonga object: %s",
		 rb_grn_inspect(CLASS_OF(rb_grn_object->self)));
    }

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

/*
 * Document-method: close
 *
 * call-seq:
 *   object.close
 *
 * _object_ が使用しているリソースを開放する。これ以降 _object_ を
 * 使うことはできない。
 */
VALUE
rb_grn_object_close (VALUE self)
{
    RbGrnObject *rb_grn_object;
    grn_obj *object;
    grn_ctx *context;

    rb_grn_object = SELF(self);
    rb_grn_object_deconstruct(rb_grn_object, &object, &context,
			      NULL, NULL, NULL, NULL);
    if (object && context) {
	rb_grn_object_run_finalizer(context, object, rb_grn_object);
	grn_obj_close(context, object);
    }

    return Qnil;
}

/*
 * Document-method: unlink
 *
 * call-seq:
 *   object.unlink
 *
 * _object_ のリファレンスカウンタを1減少する。
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
	}
	grn_obj_unlink(context, object);
    }

    return Qnil;
}

/*
 * Document-method: closed?
 *
 * call-seq:
 *   object.closed? -> true/false
 *
 * _object_ が開放済みの場合は+true+を返し、そうでない場合は
 * +false+ を返す。
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

static VALUE
rb_grn_object_inspect_content_name (VALUE inspected,
				    grn_ctx *context, grn_obj *object)
{
    int name_size;

    name_size = grn_obj_name(context, object, NULL, 0);
    if (name_size == 0) {
	rb_str_cat2(inspected, "(anonymous)");
    } else {
	grn_obj name;

	GRN_OBJ_INIT(&name, GRN_BULK, 0, GRN_ID_NIL);
	grn_bulk_space(context, &name, name_size);
	grn_obj_name(context, object, GRN_BULK_HEAD(&name), name_size);
	GRN_TEXT_PUTC(context, &name, '\0');
	rb_str_cat2(inspected, "<");
	rb_str_cat2(inspected, GRN_BULK_HEAD(&name));
	rb_str_cat2(inspected, ">");
	grn_obj_unlink(context, &name);
    }

    return inspected;
}

static VALUE
rb_grn_object_inspect_content_name_with_label (VALUE inspected,
					       grn_ctx *context, grn_obj *object)
{

    rb_str_cat2(inspected, "name: ");
    rb_grn_object_inspect_content_name(inspected, context, object);
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
		rb_grn_object_inspect_content_name(inspected,
						   context, domain_object);
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
		    rb_grn_object_inspect_content_name(inspected,
						       context, range_object);
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
    VALUE inspected_flags;

    rb_str_cat2(inspected, "flags: ");

    flags = object->header.flags;

    inspected_flags = rb_ary_new();

    if (0) {
	if (flags & GRN_OBJ_TABLE_HASH_KEY)
	    rb_ary_push(inspected_flags, rb_str_new2("TABLE_HASH_KEY"));
	if (flags & GRN_OBJ_TABLE_PAT_KEY)
	    rb_ary_push(inspected_flags, rb_str_new2("TABLE_PAT_KEY"));
	if (flags & GRN_OBJ_TABLE_DAT_KEY)
	    rb_ary_push(inspected_flags, rb_str_new2("TABLE_DAT_KEY"));
	if (flags & GRN_OBJ_TABLE_NO_KEY)
	    rb_ary_push(inspected_flags, rb_str_new2("TABLE_NO_KEY"));
	if (flags & GRN_OBJ_TABLE_VIEW)
	    rb_ary_push(inspected_flags, rb_str_new2("TABLE_VIEW"));
    }

    switch (object->header.type) {
      case GRN_COLUMN_FIX_SIZE:
      case GRN_COLUMN_VAR_SIZE:
      case GRN_TYPE:
	if (flags & GRN_OBJ_KEY_UINT)
	    rb_ary_push(inspected_flags, rb_str_new2("KEY_UINT"));
	if (flags & GRN_OBJ_KEY_INT)
	    rb_ary_push(inspected_flags, rb_str_new2("KEY_INT"));
	if (flags & GRN_OBJ_KEY_FLOAT)
	    rb_ary_push(inspected_flags, rb_str_new2("KEY_FLOAT"));
	break;
      default:
	break;
    }

    switch (object->header.type) {
      case GRN_TABLE_HASH_KEY:
      case GRN_TABLE_PAT_KEY:
      case GRN_TABLE_DAT_KEY:
	if (flags & GRN_OBJ_KEY_WITH_SIS)
	    rb_ary_push(inspected_flags, rb_str_new2("KEY_WITH_SIS"));
	if (flags & GRN_OBJ_KEY_NORMALIZE)
	    rb_ary_push(inspected_flags, rb_str_new2("KEY_NORMALIZE"));
	break;
      default:
	break;
    }

    if (0) {
	if (flags & GRN_OBJ_COLUMN_SCALAR)
	    rb_ary_push(inspected_flags, rb_str_new2("COLUMN_SCALAR"));
	if (flags & GRN_OBJ_COLUMN_VECTOR)
	    rb_ary_push(inspected_flags, rb_str_new2("COLUMN_VECTOR"));
	if (flags & GRN_OBJ_COLUMN_INDEX)
	    rb_ary_push(inspected_flags, rb_str_new2("COLUMN_INDEX"));
    }

    switch (object->header.type) {
      case GRN_COLUMN_FIX_SIZE:
      case GRN_COLUMN_VAR_SIZE:
	if (flags & GRN_OBJ_COMPRESS_ZLIB)
	    rb_ary_push(inspected_flags, rb_str_new2("COMPRESS_ZLIB"));
	if (flags & GRN_OBJ_COMPRESS_LZO)
	    rb_ary_push(inspected_flags, rb_str_new2("COMPRESS_LZO"));
	break;
      default:
	break;
    }

    if (flags & GRN_OBJ_WITH_SECTION)
	rb_ary_push(inspected_flags, rb_str_new2("WITH_SECTION"));
    if (flags & GRN_OBJ_WITH_WEIGHT)
	rb_ary_push(inspected_flags, rb_str_new2("WITH_WEIGHT"));
    if (flags & GRN_OBJ_WITH_POSITION)
	rb_ary_push(inspected_flags, rb_str_new2("WITH_POSITION"));
    if (flags & GRN_OBJ_RING_BUFFER)
	rb_ary_push(inspected_flags, rb_str_new2("RING_BUFFER"));

    if (flags & GRN_OBJ_WITH_SUBREC) {
	rb_ary_push(inspected_flags, rb_str_new2("WITH_SUBREC"));

	if (flags & GRN_OBJ_UNIT_DOCUMENT_SECTION)
	    rb_ary_push(inspected_flags, rb_str_new2("UNIT_DOCUMENT_SECTION"));
	if (flags & GRN_OBJ_UNIT_DOCUMENT_POSITION)
	    rb_ary_push(inspected_flags, rb_str_new2("UNIT_DOCUMENT_POSITION"));

	if (flags & GRN_OBJ_UNIT_SECTION_POSITION)
	    rb_ary_push(inspected_flags, rb_str_new2("UNIT_SECTION_POSITION"));

	if (flags & GRN_OBJ_UNIT_USERDEF_DOCUMENT)
	    rb_ary_push(inspected_flags, rb_str_new2("UNIT_USERDEF_DOCUMENT"));
	if (flags & GRN_OBJ_UNIT_USERDEF_SECTION)
	    rb_ary_push(inspected_flags, rb_str_new2("UNIT_USERDEF_SECTION"));
	if (flags & GRN_OBJ_UNIT_USERDEF_POSITION)
	    rb_ary_push(inspected_flags, rb_str_new2("UNIT_USERDEF_POSITION"));
    }

    rb_str_cat2(inspected, "<");
    rb_str_concat(inspected, rb_ary_join(inspected_flags, rb_str_new2("|")));
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
 * Document-method: inspect
 *
 * call-seq:
 *   object.inspect -> 詳細情報
 *
 * _object_ の詳細を示した文字列を返す。デバッグ用。
 */
static VALUE
rb_grn_object_inspect (VALUE self)
{
    VALUE inspected;

    inspected = rb_str_new2("");
    rb_grn_object_inspect_header(self, inspected);
    rb_grn_object_inspect_content(self, inspected);
    rb_grn_object_inspect_footer(self, inspected);

    return inspected;
}

/*
 * Document-method: id
 *
 * call-seq:
 *   object.id -> ID/nil
 *
 * _object_ のIDを返す。 _object_ が#closed?なときやIDがない場合
 * は +nil+ を返す。
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
 * Document-method: path
 *
 * call-seq:
 *   object.path -> ファイルパス/nil
 *
 * _object_ に対応するファイルパスを返す。一時 _object_
 * なら +nil+ を返す。
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
	return rb_str_new2(path);
}

/*
 * Document-method: temporary?
 *
 * call-seq:
 *   object.temporary? -> true/false
 *
 * _object_ が一時オブジェクトなら +true+ 、永続オブジェクトな
 * ら +false+ を返す。
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
 * Document-method: persistent?
 *
 * call-seq:
 *   object.persistent? -> true/false
 *
 * _object_ が永続オブジェクトなら +true+ 、一時オブジェクトな
 * ら +false+ を返す。
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
 * Document-method: domain
 *
 * call-seq:
 *   object.domain -> Groonga::Object/nil
 *
 * _object_ の属しているGroonga::Objectを返す。例えば、
 * Groonga::ColumnはGroonga::Tableを返す。属している
 * Groonga::Objectがない場合は +nil+ を返す。
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
 * Document-method: name
 *
 * call-seq:
 *   object.name -> 名前/nil
 *
 * _object_ の名前を返す。無名オブジェクトの場合は +nil+ を返す。
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
 * Document-method: range
 *
 * call-seq:
 *   object.range -> Groonga::Object/nil
 *
 * _object_ の値がとりうる範囲を示したGroonga::Objectを返す。
 * 例えば、Groonga::Columnの場合は
 * Groonga::Table#define_columnで指定されたGroonga::Typeや
 * Groonga::Tableを返す。
 * 範囲が指定されていないオブジェクトの場合は +nil+ を返す。
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
 * Document-method: ==
 *
 * call-seq:
 *   object == other -> true/false
 *
 * _object_ と _other_ が同じgroongaのオブジェクトなら +true+ を返
 * し、そうでなければ +false+ を返す。
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
 * Document-method: []
 *
 * call-seq:
 *   object[id] -> 値
 *
 * _object_ の _id_ に対応する値を返す。
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
      case GRN_ACCESSOR_VIEW: /* FIXME */
	GRN_OBJ_INIT(&value, GRN_BULK, 0, range_id);
	break;
      case GRN_COLUMN_VAR_SIZE:
      case GRN_COLUMN_FIX_SIZE:
	switch (object->header.flags & GRN_OBJ_COLUMN_TYPE_MASK) {
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

static grn_bool
rb_uvector_value_p (RbGrnObject *rb_grn_object, VALUE rb_value)
{
    VALUE first_element;

    switch (rb_grn_object->range->header.type) {
      case GRN_TYPE:
	/* TODO: support not sizeof(grn_id) uvector. */
	/*
	if (!(rb_grn_object->range->header.flags | GRN_OBJ_KEY_VAR_SIZE)) {
	    return GRN_TRUE;
	}
	*/
	break;
      case GRN_TABLE_HASH_KEY:
      case GRN_TABLE_PAT_KEY:
      case GRN_TABLE_DAT_KEY:
      case GRN_TABLE_NO_KEY:
      case GRN_TABLE_VIEW:
	first_element = rb_ary_entry(rb_value, 0);
	if (rb_respond_to(first_element, rb_intern("record_raw_id"))) {
	    return GRN_TRUE;
	}
	break;
      default:
	break;
    }

    return GRN_FALSE;
}

VALUE
rb_grn_object_set_raw (RbGrnObject *rb_grn_object, grn_id id,
		       VALUE rb_value, int flags, VALUE related_object)
{
    grn_ctx *context;
    grn_obj value;
    grn_rc rc;
    VALUE exception, rb_values;

    context = rb_grn_object->context;
    rb_values = rb_check_array_type(rb_value);
    if (NIL_P(rb_values)) {
	if (NIL_P(rb_value)) {
	    GRN_OBJ_INIT(&value, GRN_BULK, 0, GRN_ID_NIL);
	} else {
	    GRN_OBJ_INIT(&value, GRN_BULK, 0, GRN_ID_NIL);
	    RVAL2GRNBULK(rb_value, context, &value);
	}
    } else {
	if (rb_uvector_value_p(rb_grn_object, rb_values)) {
	    GRN_OBJ_INIT(&value, GRN_UVECTOR, 0,
			 rb_grn_object->object->header.domain);
	    RVAL2GRNUVECTOR(rb_values, context, &value, related_object);
	} else {
	    GRN_OBJ_INIT(&value, GRN_VECTOR, 0, GRN_ID_NIL);
	    RVAL2GRNVECTOR(rb_values, context, &value);
	}
    }
    rc = grn_obj_set_value(context, rb_grn_object->object, id,
			   &value, flags);
    exception = rb_grn_context_to_exception(context, related_object);
    grn_obj_unlink(context, &value);
    if (!NIL_P(exception))
	rb_exc_raise(exception);
    rb_grn_rc_check(rc, related_object);

    return Qnil;
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
 * Document-method: []=
 *
 * call-seq:
 *   object[id] = value
 *
 * _object_ の _id_ に対応する値を設定する。既存の値は上書きさ
 * れる。
 */
static VALUE
rb_grn_object_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    return rb_grn_object_set(self, rb_id, rb_value, GRN_OBJ_SET);
}

/*
 * Document-method: append
 *
 * call-seq:
 *   object.append(id, value)
 *
 * _object_ の _id_ に対応する値の最後に _value_ を追加する。
 */
static VALUE
rb_grn_object_append_value (VALUE self, VALUE rb_id, VALUE rb_value)
{
    return rb_grn_object_set(self, rb_id, rb_value, GRN_OBJ_APPEND);
}

/*
 * Document-method: prepend
 *
 * call-seq:
 *   object.prepend(id, value)
 *
 * _object_ の _id_ に対応する値の最初に _value_ を追加する。
 */
static VALUE
rb_grn_object_prepend_value (VALUE self, VALUE rb_id, VALUE rb_value)
{
    return rb_grn_object_set(self, rb_id, rb_value, GRN_OBJ_PREPEND);
}

/*
 * Document-method: remove
 *
 * call-seq:
 *   object.remove
 *
 * _object_ をメモリから解放し、それが永続オブジェクトであっ
 * た場合は、該当するファイル一式を削除する。
 */
static VALUE
rb_grn_object_remove (VALUE self)
{
    RbGrnObject *rb_grn_object;
    grn_ctx *context;
    grn_rc rc;

    rb_grn_object = SELF(self);
    if (!rb_grn_object->object)
	return Qnil;

    context = rb_grn_object->context;
    rc = grn_obj_remove(context, rb_grn_object->object);
    rb_grn_rc_check(rc, self);

    rb_iv_set(self, "@context", Qnil);

    return Qnil;
}

static VALUE
rb_grn_object_builtin_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *object;
    grn_bool builtin = GRN_FALSE;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
			      NULL, NULL, NULL, NULL);

    if (context && object) {
	builtin = grn_obj_is_builtin(context, object);
    }

    return CBOOL2RVAL(builtin);
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

    rb_define_method(rb_cGrnObject, "remove", rb_grn_object_remove, 0);

    rb_define_method(rb_cGrnObject, "builtin?", rb_grn_object_builtin_p, 0);

}
