/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-groonga-private.h"

static VALUE mGrnEncoding;
static VALUE RB_GRN_ENCODING_DEFAULT;
static VALUE RB_GRN_ENCODING_NONE;
static VALUE RB_GRN_ENCODING_EUC_JP;
static VALUE RB_GRN_ENCODING_UTF8;
static VALUE RB_GRN_ENCODING_SJIS;
static VALUE RB_GRN_ENCODING_LATIN1;
static VALUE RB_GRN_ENCODING_KOI8R;

grn_encoding
rb_grn_encoding_from_ruby_object (VALUE object)
{
    if (NIL_P(object))
        return GRN_ENC_DEFAULT;

    if (rb_funcall(object, rb_intern("=="), 1, RB_GRN_ENCODING_DEFAULT)) {
        return GRN_ENC_DEFAULT;
    } else if (rb_funcall(object, rb_intern("=="), 1, RB_GRN_ENCODING_NONE)) {
        return GRN_ENC_NONE;
    } else if (rb_funcall(object, rb_intern("=="), 1, RB_GRN_ENCODING_EUC_JP)) {
        return GRN_ENC_EUC_JP;
    } else if (rb_funcall(object, rb_intern("=="), 1, RB_GRN_ENCODING_UTF8)) {
        return GRN_ENC_UTF8;
    } else if (rb_funcall(object, rb_intern("=="), 1, RB_GRN_ENCODING_SJIS)) {
        return GRN_ENC_SJIS;
    } else if (rb_funcall(object, rb_intern("=="), 1, RB_GRN_ENCODING_LATIN1)) {
        return GRN_ENC_LATIN1;
    } else if (rb_funcall(object, rb_intern("=="), 1, RB_GRN_ENCODING_KOI8R)) {
        return GRN_ENC_KOI8R;
    } else {
        rb_raise(rb_eGrnError, "unknown encoding: %s", rb_grn_inspect(object));
    }

    return GRN_ENC_DEFAULT;
}

void
rb_grn_init_encoding (VALUE mGroonga)
{
    mGrnEncoding = rb_define_module_under(mGroonga, "Encoding");

#define DEFINE_ENCODING(name, value)                                    \
    RB_GRN_ENCODING_ ## name = RB_GRN_INTERN(value);                    \
    rb_define_const(mGrnEncoding, #name, RB_GRN_ENCODING_ ## name)

    DEFINE_ENCODING(DEFAULT, "default");
    DEFINE_ENCODING(NONE, "none");
    DEFINE_ENCODING(EUC_JP, "euc_jp");
    DEFINE_ENCODING(SJIS, "sjis");
    DEFINE_ENCODING(UTF8, "utf8");
    DEFINE_ENCODING(LATIN1, "latin1");
    DEFINE_ENCODING(KOI8R, "koi8r");

#undef DEFINE_ENCODING
}
