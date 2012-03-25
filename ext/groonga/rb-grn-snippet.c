/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009-2012  Kouhei Sutou <kou@clear-code.com>

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

#define SELF(object) ((RbGrnSnippet *)DATA_PTR(object))

VALUE rb_cGrnSnippet;

/*
 * Document-class: Groonga::Snippet
 *
 * スニペット（検索語周辺のテキスト）を生成するためのオブジェクト。
 */

void
rb_grn_snippet_finalizer (grn_ctx *context, grn_obj *object,
			  RbGrnSnippet *rb_grn_snippet)
{
    rb_grn_context_unregister_floating_object(RB_GRN_OBJECT(rb_grn_snippet));
}

void
rb_grn_snippet_bind (RbGrnSnippet *rb_grn_snippet,
		     grn_ctx *context, grn_obj *snippet)
{
}

void
rb_grn_snippet_deconstruct (RbGrnSnippet *rb_grn_snippet,
			    grn_obj **snippet,
			    grn_ctx **context)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_snippet);
    rb_grn_object_deconstruct(rb_grn_object, snippet, context,
			      NULL, NULL,
			      NULL, NULL);
}

/*
 * call-seq:
 *   Groonga::Snippet.new(options={})
 *
 * スニペットを作成する。 _options_ に指定可能な値は以下の通
 * り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :context (Groonga::Context.default) The context
 *
 *   スキーマ作成時に使用するGroonga::Contextを指定する。
 *
 * @option options :normalize The normalize
 *
 *   キーワード文字列・スニペット元の文字列を正規化するかど
 *   うか。省略した場合は +false+ で正規化しない。
 *
 * @option options :skip_leading_spaces (false) The skip_leading_spaces
 *
 *   先頭の空白を無視するかどうか。省略した場合は で無
 *   視しない。
 *
 * @option options :width (100) The width
 *
 *   スニペット文字列の長さ。省略した場合は100文字。
 *
 * @option options :max_results (3) The max_results
 *   生成するスニペットの最大数。省略した場合は3。
 *
 * @option options :html_escape (false) The html_escape
 *   スニペット内の+<+, +>+, +&+, +"+をHTMLエスケープするか
 *   どうか。省略した場合は+false+で、HTMLエスケープしない。
 *
 * @option options :default_open_tag ("") The default_open_tag
 *   デフォルトの開始タグ。省略した場合は""(空文字列)
 *
 * @option options :default_close_tag ("") The default_close_tag
 *   デフォルトの終了タグ。省略した場合は""(空文字列)
 */
static VALUE
rb_grn_snippet_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_snip *snippet = NULL;
    VALUE options;
    VALUE rb_context, rb_normalize, rb_skip_leading_spaces;
    VALUE rb_width, rb_max_results, rb_default_open_tag, rb_default_close_tag;
    VALUE rb_html_escape;
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
                        "normalize", &rb_normalize,
                        "skip_leading_spaces", &rb_skip_leading_spaces,
                        "width", &rb_width,
                        "max_results", &rb_max_results,
                        "default_open_tag", &rb_default_open_tag,
                        "default_close_tag", &rb_default_close_tag,
                        "html_escape", &rb_html_escape,
                        NULL);

    context = rb_grn_context_ensure(&rb_context);

    if (RVAL2CBOOL(rb_normalize))
        flags |= GRN_SNIP_NORMALIZE;
    if (RVAL2CBOOL(rb_skip_leading_spaces))
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

    if (RVAL2CBOOL(rb_html_escape))
        mapping = (grn_snip_mapping *)-1;

    snippet = grn_snip_open(context, flags, width, max_results,
                            default_open_tag, default_open_tag_length,
                            default_close_tag, default_close_tag_length,
                            mapping);
    rb_grn_context_check(context, rb_ary_new4(argc, argv));

    rb_grn_object_assign(Qnil, self, rb_context, context, (grn_obj *)snippet);
    rb_grn_context_register_floating_object(DATA_PTR(self));

    rb_iv_set(self, "@context", rb_context);

    return Qnil;
}

/*
 * call-seq:
 *   snippet.add_keyword(keyword, options={})
 *
 * _keyword_ を追加する。 _options_ に指定可能な値は
 * 以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :open_tag (default_open_tag) The open_tag
 *
 *   開始タグ。省略した場合はGroonga::Snippet.newで指定し
 *   た+:default_open_tag+。
 *
 * @option options :close_tag (:default_close_tag) The close_tag
 *
 *   終了タグ。省略した場合はGroonga::Snippet.newで指定し
 *   た+:default_close_tag+。
 */
static VALUE
rb_grn_snippet_add_keyword (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *snippet;
    grn_rc rc;
    VALUE rb_keyword, options;
    VALUE rb_open_tag, rb_close_tag;
    char *keyword, *open_tag = NULL, *close_tag = NULL;
    unsigned int keyword_length, open_tag_length = 0, close_tag_length = 0;

    rb_grn_snippet_deconstruct(SELF(self), &snippet, &context);

    rb_scan_args(argc, argv, "11", &rb_keyword, &options);

    keyword = StringValuePtr(rb_keyword);
    keyword_length = RSTRING_LEN(rb_keyword);

    rb_grn_scan_options(options,
                        "open_tag", &rb_open_tag,
                        "close_tag", &rb_close_tag,
                        NULL);

    if (!NIL_P(rb_open_tag)) {
        open_tag = StringValuePtr(rb_open_tag);
        open_tag_length = RSTRING_LEN(rb_open_tag);
    }

    if (!NIL_P(rb_close_tag)) {
        close_tag = StringValuePtr(rb_close_tag);
        close_tag_length = RSTRING_LEN(rb_close_tag);
    }

    rc = grn_snip_add_cond(context, (grn_snip *)snippet,
                           keyword, keyword_length,
                           open_tag, open_tag_length,
                           close_tag, close_tag_length);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   snippet.execute(string) -> スニペットの配列
 *
 * _string_ を走査し、スニペットを作成する。
 */
static VALUE
rb_grn_snippet_execute (VALUE self, VALUE rb_string)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *snippet;
    char *string;
    unsigned int string_length;
    unsigned int i, n_results, max_tagged_length;
    VALUE rb_results;
    char *result;

    rb_grn_snippet_deconstruct(SELF(self), &snippet, &context);

    if (TYPE(rb_string) != T_STRING) {
	rb_raise(rb_eGrnInvalidArgument,
		 "snippet text must be String: <%s>",
		 rb_grn_inspect(rb_string));
    }

#ifdef HAVE_RUBY_ENCODING_H
    rb_string = rb_grn_context_rb_string_encode(context, rb_string);
#endif
    string = StringValuePtr(rb_string);
    string_length = RSTRING_LEN(rb_string);

    rc = grn_snip_exec(context, (grn_snip *)snippet, string, string_length,
                       &n_results, &max_tagged_length);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    rb_results = rb_ary_new2(n_results);
    result = ALLOCA_N(char, max_tagged_length);
    for (i = 0; i < n_results; i++) {
        VALUE rb_result;
        unsigned result_length;

        rc = grn_snip_get_result(context, (grn_snip *)snippet,
				 i, result, &result_length);
        rb_grn_rc_check(rc, self);
        rb_result = rb_grn_context_rb_string_new(context, result, result_length);
        rb_ary_push(rb_results, rb_result);
    }

    return rb_results;
}

void
rb_grn_init_snippet (VALUE mGrn)
{
    rb_cGrnSnippet = rb_define_class_under(mGrn, "Snippet", rb_cGrnObject);

    rb_define_method(rb_cGrnSnippet, "initialize",
                     rb_grn_snippet_initialize, -1);
    rb_define_method(rb_cGrnSnippet, "add_keyword",
                     rb_grn_snippet_add_keyword, -1);
    rb_define_method(rb_cGrnSnippet, "execute",
                     rb_grn_snippet_execute, 1);
}
