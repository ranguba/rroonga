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

#define SELF(object) ((RbGrnExpression *)DATA_PTR(object))

VALUE rb_cGrnExpression;

/*
 * Document-class: Groonga::Expression < Groonga::Object
 *
 * 検索条件やデータベースへの操作を表現するオブジェクト。
 *
 */

void
rb_grn_expression_finalizer (grn_ctx *context, grn_obj *object,
			     RbGrnExpression *rb_grn_expression)
{
    if (context && rb_grn_expression->value)
	grn_obj_unlink(context, rb_grn_expression->value);

    rb_grn_context_unregister_floating_object(RB_GRN_OBJECT(rb_grn_expression));

    rb_grn_expression->value = NULL;
}

void
rb_grn_expression_bind (RbGrnExpression *rb_grn_expression,
                        grn_ctx *context, grn_obj *expression)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_expression);

    rb_grn_expression->value = grn_obj_open(context, GRN_BULK, 0,
                                            rb_grn_object->range_id);
}

void
rb_grn_expression_deconstruct (RbGrnExpression *rb_grn_expression,
                               grn_obj **expression,
                               grn_ctx **context,
                               grn_id *domain_id,
                               grn_obj **domain,
			       grn_obj **value,
                               grn_id *range_id,
                               grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_expression);
    rb_grn_object_deconstruct(rb_grn_object, expression, context,
			      domain_id, domain,
			      range_id, range);

    if (value)
	*value = rb_grn_expression->value;
}

static VALUE
rb_grn_expression_initialize (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression;
    VALUE options, rb_context, rb_name;
    char *name = NULL;
    unsigned name_size = 0;

    rb_scan_args(argc, argv, "01", &options);
    rb_grn_scan_options(options,
                        "context", &rb_context,
                        "name", &rb_name,
                        NULL);

    context = rb_grn_context_ensure(&rb_context);

    if (!NIL_P(rb_name)) {
	name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
    }

    expression = grn_expr_create(context, name, name_size);
    rb_grn_context_check(context, self);
    rb_grn_object_assign(Qnil, self, rb_context, context, expression);
    rb_grn_context_register_floating_object(DATA_PTR(self));

    rb_iv_set(self, "@objects", rb_ary_new());

    return Qnil;
}

/*
 * call-seq:
 *   expression.define_variable(options={}) -> Groonga::Variable
 *
 * _expression_ で使用可能な変数を作成する。
 *
 * _options_ に指定可能な値は以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :name (false)
 *
 *   変数の名前。省略した場合は名前を付けない。
 * @option options :domain
 *
 *   テーブルを指定すると、そのテーブル用のレコードとして初期化する。
 */
static VALUE
rb_grn_expression_define_variable (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression, *variable;
    char *name = NULL;
    unsigned name_size = 0;
    VALUE options, rb_name, rb_domain, rb_variable;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    rb_grn_scan_options(options,
			"name", &rb_name,
			"domain", &rb_domain,
			NULL);

    if (!NIL_P(rb_name)) {
	name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
    }

    variable = grn_expr_add_var(context, expression, name, name_size);
    rb_variable = GRNVARIABLE2RVAL(context, variable);

    if (RVAL2CBOOL(rb_obj_is_kind_of(rb_domain, rb_cGrnTable))) {
	grn_id domain_id;
	domain_id = NUM2UINT(rb_funcall(rb_domain, rb_intern("id"), 0));
	GRN_RECORD_INIT(variable, 0, domain_id);
    }

    return rb_variable;
}

/*
 * call-seq:
 *   expression.append_object(object,
 *                            operation=Groonga::Operator::PUSH,
 *                            n_arguments=1) -> self
 *
 * _object_ を追加し、 _n_arguments_ 個の引数を取る _operation_ を追加する。
 */
static VALUE
rb_grn_expression_append_object (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_object, rb_operation, rb_n_arguments;
    grn_ctx *context = NULL;
    grn_obj *expression, *object;
    grn_operator operation = GRN_OP_PUSH;
    int n_arguments = 1;

    rb_scan_args(argc, argv, "12", &rb_object, &rb_operation, &rb_n_arguments);
    if (!NIL_P(rb_operation))
        operation = NUM2INT(rb_operation);
    if (!NIL_P(rb_n_arguments))
        n_arguments = NUM2INT(rb_n_arguments);

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    object = RVAL2GRNOBJECT(rb_object, &context);
    grn_expr_append_obj(context, expression, object,
                        operation, n_arguments);
    rb_grn_context_check(context, self);
    rb_ary_push(rb_iv_get(self, "@objects"), rb_object);

    return self;
}

/*
 * call-seq:
 *   expression.append_constant(constant,
 *                              operation=Groonga::Operator::PUSH,
 *                              n_arguments=1) -> self
 *
 * _constant_ を追加し、 _n_arguments_ 個の引数を取る _operation_ を追加する。
 */
static VALUE
rb_grn_expression_append_constant (int argc, VALUE *argv, VALUE self)
{
    VALUE rb_constant, rb_operator, rb_n_arguments;
    VALUE exception;
    grn_ctx *context = NULL;
    grn_obj *expression, *constant = NULL;
    grn_operator operator = GRN_OP_PUSH;
    int n_arguments = 1;

    rb_scan_args(argc, argv, "12", &rb_constant, &rb_operator, &rb_n_arguments);
    if (!NIL_P(rb_operator))
        operator = NUM2INT(rb_operator);
    if (!NIL_P(rb_n_arguments))
        n_arguments = NUM2INT(rb_n_arguments);

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL, NULL,
				  NULL, NULL);

    RVAL2GRNOBJ(rb_constant, context, &constant);
    grn_expr_append_const(context, expression, constant, operator, n_arguments);

    exception = rb_grn_context_to_exception(context, self);
    grn_obj_unlink(context, constant);
    if (!NIL_P(exception))
	rb_exc_raise(exception);

    return self;
}

/*
 * call-seq:
 *   expression.append_operation(operation, n_arguments)
 *
 * _n_arguments_ 個の引数を取る _operation_ を追加する。
 */
static VALUE
rb_grn_expression_append_operation (VALUE self, VALUE rb_operation,
				    VALUE rb_n_arguments)
{
    grn_ctx *context = NULL;
    grn_obj *expression;
    grn_operator operation;
    int n_arguments = 0;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    operation = NUM2INT(rb_operation);
    n_arguments = NUM2INT(rb_n_arguments);
    grn_expr_append_op(context, expression, operation, n_arguments);
    rb_grn_context_check(context, self);
    return Qnil;
}

/*
 * call-seq:
 *   expression.parse(query, options={})
 *
 * 文字列 _query_ をパースする。
 *
 * _options_ に指定可能な値は以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :default_column The default_column
 *
 *   "column_name:hoge"ではなく"hoge"のようにcolumn_nameが指
 *   定されない条件の検索対象となるカラムを指定する。
 * @option options :default_operator (Groonga::Operator::AND)
 *
 *   The default_operator
 *   "+"や"OR"で繋がれず、ただ列挙された複数の条件があった時、
 *   _expression_ 全体として各レコードをヒットとみなすかの論理
 *   条件を指定する。省略した場合はGroonga::Operator::AND。
 *   [Groonga::Operator::OR]
 *     レコードはいずれかの条件にマッチすればいい。
 *   [Groonga::Operator::AND]
 *     レコードは全ての条件にマッチしなければならない。
 *   [Groonga::Operator::BUT]
 *     最初の条件にレコードはマッチし、残りの条件にレコードは
 *     マッチしてはならない。
 * @option options :default_mode (Groonga::Operator::MATCH) The default_mode
 *
 *   検索時のモードを指定する。省略した場合は
 *   Groonga::Operator::MATCH。（FIXME: モードによってどう
 *   いう動作になるかを書く。）
 *
 * @option options :syntax (:query) The syntax
 *
 *   _query_ の構文を指定する。指定可能な値は以下の通り。省略
 *   した場合は +:query+ 。
 *
 *   [+nil+]
 *     +:query+と同様。
 *   [+:query+]
 *     「文字列1 OR 文字列2」で「"文字列1"あるいは"文字列2"
 *     にマッチという検索エンジンで利用できるような構文を使
 *     う。
 *
 *     参考: grn式のquery形式（link:text/expression_rdoc.html）
 *   [+:script+]
 *     「[カラム名] == [値]」というようにECMAScript風の構文
 *     を使う。
 *
 *     参考: grn式のscript形式（link:text/expression_rdoc.html）
 *
 * @option options :allow_pragma The allow_pragma
 *   _query_ の構文に query を用いているとき（ +:syntax+
 *   オプション参照）、「*E-1」というようにクエリの先頭で
 *   pragmaを利用できるようにする。script構文を用いている
 *   ときはこのオプションを利用できない。
 *
 *   デフォルトではプラグマを利用できる。
 *
 *   参考: grn式のquery形式（link:text/expression_rdoc.html）
 *
 * @option options :allow_column The allow_column
 *
 *   _query_ の構文にqueryを用いているとき（+:syntax+オプショ
 *   ン参照）、「カラム名:値」というようにカラム名を指定した
 *   条件式を利用できるようにする。script構文を用いていると
 *   きはこのオプションを利用できない。
 *
 *   デフォルトではカラム名を指定した条件式を利用できる。
 *
 *   参考: grn式のquery形式（link:text/expression_rdoc.html）
 *
 * @option options :allow_update The allow_update
 *
 *   _query_の構文にscriptを用いているとき（ +:syntax+ オプショ
 *   ン参照）、「カラム名 = 値」というように更新操作を利用で
 *   きるようにする。query構文を用いているときはこのオプショ
 *   ンを利用できない。
 *
 *   デフォルトでは更新操作を利用できる。
 *
 *   参考: grn式のscript形式（link:text/expression_rdoc.html）
 */
static VALUE
rb_grn_expression_parse (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression, *default_column;
    grn_bool default_column_is_created = GRN_FALSE;
    grn_operator default_operator = GRN_OP_AND;
    grn_operator default_mode = GRN_OP_MATCH;
    grn_rc rc;
    char *query = NULL;
    unsigned query_size = 0;
    grn_expr_flags flags = 0;
    VALUE options, rb_query, rb_default_column, rb_default_operator;
    VALUE rb_default_mode, rb_syntax;
    VALUE rb_allow_pragma, rb_allow_column, rb_allow_update;
    VALUE exception = Qnil;

    rb_scan_args(argc, argv, "11", &rb_query, &options);
    rb_grn_scan_options(options,
                        "default_column", &rb_default_column,
                        "default_operator", &rb_default_operator,
                        "default_mode", &rb_default_mode,
			"syntax", &rb_syntax,
			"allow_pragma", &rb_allow_pragma,
			"allow_column", &rb_allow_column,
			"allow_update", &rb_allow_update,
                        NULL);

    query = StringValuePtr(rb_query);
    query_size = RSTRING_LEN(rb_query);

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    if (NIL_P(rb_default_column)) {
	default_column = NULL;
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(rb_default_column, rb_cGrnObject))) {
	default_column = RVAL2GRNOBJECT(rb_default_column, &context);
    } else {
	default_column = RVAL2GRNBULK(rb_default_column, context, NULL);
	default_column_is_created = GRN_TRUE;
    }

    if (!NIL_P(rb_default_mode))
	default_mode = RVAL2GRNOPERATOR(rb_default_mode);
    if (!NIL_P(rb_default_operator))
	default_operator = RVAL2GRNOPERATOR(rb_default_operator);

    if (NIL_P(rb_syntax) ||
	rb_grn_equal_option(rb_syntax, "query")) {
	flags = GRN_EXPR_SYNTAX_QUERY;
    } else if (rb_grn_equal_option(rb_syntax, "script")) {
	flags = GRN_EXPR_SYNTAX_SCRIPT;
    } else {
	rb_raise(rb_eArgError,
		 "syntax should be one of "
		 "[nil, :query, :script]: %s",
		 rb_grn_inspect(rb_syntax));
    }

    if (NIL_P(rb_allow_pragma)) {
	if (!(flags & GRN_EXPR_SYNTAX_SCRIPT))
	    flags |= GRN_EXPR_ALLOW_PRAGMA;
    } else {
	if ((flags & GRN_EXPR_SYNTAX_SCRIPT))
	    rb_raise(rb_eArgError,
		     ":allow_pragma isn't allowed in script syntax");
	if (RVAL2CBOOL(rb_allow_pragma))
	    flags |= GRN_EXPR_ALLOW_PRAGMA;
    }

    if (NIL_P(rb_allow_column)) {
	if (!(flags & GRN_EXPR_SYNTAX_SCRIPT))
	    flags |= GRN_EXPR_ALLOW_COLUMN;
    } else {
	if ((flags & GRN_EXPR_SYNTAX_SCRIPT))
	    rb_raise(rb_eArgError,
		     ":allow_column isn't allowed in script syntax");
	if (RVAL2CBOOL(rb_allow_column))
	    flags |= GRN_EXPR_ALLOW_COLUMN;
    }

    if (NIL_P(rb_allow_update)) {
	flags |= GRN_EXPR_ALLOW_UPDATE;
    } else {
	if (RVAL2CBOOL(rb_allow_update))
	    flags |= GRN_EXPR_ALLOW_UPDATE;
    }

    rc = grn_expr_parse(context, expression, query, query_size,
			default_column, default_mode, default_operator,
			flags);
    if (rc != GRN_SUCCESS) {
	VALUE related_object;

	related_object = rb_ary_new3(2, self, rb_ary_new4(argc, argv));
	exception = rb_grn_context_to_exception(context, related_object);
    }
    if (default_column_is_created)
	grn_obj_unlink(context, default_column);

    if (!NIL_P(exception))
	rb_exc_raise(exception);

    return Qnil;
}

/*
 * call-seq:
 *   expression.execute -> 値
 *
 * _expression_を実行し、実行した結果を返す。
 */
static VALUE
rb_grn_expression_execute (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression;
    grn_obj *result;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    result = grn_expr_exec(context, expression, 0);
    rb_grn_context_check(context, self);

    return GRNOBJ2RVAL(Qnil, context, result, self);
}

/*
 * call-seq:
 *   expression.compile
 *
 * _expression_をコンパイルする。
 */
static VALUE
rb_grn_expression_compile (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression;
    grn_rc rc;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    rc = grn_expr_compile(context, expression);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * call-seq:
 *   expression[name] -> 変数の値
 *   expression[offset] -> 変数の値
 *
 * _expression_ で使用可能な変数のうち、名前が _name_ または
 * _offset_ 番目にExpression#append_objectされた変数の値を返
 * す。
 */
static VALUE
rb_grn_expression_array_reference (VALUE self, VALUE rb_name_or_offset)
{
    grn_ctx *context = NULL;
    grn_obj *expression, *variable, *value;
    char *name = NULL;
    unsigned name_size = 0;
    int offset;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    switch (TYPE(rb_name_or_offset)) {
    case T_STRING:
        name = RSTRING_PTR(rb_name_or_offset);
        name_size = RSTRING_LEN(rb_name_or_offset);
        variable = grn_expr_get_var(context, expression, name, name_size);
        return GRNBULK2RVAL(context, variable, NULL, self);
        break;
    case T_FIXNUM:
        offset = NUM2INT(rb_name_or_offset);
        value = grn_expr_get_var_by_offset(context, expression, offset);
        return GRNBULK2RVAL(context, value, NULL, self);
        break;
    default:
        rb_raise(rb_eArgError, "xxx");
        break;
    }

    return Qnil;
}

/* REMOVE ME */
grn_rc grn_expr_inspect(grn_ctx *ctx, grn_obj *buf, grn_obj *expr);

/*
 * call-seq:
 *   _expression_.inspect -> String
 *
 * _expression_ の中身を人に見やすい文字列で返す。
 */
static VALUE
rb_grn_expression_inspect (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj inspected;
    grn_obj *expression;
    VALUE rb_inspected;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    GRN_TEXT_INIT(&inspected, 0);
    GRN_TEXT_PUTS(context, &inspected, "#<Groonga::Expression ");
#ifdef WIN32
    GRN_TEXT_PUTS(context, &inspected, "(not supported on Windows)");
#else
    grn_expr_inspect(context, &inspected, expression);
#endif
    GRN_TEXT_PUTS(context, &inspected, ">");
    rb_inspected = rb_grn_context_rb_string_new(context,
						GRN_TEXT_VALUE(&inspected),
						GRN_TEXT_LEN(&inspected));
    GRN_OBJ_FIN(context, &inspected);

    return rb_inspected;
}

/*
 * call-seq:
 *   expression.snippet(tags, options) -> Groonga::Snippet
 *
 * _expression_ からGroonga::Snippetを生成する。 _tags_ には
 * キーワードの前後に挿入するタグの配列を以下のような形式で指定
 * する。
 *
 * <pre>
 * !!!ruby
 * [
 *  ["キーワード前に挿入する文字列1", "キーワード後に挿入する文字列1"],
 *  ["キーワード前に挿入する文字列2", "キーワード後に挿入する文字列2"],
 *  # ...,
 * ]
 * </pre>
 *
 * もし、1つのスニペットの中に _tags_ で指定したタグより多くの
 * キーワードが含まれている場合は、以下のように、また、先頭
 * のタグから順番に使われる。
 *
 * <pre>
 * !!!ruby
 * expression.parse("Ruby groonga 検索")
 * tags = [["<tag1>", "</tag1>"], ["<tag2>", "</tag2>"]]
 * snippet = expression.snippet(tags)
 * p snippet.execute("Rubyでgroonga使って全文検索、高速検索。")
 *    # => ["<tag1>Ruby</tag1>で<tag2>groonga</tag2>"
 *    # =>  "使って全文<tag1>検索</tag1>、高速<tag2>検索</tag2>。"]
 * </pre>
 *
 * _options_ に指定可能な値は以下の通り。
 * @param options [::Hash] The name and value
 *   pairs. Omitted names are initialized as the default value.
 * @option options :normalize (false) The normalize
 *
 *   キーワード文字列・スニペット元の文字列を正規化するかど
 *   うか。省略した場合は +false+ で正規化しない。
 *
 * @option options :skip_leading_spaces (false) The skip_leading_spaces
 *
 *   先頭の空白を無視するかどうか。省略した場合は +false+ で無
 *   視しない。
 *
 * @option options :width (100) The width
 *
 *   スニペット文字列の長さ。省略した場合は100文字。
 * @option options :max_results (3) The max_results
 *
 *   生成するスニペットの最大数。省略した場合は3。
 * @option options :html_escape (false) The html_escape
 *
 *   スニペット内の +<+, +>+, +&+, +"+ をHTMLエスケープするか
 *   どうか。省略した場合は +false+ で、HTMLエスケープしない。
 */
static VALUE
rb_grn_expression_snippet (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *expression;
    grn_snip *snippet;
    VALUE options;
    VALUE rb_normalize, rb_skip_leading_spaces;
    VALUE rb_width, rb_max_results, rb_tags;
    VALUE rb_html_escape;
    VALUE *rb_tag_values;
    VALUE related_object;
    unsigned int i;
    int flags = GRN_SNIP_COPY_TAG;
    unsigned int width = 100;
    unsigned int max_results = 3;
    unsigned int n_tags = 0;
    char **open_tags = NULL;
    unsigned int *open_tag_lengths = NULL;
    char **close_tags = NULL;
    unsigned int *close_tag_lengths = NULL;
    grn_snip_mapping *mapping = NULL;

    rb_grn_expression_deconstruct(SELF(self), &expression, &context,
                                  NULL, NULL,
                                  NULL, NULL, NULL);

    rb_scan_args(argc, argv, "11", &rb_tags, &options);

    rb_grn_scan_options(options,
                        "normalize", &rb_normalize,
                        "skip_leading_spaces", &rb_skip_leading_spaces,
                        "width", &rb_width,
                        "max_results", &rb_max_results,
                        "html_escape", &rb_html_escape,
                        NULL);

    if (TYPE(rb_tags) != T_ARRAY) {
	rb_raise(rb_eArgError,
		 "tags should be "
		 "[\"open_tag\", \"close_tag\"] or "
		 "[[\"open_tag1\", \"close_tag1\"], ...]: %s",
		 rb_grn_inspect(rb_tags));
    }

    if (TYPE(RARRAY_PTR(rb_tags)[0]) == T_STRING) {
	rb_tags = rb_ary_new3(1, rb_tags);
    }

    rb_tag_values = RARRAY_PTR(rb_tags);
    n_tags = RARRAY_LEN(rb_tags);
    open_tags = ALLOCA_N(char *, n_tags);
    open_tag_lengths = ALLOCA_N(unsigned int, n_tags);
    close_tags = ALLOCA_N(char *, n_tags);
    close_tag_lengths = ALLOCA_N(unsigned int, n_tags);
    for (i = 0; i < n_tags; i++) {
	VALUE *tag_pair;

	if (TYPE(rb_tag_values[i]) != T_ARRAY ||
	    RARRAY_LEN(rb_tag_values[i]) != 2) {
	    rb_raise(rb_eArgError,
		     "tags should be "
		     "[\"open_tag\", \"close_tag\"] or"
		     "[[\"open_tag1\", \"close_tag1\"], ...]: %s",
		     rb_grn_inspect(rb_tags));
	}
	tag_pair = RARRAY_PTR(rb_tag_values[i]);
	open_tags[i] = StringValuePtr(tag_pair[0]);
	open_tag_lengths[i] = RSTRING_LEN(tag_pair[0]);
	close_tags[i] = StringValuePtr(tag_pair[1]);
	close_tag_lengths[i] = RSTRING_LEN(tag_pair[1]);
    }

    if (RVAL2CBOOL(rb_normalize))
        flags |= GRN_SNIP_NORMALIZE;
    if (RVAL2CBOOL(rb_skip_leading_spaces))
        flags |= GRN_SNIP_SKIP_LEADING_SPACES;

    if (!NIL_P(rb_width))
        width = NUM2UINT(rb_width);

    if (!NIL_P(rb_max_results))
        max_results = NUM2UINT(rb_max_results);

    if (RVAL2CBOOL(rb_html_escape))
        mapping = (grn_snip_mapping *)-1;

    snippet = grn_expr_snip(context, expression, flags, width, max_results,
			    n_tags,
                            (const char **)open_tags, open_tag_lengths,
                            (const char **)close_tags, close_tag_lengths,
                            mapping);
    related_object = rb_ary_new3(2, self, rb_ary_new4(argc, argv));
    rb_grn_context_check(context, related_object);

    return GRNOBJECT2RVAL(Qnil, context, (grn_obj *)snippet, GRN_TRUE);
}

void
rb_grn_init_expression (VALUE mGrn)
{
    rb_cGrnExpression = rb_define_class_under(mGrn, "Expression", rb_cGrnObject);

    rb_define_method(rb_cGrnExpression, "initialize",
                     rb_grn_expression_initialize, -1);

    rb_define_method(rb_cGrnExpression, "define_variable",
                     rb_grn_expression_define_variable, -1);
    rb_define_method(rb_cGrnExpression, "append_object",
                     rb_grn_expression_append_object, -1);
    rb_define_method(rb_cGrnExpression, "append_constant",
                     rb_grn_expression_append_constant, -1);
    rb_define_method(rb_cGrnExpression, "append_operation",
                     rb_grn_expression_append_operation, 2);

    rb_define_method(rb_cGrnExpression, "parse",
                     rb_grn_expression_parse, -1);

    rb_define_method(rb_cGrnExpression, "execute",
                     rb_grn_expression_execute, 0);
    rb_define_method(rb_cGrnExpression, "compile",
                     rb_grn_expression_compile, 0);

    rb_define_method(rb_cGrnExpression, "[]",
                     rb_grn_expression_array_reference, 1);

    rb_define_method(rb_cGrnExpression, "snippet",
                     rb_grn_expression_snippet, -1);

    rb_define_method(rb_cGrnExpression, "inspect",
                     rb_grn_expression_inspect, 0);
}
