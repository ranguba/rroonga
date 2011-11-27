/* -*- coding: utf-8; c-file-style: "ruby" -*- */
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

#define SELF(object) (RB_GRN_OBJECT(DATA_PTR(object)))

VALUE rb_mGrnEncodingSupport;

/*
 * Document-module: Groonga::EncodingSupport
 *
 * オブジェクトにエンコーディング関連の機能を提供するモジュー
 * ル。
 */

/*
 * call-seq:
 *   object.encoding -> エンコーディング
 *
 * オブジェクトのエンコーディングを返す。
 */
static VALUE
rb_grn_encoding_support_get_encoding (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *object = NULL;
    grn_obj *encoding_value;
    grn_encoding encoding;

    rb_grn_object_deconstruct(SELF(self), &object, &context,
                              NULL, NULL, NULL, NULL);
    encoding_value = grn_obj_get_info(context, object, GRN_INFO_ENCODING, NULL);
    rb_grn_context_check(context, self);

    memcpy(&encoding, GRN_BULK_HEAD(encoding_value), sizeof(encoding));
    grn_obj_unlink(context, encoding_value);

    return GRNENCODING2RVAL(encoding);
}

void
rb_grn_init_encoding_support (VALUE mGrn)
{
    rb_mGrnEncodingSupport = rb_define_module_under(mGrn, "EncodingSupport");

    rb_define_method(rb_mGrnEncodingSupport, "encoding",
		     rb_grn_encoding_support_get_encoding, 0);
}
