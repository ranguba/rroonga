/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2020  Sutou Kouhei <kou@clear-code.com>

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

#define SELF(object) (RVAL2GRNTYPE(object))

VALUE rb_cGrnType;

/*
 * Document-class: Groonga::Type
 *
 * groongaのテーブルの主キーや、カラムの値の型のためのオブジェ
 * クト。型として使用可能なものはgroongaで予め定義済みの型、ユ
 * ーザが定義する型、またはユーザが定義したテーブル。
 */


grn_obj *
rb_grn_type_from_ruby_object (VALUE object)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnType))) {
        rb_raise(rb_eTypeError, "not a groonga type");
    }

    return RVAL2GRNOBJECT(object, NULL);
}

VALUE
rb_grn_type_to_ruby_object (grn_ctx *context, grn_obj *type,
                            grn_bool owner)
{
    return GRNOBJECT2RVAL(rb_cGrnType, context, type, owner);
}

/*
 * 名前が _name_ の型を作成する。
 *
 * @overload new(name, options={})
 *   @param name [String] 作成する型の名前
 *   @param options [::Hash] The name and value
 *     pairs. Omitted names are initialized as the default value
 *   @option options [Symbol] :type (:variable)
 *     :integer（符号付き整数）、:int（:integerの省略
 *     形）、:unsigned_integer（符号なし整
 *     数）、:uint（:unsigned_integerの省略形）、:float（浮動小数点
 *     数）、:variable（可変長文字列）のいずれかを指定する。省略した場
 *     合は:variableを指定したものと扱う。
 *     :variableを指定した場合は必ず +:size+ を指定しなければいけない。
 *   @option options [Context] :context
 *     型の作成時に利用するGroonga::Contextを指定する。省略すると
 *     Groonga::Context.defaultを用いる。
 *   @option options [Integer] :size
 *     +:option+ が:variableの場合は最大長、それ以外の場合は長さを
 *     指定する(単位:byte)。
 */
static VALUE
rb_grn_type_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *type;
    const char *name = NULL;
    unsigned name_size, size = 0;
    grn_obj_flags flags = 0;
    VALUE rb_name, options, rb_context, rb_type, rb_size;

    rb_scan_args(argc, argv, "11", &rb_name, &options);

    rb_grn_scan_options(options,
                        "context", &rb_context,
                        "type", &rb_type,
                        "size", &rb_size,
                        NULL);

    name = StringValuePtr(rb_name);
    name_size = RSTRING_LEN(rb_name);

    context = rb_grn_context_ensure(&rb_context);

    if (NIL_P(rb_type) ||
        rb_grn_equal_option(rb_type, "variable")) {
        flags = GRN_OBJ_KEY_VAR_SIZE;
    } else if (rb_grn_equal_option(rb_type, "integer") ||
               rb_grn_equal_option(rb_type, "int")) {
        flags = GRN_OBJ_KEY_INT;
        size = sizeof(int);
    } else if (rb_grn_equal_option(rb_type, "unsigned_integer") ||
               rb_grn_equal_option(rb_type, "uint")) {
        flags = GRN_OBJ_KEY_UINT;
        size = sizeof(unsigned int);
    } else if (rb_grn_equal_option(rb_type, "float")) {
        flags = GRN_OBJ_KEY_FLOAT;
        size = sizeof(double);
    } else {
        rb_raise(rb_eArgError,
                 ":type should be one of "
                 "[:integer, :int, :unsigned_integer, :uint, "
                 ":float, :variable]: %s",
                 rb_grn_inspect(options));
    }

    if (NIL_P(rb_size)) {
        if (size == 0)
            rb_raise(rb_eArgError, "size is missing: %s",
                     rb_grn_inspect(options));
    } else {
        size = NUM2UINT(rb_size);
    }

    type = grn_type_create(context, name, name_size, flags, size);
    rb_grn_object_assign(Qnil, self, rb_context, context, type);
    rb_grn_context_check(context, rb_ary_new_from_values(argc, argv));

    return Qnil;
}

static VALUE
rb_grn_type_size (VALUE self)
{
    grn_ctx *context;
    grn_obj *type;

    type = RVAL2GRNOBJECT(self, &context);

    return UINT2NUM(grn_obj_get_range(context, type));
}

static VALUE
rb_grn_type_flags (VALUE self)
{
    grn_ctx *context;
    grn_obj *type;

    type = RVAL2GRNOBJECT(self, &context);

    return UINT2NUM(type->header.flags);
}

static VALUE
rb_grn_type_fixed_size_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *type;

    type = RVAL2GRNOBJECT(self, &context);

    return CBOOL2RVAL(!(type->header.flags & GRN_OBJ_KEY_VAR_SIZE));
}

static VALUE
rb_grn_type_variable_size_p (VALUE self)
{
    grn_ctx *context;
    grn_obj *type;

    type = RVAL2GRNOBJECT(self, &context);

    return CBOOL2RVAL(type->header.flags & GRN_OBJ_KEY_VAR_SIZE);
}

static VALUE
rb_grn_type_builtin_p (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *type;
    grn_id id;

    type = RVAL2GRNOBJECT(self, &context);
    id =  grn_obj_id(context, type);
    return CBOOL2RVAL(grn_type_id_is_builtin(context, id));
}

static VALUE
rb_grn_type_unsigned_integer_p (VALUE self)
{
    grn_obj *type;
    grn_obj_flags key_type;

    type = RVAL2GRNOBJECT(self, NULL);
    key_type = type->header.flags & GRN_OBJ_KEY_MASK;
    return CBOOL2RVAL(key_type == GRN_OBJ_KEY_UINT);
}

static VALUE
rb_grn_type_integer_p (VALUE self)
{
    grn_obj *type;
    grn_obj_flags key_type;

    type = RVAL2GRNOBJECT(self, NULL);
    key_type = type->header.flags & GRN_OBJ_KEY_MASK;
    return CBOOL2RVAL(key_type == GRN_OBJ_KEY_INT);
}

static VALUE
rb_grn_type_float_p (VALUE self)
{
    grn_obj *type;
    grn_obj_flags key_type;

    type = RVAL2GRNOBJECT(self, NULL);
    key_type = type->header.flags & GRN_OBJ_KEY_MASK;
    return CBOOL2RVAL(key_type == GRN_OBJ_KEY_FLOAT);
}

static VALUE
rb_grn_type_geo_point_p (VALUE self)
{
    grn_obj *type;
    grn_obj_flags key_type;

    type = RVAL2GRNOBJECT(self, NULL);
    key_type = type->header.flags & GRN_OBJ_KEY_MASK;
    return CBOOL2RVAL(key_type == GRN_OBJ_KEY_GEO_POINT);
}

static VALUE
rb_grn_type_text_family_p (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *type;
    grn_id id;

    type = RVAL2GRNOBJECT(self, &context);
    id =  grn_obj_id(context, type);
    return CBOOL2RVAL(grn_type_id_is_text_family(context, id));
}

static VALUE
rb_grn_type_number_family_p (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *type;
    grn_id id;

    type = RVAL2GRNOBJECT(self, &context);
    id =  grn_obj_id(context, type);
    return CBOOL2RVAL(grn_type_id_is_number_family(context, id));
}

void
rb_grn_init_type (VALUE mGrn)
{
    rb_cGrnType = rb_define_class_under(mGrn, "Type", rb_cGrnObject);

    rb_define_method(rb_cGrnType, "initialize", rb_grn_type_initialize, -1);
    rb_define_method(rb_cGrnType, "size", rb_grn_type_size, 0);
    rb_define_method(rb_cGrnType, "flags", rb_grn_type_flags, 0);
    rb_define_method(rb_cGrnType, "fixed_size?", rb_grn_type_fixed_size_p, 0);
    rb_define_method(rb_cGrnType, "variable_size?",
                     rb_grn_type_variable_size_p, 0);

    rb_define_method(rb_cGrnType, "builtin?",
                     rb_grn_type_builtin_p, 0);

    rb_define_method(rb_cGrnType, "unsigned_integer?",
                     rb_grn_type_unsigned_integer_p, 0);
    rb_define_alias(rb_cGrnType, "uint?", "unsigned_integer?");

    rb_define_method(rb_cGrnType, "integer?", rb_grn_type_integer_p, 0);
    rb_define_alias(rb_cGrnType, "int?", "integer?");

    rb_define_method(rb_cGrnType, "float?", rb_grn_type_float_p, 0);
    rb_define_method(rb_cGrnType, "geo_point?", rb_grn_type_geo_point_p, 0);
    rb_define_method(rb_cGrnType, "text_family?", rb_grn_type_text_family_p, 0);
    rb_define_method(rb_cGrnType, "number_family?",
                     rb_grn_type_number_family_p, 0);

    /* 任意のテーブルに属する全てのレコード(Object型はv1.2で
       サポートされます)。 */
    rb_define_const(rb_cGrnType, "OBJECT", INT2NUM(GRN_DB_OBJECT));
    /* bool型。trueとfalse。 */
    rb_define_const(rb_cGrnType, "BOOLEAN", INT2NUM(GRN_DB_BOOL));
    /* bool型。trueとfalse。 */
    rb_define_const(rb_cGrnType, "BOOL", INT2NUM(GRN_DB_BOOL));
    /* 8bit符号付き整数。 */
    rb_define_const(rb_cGrnType, "INT8", INT2NUM(GRN_DB_INT8));
    /* 8bit符号なし整数。 */
    rb_define_const(rb_cGrnType, "UINT8", INT2NUM(GRN_DB_UINT8));
    /* 16bit符号付き整数。 */
    rb_define_const(rb_cGrnType, "INT16", INT2NUM(GRN_DB_INT16));
    /* 16bit符号なし整数。 */
    rb_define_const(rb_cGrnType, "UINT16", INT2NUM(GRN_DB_UINT16));
    /* 32bit符号付き整数。 */
    rb_define_const(rb_cGrnType, "INT32", INT2NUM(GRN_DB_INT32));
    /* 32bit符号なし整数。 */
    rb_define_const(rb_cGrnType, "UINT32", INT2NUM(GRN_DB_UINT32));
    /* 64bit符号付き整数。 */
    rb_define_const(rb_cGrnType, "INT64", INT2NUM(GRN_DB_INT64));
    /* 64bit符号なし整数。 */
    rb_define_const(rb_cGrnType, "UINT64", INT2NUM(GRN_DB_UINT64));
#if RB_GRN_HAVE_FLOAT32
    /* 32bit floating pointer number in IEEE754 format. */
    rb_define_const(rb_cGrnType, "FLOAT32", INT2NUM(GRN_DB_FLOAT32));
#endif
    /* ieee754形式の64bit浮動小数点数。 */
    rb_define_const(rb_cGrnType, "FLOAT", INT2NUM(GRN_DB_FLOAT));
    /* 1970年1月1日0時0分0秒からの経過マイクロ秒数を64bit符
       号付き整数で表現した値。 */
    rb_define_const(rb_cGrnType, "TIME", INT2NUM(GRN_DB_TIME));
    /* 4Kbyte以下の文字列。 */
    rb_define_const(rb_cGrnType, "SHORT_TEXT", INT2NUM(GRN_DB_SHORT_TEXT));
    /* 64Kbyte以下の文字列。 */
    rb_define_const(rb_cGrnType, "TEXT", INT2NUM(GRN_DB_TEXT));
    /* 2Gbyte以下の文字列。 */
    rb_define_const(rb_cGrnType, "LONG_TEXT", INT2NUM(GRN_DB_LONG_TEXT));
    rb_define_const(rb_cGrnType, "DELIMIT", INT2NUM(GRN_DB_DELIMIT));
    rb_define_const(rb_cGrnType, "UNIGRAM", INT2NUM(GRN_DB_UNIGRAM));
    rb_define_const(rb_cGrnType, "BIGRAM", INT2NUM(GRN_DB_BIGRAM));
    rb_define_const(rb_cGrnType, "TRIGRAM", INT2NUM(GRN_DB_TRIGRAM));
    rb_define_const(rb_cGrnType, "MECAB", INT2NUM(GRN_DB_MECAB));
}
