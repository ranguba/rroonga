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

#define SELF(object) ((RbGrnColumn *)DATA_PTR(object))

VALUE rb_cGrnColumn;

/*
 * Document-class: Groonga::Column < Groonga::Object
 *
 * テーブルに情報を付加するためのオブジェクト。テーブルに複
 * 数のカラムを定義することによりレコード毎に複数の情報を付
 * 加することができる。
 *
 * カラムには大きく分けて3種類ある。
 * [Groonga::FixSizeColumn]
 *   固定長のデータを格納するカラム。
 * [Groonga::VariableSizeColumn]
 *   可変長のデータを格納するカラム。
 * [Groonga::IndexColumn]
 *   転置インデックスを格納するカラム。全文検索や参照元レコー
 *   ドの検索を行う場合はこのカラムを使用する。
 *
 * 固定長データ用カラム・可変長データ用カラムは1つのデータだ
 * けを格納するか複数のデータを格納するかを選ぶことができる。
 * 1つのデータの場合はスカラ値、複数のデータの場合はスカラー
 * 値を格納するという。
 *
 * カラムは名前を持ち、1つのテーブルでは同じカラム名を持つカ
 * ラムを複数定義することはできない。
 */

grn_obj *
rb_grn_column_from_ruby_object (VALUE object, grn_ctx **context)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(object, rb_cGrnColumn))) {
	rb_raise(rb_eTypeError, "not a groonga column");
    }

    return RVAL2GRNOBJECT(object, context);
}

VALUE
rb_grn_column_to_ruby_object (VALUE klass, grn_ctx *context, grn_obj *column,
			      rb_grn_boolean owner)
{
    return GRNOBJECT2RVAL(klass, context, column, owner);
}

void
rb_grn_column_bind (RbGrnColumn *rb_column,
		    grn_ctx *context, grn_obj *column)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_column);

    rb_column->value = grn_obj_open(context, GRN_BULK, 0,
                                    rb_grn_object->range_id);
}

void
rb_grn_column_finalizer (grn_ctx *context, grn_obj *grn_object,
			 RbGrnColumn *rb_column)
{
    if (context && rb_column->value)
	grn_obj_close(context, rb_column->value);
    rb_column->value = NULL;
}

void
rb_grn_column_deconstruct (RbGrnColumn *rb_column,
			   grn_obj **column,
			   grn_ctx **context,
			   grn_id *domain_id,
			   grn_obj **domain,
			   grn_obj **value,
			   grn_id *range_id,
			   grn_obj **range)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_column);
    rb_grn_object_deconstruct(rb_grn_object, column, context,
			      domain_id, domain,
			      range_id, range);

    if (value)
	*value = rb_column->value;
}


/*
 * call-seq:
 *   column.table -> Groonga::Table
 *
 * カラムが所属するテーブルを返す。
 */
static VALUE
rb_grn_column_get_table (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj *table;

    rb_grn_object_deconstruct((RbGrnObject *)(SELF(self)), &column, &context,
			      NULL, NULL,
			      NULL, NULL);
    table = grn_column_table(context, column);
    rb_grn_context_check(context, self);

    return GRNOBJECT2RVAL(Qnil, context, table, RB_GRN_FALSE);
}

/*
 * call-seq:
 *   column.local_name
 *
 * テーブル名を除いたカラム名を返す。
 *
 *   items = Groonga::Array.create(:name => "<items>")
 *   title = items.define_column("title", "<shorttext>")
 *   title.name # => "<items>.title"
 *   title.local_name # => "title"
 */
static VALUE
rb_grn_column_get_local_name (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    int name_size;
    VALUE rb_name;

    rb_grn_object_deconstruct((RbGrnObject *)(SELF(self)), &column, &context,
			      NULL, NULL,
			      NULL, NULL);
    name_size = grn_column_name(context, column, NULL, 0);
    if (name_size == 0)
	return Qnil;

    rb_name = rb_str_buf_new(name_size);
    rb_str_set_len(rb_name, name_size);
    grn_column_name(context, column, RSTRING_PTR(rb_name), name_size);
    return rb_name;
}

/*
 * call-seq:
 *   column.select(options) {|record| ...} -> Groonga::Hash
 *   column.select(query, options) -> Groonga::Hash
 *   column.select(expression, options) -> Groonga::Hash
 *
 * カラムが所属するテーブルからブロックまたは文字列で指定し
 * た条件にマッチするレコードを返す。返されたテーブルには
 * +expression+という特異メソッドがあり、指定した条件を表し
 * ているGroonga::Expressionを取得できる。
 * Groonga::Expression#snippetを使うことにより、指定した条件
 * 用のスニペットを簡単に生成できる。
 *
 *   results = description_column.select do |column|
 *     column =~ "groonga"
 *   end
 *   snippet = results.expression.snippet([["<em>", "</em>"]])
 *   results.each do |record|
 *     puts "#{record['name']}の説明文の中で「groonga」が含まれる部分"
 *     snippet.execute(record["description"].each do |snippet|
 *       puts "---"
 *       puts "#{snippet}..."
 *       puts "---"
 *     end
 *   end
 *
 * 出力例
 *   Ruby/groongaの説明文の中で「groonga」が含まれる部分
 *   ---
 *   Ruby/<em>groonga</em>は<em>groonga</em>のいわゆるDB-APIの層の...
 *   ---
 *
 * _query_には「[カラム名]:[演算子][値]」という書式で条件を
 * 指定する。演算子は以下の通り。
 *
 * [なし]
 *   [カラム値] == [値]
 * [<tt>!</tt>]
 *   [カラム値] != [値]
 * [<tt><</tt>]
 *   [カラム値] < [値]
 * [<tt>></tt>]
 *   [カラム値] > [値]
 * [<tt><=</tt>]
 *   [カラム値] <= [値]
 * [<tt>>=</tt>]
 *   [カラム値] >= [値]
 * [<tt>@</tt>]
 *   [カラム値]が[値]を含んでいるかどうか
 *
 * 例:
 *   "groonga" # _column_カラムの値が"groonga"のレコードにマッチ
 *   "name:daijiro" # _column_カラムが属しているテーブルの
 *                  # "name"カラムの値が"daijiro"のレコードにマッチ
 *   "description:@groonga" # _column_カラムが属しているテーブルの
 *                          # "description"カラムが
 *                          # "groonga"を含んでいるレコードにマッチ
 *
 * _expression_には既に作成済みのGroonga::Expressionを渡す
 *
 * ブロックで条件を指定する場合は
 * Groonga::ColumnExpressionBuilderを参照。
 *
 * _options_に指定可能な値は以下の通り。
 *
 * [+:operator+]
 *   マッチしたレコードをどのように扱うか。指定可能な値は以
 *   下の通り。省略した場合はGroonga::Operation::OR。
 *
 *   [Groonga::Operation::OR]
 *     マッチしたレコードを追加。すでにレコードが追加され
 *     ている場合は何もしない。
 *   [Groonga::Operation::AND]
 *     マッチしたレコードのスコアを増加。マッチしなかった
 *     レコードを削除。
 *   [Groonga::Operation::BUT]
 *     マッチしたレコードを削除。
 *   [Groonga::Operation::ADJUST]
 *     マッチしたレコードのスコアを増加。
 *
 * [+:result+]
 *   検索結果を格納するテーブル。マッチしたレコードが追加さ
 *   れていく。省略した場合は新しくテーブルを作成して返す。
 *
 * [+:name+]
 *   条件の名前。省略した場合は名前を付けない。
 *
 * [+:parser+]
 *   _query_をパースする時に使用するパーサー。省略した場合は
 *   +:query+。（FIXME: デフォルトを+:column_query+にして、
 *   +:default_mode+も指定できるようにする？）
 */
static VALUE
rb_grn_column_select (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *table, *column, *result, *expression;
    grn_operator operator = GRN_OP_OR;
    VALUE options;
    VALUE rb_query, condition_or_options;
    VALUE rb_name, rb_operator, rb_result, rb_parser = Qnil;
    VALUE builder;
    VALUE rb_expression = Qnil;
    
    rb_query = Qnil;

    rb_scan_args(argc, argv, "02", &condition_or_options, &options);

    rb_grn_column_deconstruct(SELF(self), &column, &context,
			      NULL, NULL,
			      NULL, NULL, NULL);
    table = grn_column_table(context, column);

    if (RVAL2CBOOL(rb_obj_is_kind_of(condition_or_options, rb_cString))) {
        rb_query = condition_or_options;
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(condition_or_options,
                                            rb_cGrnExpression))) {
        rb_expression = condition_or_options;
    } else {
        if (!NIL_P(options))
            rb_raise(rb_eArgError,
		     "should be [query_string, option_hash], "
		     "[expression, opion_hash] "
		     "or [option_hash]: %s",
		     rb_grn_inspect(rb_ary_new4(argc, argv)));
        options = condition_or_options;
    }
    
    rb_grn_scan_options(options,
			"operator", &rb_operator,
			"result", &rb_result,
			"name", &rb_name,
			"parser", &rb_parser,
			NULL);

    if (!NIL_P(rb_operator))
	operator = NUM2INT(rb_operator);

    if (NIL_P(rb_result)) {
	result = grn_table_create(context, NULL, 0, NULL,
				  GRN_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
				  table,
				  0);
	rb_result = GRNTABLE2RVAL(context, result, RB_GRN_TRUE);
    } else {
	result = RVAL2GRNTABLE(rb_result, &context);
    }

    if (NIL_P(rb_expression)) {
      builder = rb_grn_column_expression_builder_new(self, rb_name, rb_query);
      rb_funcall(builder, rb_intern("parser="), 1, rb_parser);
      rb_expression = rb_grn_column_expression_builder_build(builder);
    }
    rb_grn_object_deconstruct(RB_GRN_OBJECT(DATA_PTR(rb_expression)),
                              &expression, NULL,
                              NULL, NULL, NULL, NULL);

    grn_table_select(context, table, expression, result, operator);
    rb_grn_context_check(context, self);

    rb_attr(rb_singleton_class(rb_result),
	    rb_intern("expression"),
	    RB_GRN_TRUE, RB_GRN_FALSE, RB_GRN_FALSE);
    rb_iv_set(rb_result, "@expression", rb_expression);

    return rb_result;
}

void
rb_grn_init_column (VALUE mGrn)
{
    rb_cGrnColumn = rb_define_class_under(mGrn, "Column", rb_cGrnObject);

    rb_define_method(rb_cGrnColumn, "table", rb_grn_column_get_table, 0);
    rb_define_method(rb_cGrnColumn, "local_name",
		     rb_grn_column_get_local_name, 0);

    rb_define_method(rb_cGrnColumn, "select", rb_grn_column_select, -1);

    rb_grn_init_fix_size_column(mGrn);
    rb_grn_init_variable_size_column(mGrn);
    rb_grn_init_index_column(mGrn);
}
