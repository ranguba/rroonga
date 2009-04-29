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

#include "rb-grn.h"

/*
 * Document-module: Groonga::Encoding
 *
 * groongaがサポートしてるエンコーディングが定義されてい
 * るモジュールです。
 */

static VALUE mGrnEncoding;
static VALUE RB_GRN_ENCODING_DEFAULT;
static VALUE RB_GRN_ENCODING_NONE;
static VALUE RB_GRN_ENCODING_EUC_JP;
static VALUE RB_GRN_ENCODING_UTF8;
static VALUE RB_GRN_ENCODING_SJIS;
static VALUE RB_GRN_ENCODING_LATIN1;
static VALUE RB_GRN_ENCODING_KOI8R;

grn_encoding
rb_grn_encoding_from_ruby_object (VALUE object, grn_ctx *context)
{
    if (NIL_P(object)) {
	if (context)
	    return context->encoding;
	else
	    return GRN_ENC_DEFAULT;
    }

    if (rb_grn_equal_option(object, "default")) {
        return GRN_ENC_DEFAULT;
    } else if (rb_grn_equal_option(object, "none")) {
        return GRN_ENC_NONE;
    } else if (rb_grn_equal_option(object, "euc_jp") ||
	       rb_grn_equal_option(object, "euc-jp")) {
        return GRN_ENC_EUC_JP;
    } else if (rb_grn_equal_option(object, "utf8") ||
	       rb_grn_equal_option(object, "utf-8")) {
        return GRN_ENC_UTF8;
    } else if (rb_grn_equal_option(object, "sjis") ||
	       rb_grn_equal_option(object, "shift_jis") ||
	       rb_grn_equal_option(object, "shift-jis")) {
        return GRN_ENC_SJIS;
    } else if (rb_grn_equal_option(object, "latin1")) {
        return GRN_ENC_LATIN1;
    } else if (rb_grn_equal_option(object, "koi8r")) {
        return GRN_ENC_KOI8R;
    } else {
        rb_raise(rb_eGrnError, "unknown encoding: %s", rb_grn_inspect(object));
    }

    return GRN_ENC_DEFAULT;
}

VALUE
rb_grn_encoding_to_ruby_object (grn_encoding encoding)
{
    VALUE rb_encoding;

    switch (encoding) {
      case GRN_ENC_DEFAULT:
	rb_encoding = RB_GRN_ENCODING_DEFAULT;
        break;
      case GRN_ENC_NONE:
	rb_encoding = RB_GRN_ENCODING_NONE;
        break;
      case GRN_ENC_EUC_JP:
	rb_encoding = RB_GRN_ENCODING_EUC_JP;
        break;
      case GRN_ENC_UTF8:
	rb_encoding = RB_GRN_ENCODING_UTF8;
        break;
      case GRN_ENC_SJIS:
	rb_encoding = RB_GRN_ENCODING_SJIS;
        break;
      case GRN_ENC_LATIN1:
	rb_encoding = RB_GRN_ENCODING_LATIN1;
        break;
      case GRN_ENC_KOI8R:
	rb_encoding = RB_GRN_ENCODING_KOI8R;
        break;
      default:
	rb_raise(rb_eArgError, "unknown encoding: %d", encoding);
	break;
    }

    return rb_encoding;
}

void
rb_grn_init_encoding (VALUE mGrn)
{
    mGrnEncoding = rb_define_module_under(mGrn, "Encoding");

#define DEFINE_ENCODING(name, value)                                    \
    RB_GRN_ENCODING_ ## name = RB_GRN_INTERN(value);                    \
    rb_define_const(mGrnEncoding, #name, RB_GRN_ENCODING_ ## name)

    RB_GRN_ENCODING_DEFAULT = RB_GRN_INTERN("default");
    /* groongaをビルドしたときに指定したエンコーディング。 */
    rb_define_const(mGrnEncoding, "DEFAULT", RB_GRN_ENCODING_DEFAULT);

    RB_GRN_ENCODING_NONE = RB_GRN_INTERN("none");
    /* 文字列をバイト列として扱うエンコーディング。 */
    rb_define_const(mGrnEncoding, "NONE", RB_GRN_ENCODING_NONE);

    RB_GRN_ENCODING_EUC_JP = RB_GRN_INTERN("euc_jp");
    /* EUC-JP */
    rb_define_const(mGrnEncoding, "EUC_JP", RB_GRN_ENCODING_EUC_JP);

    RB_GRN_ENCODING_SJIS = RB_GRN_INTERN("sjis");
    /* ShiftJIS */
    rb_define_const(mGrnEncoding, "SJIS", RB_GRN_ENCODING_SJIS);

    RB_GRN_ENCODING_UTF8 = RB_GRN_INTERN("utf8");
    /* UTF-8 */
    rb_define_const(mGrnEncoding, "UTF8", RB_GRN_ENCODING_UTF8);

    RB_GRN_ENCODING_LATIN1 = RB_GRN_INTERN("latin1");
    /* Latin-1。ISO-8859-1ではなくWindows-1252(CP1252)。 */
    rb_define_const(mGrnEncoding, "LATIN1", RB_GRN_ENCODING_LATIN1);

    RB_GRN_ENCODING_KOI8R = RB_GRN_INTERN("koi8r");
    /* KOI8-R */
    rb_define_const(mGrnEncoding, "KOI8R", RB_GRN_ENCODING_KOI8R);

#undef DEFINE_ENCODING
}
