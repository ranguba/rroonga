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

#define SELF(object) (rb_rb_grn_snippet_from_ruby_object(object))

typedef struct _RbGrnSnippet RbGrnSnippet;
struct _RbGrnSnippet
{
    grn_ctx *context;
    grn_snip *snippet;
};

VALUE rb_cGrnSnippet;

static RbGrnSnippet *
rb_rb_grn_snippet_from_ruby_object (VALUE object)
{
    RbGrnSnippet *rb_grn_snippet;

    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnSnippet))) {
	rb_raise(rb_eTypeError, "not a groonga snippet");
    }

    Data_Get_Struct(object, RbGrnSnippet, rb_grn_snippet);
    if (!rb_grn_snippet)
	rb_raise(rb_eGrnError, "groonga snippet is NULL");

    return rb_grn_snippet;
}

grn_snip *
rb_grn_snippet_from_ruby_object (VALUE object)
{
    if (NIL_P(object))
        return NULL;

    return SELF(object)->snippet;
}

static void
rb_rb_grn_snippet_free (void *object)
{
    RbGrnSnippet *rb_grn_snippet = object;

    if (rb_grn_snippet->snippet) {
        grn_snip_close(rb_grn_snippet->context,
                       rb_grn_snippet->snippet);
        rb_grn_snippet->snippet = NULL;
    }

    xfree(object);
}

VALUE
rb_grn_snippet_to_ruby_object (grn_ctx *context, grn_snip *snippet)
{
    RbGrnSnippet *rb_grn_snippet;

    if (!snippet)
        return Qnil;

    rb_grn_snippet = ALLOC(RbGrnSnippet);
    rb_grn_snippet->context = context;
    rb_grn_snippet->snippet = snippet;

    return Data_Wrap_Struct(rb_cGrnSnippet, NULL,
                            rb_rb_grn_snippet_free, rb_grn_snippet);
}

static VALUE
rb_grn_snippet_alloc (VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, rb_rb_grn_snippet_free, NULL);
}

static VALUE
rb_grn_snippet_initialize (int argc, VALUE *argv, VALUE self)
{
    RbGrnSnippet *rb_grn_snippet;
    grn_ctx *context = NULL;
    grn_snip *snippet = NULL;
    VALUE options;
    VALUE rb_context, rb_encoding, rb_normalize, rb_skip_reading_spaces;
    VALUE rb_width, rb_max_results, rb_default_open_tag, rb_default_close_tag;
    VALUE rb_html_escape;
    grn_encoding encoding;
    int flags = GRN_SNIP_COPY_TAG;
    unsigned int width = 100;
    unsigned int max_results = 3;
    char *default_open_tag = NULL;
    unsigned int default_open_tag_length = 0;
    char *default_close_tag = NULL;
    unsigned int default_close_tag_length = 0;
    grn_snip_mapping *mapping = NULL;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
                        "context", &rb_context,
                        "encoding", &rb_encoding,
                        "normalize", &rb_normalize,
                        "skip_reading_spaces", &rb_skip_reading_spaces,
                        "width", &rb_width,
                        "max_results", &rb_max_results,
                        "default_open_tag", &rb_default_open_tag,
                        "default_close_tag", &rb_default_close_tag,
                        "html_escape", &rb_html_escape,
                        NULL);

    context = rb_grn_context_ensure(rb_context);
    encoding = RVAL2GRNENCODING(rb_encoding);

    if (RVAL2CBOOL(rb_normalize))
        flags |= GRN_SNIP_NORMALIZE;
    if (RVAL2CBOOL(rb_skip_reading_spaces))
        flags |= GRN_SNIP_SKIP_LEADING_SPACES;

    if (!NIL_P(rb_width))
        width = NUM2UINT(rb_width);

    if (!NIL_P(rb_max_results))
        max_results = NUM2UINT(rb_max_results);

    if (!NIL_P(rb_default_open_tag)) {
        default_open_tag = StringValuePtr(rb_default_open_tag);
        default_open_tag_length = RSTRING_LEN(rb_default_open_tag);
    }

    if (!NIL_P(rb_default_close_tag)) {
        default_close_tag = StringValuePtr(rb_default_close_tag);
        default_close_tag_length = RSTRING_LEN(rb_default_close_tag);
    }

    if (CBOOL2RVAL(rb_html_escape))
        mapping = (grn_snip_mapping *)-1;

    snippet = grn_snip_open(context, encoding, flags, width, max_results,
                            default_open_tag, default_open_tag_length,
                            default_close_tag, default_close_tag_length,
                            mapping);

    rb_grn_snippet = ALLOC(RbGrnSnippet);
    DATA_PTR(self) = rb_grn_snippet;
    rb_grn_snippet->context = context;
    rb_grn_snippet->snippet = snippet;

    return Qnil;
}

void
rb_grn_init_snippet (VALUE mGrn)
{
    rb_cGrnSnippet = rb_define_class_under(mGrn, "Snippet", rb_cObject);
    rb_define_alloc_func(rb_cGrnSnippet, rb_grn_snippet_alloc);

    rb_define_method(rb_cGrnSnippet, "initialize",
                     rb_grn_snippet_initialize, -1);
}
