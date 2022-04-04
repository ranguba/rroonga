/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2022  Sutou Kouhei <kou@clear-code.com>
  Copyright (C) 2016  Masafumi Yokoyama <yokoyama@clear-code.com>
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

#include "rb-grn.h"

#include <string.h>

#define SELF(object) ((RbGrnIndexColumn *)RTYPEDDATA_DATA(object))

VALUE rb_cGrnIndexColumn;

/*
 * Document-class: Groonga::IndexColumn < Groonga::Column
 *
 * 転置索引エントリを格納するカラム。このカラムを利用するこ
 * とにより高速な全文検索を実現できる。
 *
 * テーブルにGroonga::IndexColumnを定義する方法は
 * {Groonga::Table#define_index_column} を参照。
 */

void
rb_grn_index_column_finalizer (grn_ctx *context, grn_obj *object,
                               RbGrnIndexColumn *rb_grn_index_column)
{
    if (!context)
        return;

    grn_obj_unlink(context, rb_grn_index_column->id_query);
    grn_obj_unlink(context, rb_grn_index_column->string_query);
    grn_obj_unlink(context, rb_grn_index_column->old_value);
    grn_obj_unlink(context, rb_grn_index_column->set_value);

    rb_grn_column_finalizer(context, object, RB_GRN_COLUMN(rb_grn_index_column));
}

void
rb_grn_index_column_bind (RbGrnIndexColumn *rb_grn_index_column,
                          grn_ctx *context, grn_obj *column)
{
    RbGrnColumn *rb_grn_column;
    RbGrnObject *rb_grn_object;

    rb_grn_column = RB_GRN_COLUMN(rb_grn_index_column);
    rb_grn_column_bind(rb_grn_column, context, column);

    grn_obj_reinit(context,
                   rb_grn_column->value,
                   GRN_DB_UINT32,
                   0);

    rb_grn_object = RB_GRN_OBJECT(rb_grn_index_column);
    rb_grn_index_column->old_value = grn_obj_open(context, GRN_BULK, 0,
                                                  rb_grn_object->range_id);
    rb_grn_index_column->set_value =
        grn_obj_open(context, GRN_VECTOR, 0,
                     rb_grn_object->range->header.domain);

    rb_grn_index_column->id_query = grn_obj_open(context, GRN_BULK, 0,
                                                 rb_grn_object->domain_id);
    rb_grn_index_column->string_query = grn_obj_open(context, GRN_BULK,
                                                     GRN_OBJ_DO_SHALLOW_COPY,
                                                     GRN_DB_SHORT_TEXT);
}

void
rb_grn_index_column_deconstruct (RbGrnIndexColumn *rb_grn_index_column,
                                 grn_obj **column,
                                 grn_ctx **context,
                                 grn_id *domain_id,
                                 grn_obj **domain,
                                 grn_obj **value,
                                 grn_obj **old_value,
                                 grn_obj **set_value,
                                 grn_id *range_id,
                                 grn_obj **range,
                                 grn_obj **id_query,
                                 grn_obj **string_query)
{
    RbGrnObject *rb_grn_object;

    rb_grn_object = RB_GRN_OBJECT(rb_grn_index_column);
    rb_grn_column_deconstruct(RB_GRN_COLUMN(rb_grn_object), column, context,
                              domain_id, domain, value,
                              range_id, range);

    if (old_value)
        *old_value = rb_grn_index_column->old_value;
    if (set_value)
        *set_value = rb_grn_index_column->set_value;
    if (id_query)
        *id_query = rb_grn_index_column->id_query;
    if (string_query)
        *string_query = rb_grn_index_column->string_query;
}

static VALUE
rb_grn_index_column_inspect_content (VALUE self, VALUE inspected)
{
    grn_ctx *context = NULL;
    grn_obj *index_column;
    grn_obj source_ids;
    unsigned int i, n_ids;

    rb_grn_index_column_deconstruct(SELF(self), &index_column, &context,
                                    NULL, NULL,
                                    NULL, NULL, NULL,
                                    NULL, NULL,
                                    NULL, NULL);
    if (!context)
        return inspected;
    if (!index_column)
        return inspected;

    GRN_UINT32_INIT(&source_ids, GRN_OBJ_VECTOR);

    grn_obj_get_info(context, index_column, GRN_INFO_SOURCE, &source_ids);
    n_ids = GRN_BULK_VSIZE(&source_ids) / sizeof(grn_id);

    rb_str_cat2(inspected, ", ");
    rb_str_cat2(inspected, "sources: ");

    rb_str_cat2(inspected, "<");
    for (i = 0; i < n_ids; i++) {
        grn_id source_id;
        grn_obj *source;

        if (i > 0) {
            rb_str_cat2(inspected, ",");
        }

        source_id = GRN_UINT32_VALUE_AT(&source_ids, i);
        source = grn_ctx_at(context, source_id);
        if (source) {
            char source_name[GRN_TABLE_MAX_KEY_SIZE];
            unsigned int source_name_size;

            switch (source->header.type) {
            case GRN_TABLE_HASH_KEY:
            case GRN_TABLE_PAT_KEY:
            case GRN_TABLE_DAT_KEY:
            case GRN_TABLE_NO_KEY:
                rb_str_cat2(inspected, GRN_COLUMN_NAME_KEY);
                break;
            default:
                source_name_size =
                    grn_column_name(context, source,
                                    source_name, GRN_TABLE_MAX_KEY_SIZE);
                rb_str_cat(inspected, source_name, source_name_size);
                break;
            }

            grn_obj_unlink(context, source);
        } else {
            rb_str_catf(inspected, "(nil:%u)", source_id);
        }
    }
    rb_str_cat2(inspected, ">");

    grn_obj_unlink(context, &source_ids);

    return inspected;
}

/*
 * Inspects the index column.
 *
 * @overload inspect
 *   @return [String] the inspected string.
 */
static VALUE
rb_grn_index_column_inspect (VALUE self)
{
    VALUE inspected;

    inspected = rb_str_new_cstr("");
    rb_grn_object_inspect_header(self, inspected);
    rb_grn_object_inspect_content(self, inspected);
    rb_grn_index_column_inspect_content(self, inspected);
    rb_grn_object_inspect_footer(self, inspected);

    return inspected;
}

static VALUE
rb_grn_index_column_array_reference (VALUE self, VALUE rb_token)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj *domain;
    grn_id token_id;
    grn_obj *value;
    VALUE rb_value;

    rb_grn_index_column_deconstruct(SELF(self),
                                    &column,
                                    &context,
                                    NULL,
                                    &domain,
                                    &value,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);

    token_id = RVAL2GRNID(rb_token, context, domain, self);
    GRN_BULK_REWIND(value);
    grn_obj_get_value(context, column, token_id, value);
    rb_grn_context_check(context, self);
    rb_value = GRNVALUE2RVAL(context, value, NULL, self);

    return rb_value;
}

/*
 * Adds a record that has @value@ content to inverted index for fast
 * fulltext serach. Normally, this method is not used
 * explicitly. Inverted index for fulltext search is updated
 * automatically by using @:source@ option of
 * {Groonga::Table#define_index_column}.
 *
 * @example Adds sentences of an article to index
 *   articles = Groonga::Array.create(:name => "Articles")
 *   articles.define_column("title", "ShortText")
 *   articles.define_column("content", "Text")
 *
 *   terms = Groonga::Hash.create(:name => "Terms",
 *                                :key_type => "ShortText",
 *                                :default_tokenizer => "TokenBigram")
 *   content_index = terms.define_index_column("content", articles,
 *                                             :with_position => true,
 *                                             :with_section => true)
 *
 *   content = <<-CONTENT
 *   Groonga is a fast and accurate full text search engine based on
 *   inverted index. One of the characteristics of groonga is that a
 *   newly registered document instantly appears in search
 *   results. Also, groonga allows updates without read locks. These
 *   characteristics result in superior performance on real-time
 *   applications.
 *
 *   Groonga is also a column-oriented database management system
 *   (DBMS). Compared with well-known row-oriented systems, such as
 *   MySQL and PostgreSQL, column-oriented systems are more suited for
 *   aggregate queries. Due to this advantage, groonga can cover
 *   weakness of row-oriented systems.
 *
 *   The basic functions of groonga are provided in a C library. Also,
 *   libraries for using groonga in other languages, such as Ruby, are
 *   provided by related projects. In addition, groonga-based storage
 *   engines are provided for MySQL and PostgreSQL. These libraries
 *   and storage engines allow any application to use groonga. See
 *   usage examples.
 *   CONTENT
 *
 *   groonga = articles.add(:title => "groonga", :content => content)
 *
 *   content.split(/\n{2,}/).each_with_index do |sentence, i|
 *     content_index.add(groonga, sentence, :section => i + 1)
 *   end
 *
 *   content_index.search("engine").each do |record|
 *     p record.key["title"] # -> "groonga"
 *   end
 *
 * @overload add(record, value, options={})
 *   @param [Groonga::Record, Integer] record
 *     The record that has a @value@ as its value. It can be Integer as
 *     record id.
 *   @param [String] value
 *     The value of the @record@.
 *   @param [::Hash] options
 *     The options.
 *   @option options [Integer] :section (1)
 *     The section number. It is one-origin.
 *
 *     You must specify @{:with_section => true}@ in
 *     {Groonga::Table#define_index_column} to use this option.
 *   @return [void]
 *
 * @since 3.0.2
 */
static VALUE
rb_grn_index_column_add (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column, *range;
    grn_rc rc;
    grn_id id;
    unsigned int section;
    grn_obj *new_value;
    VALUE rb_record, rb_value, rb_options, rb_section;

    rb_scan_args(argc, argv, "21", &rb_record, &rb_value, &rb_options);

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    &new_value, NULL, NULL,
                                    NULL, &range,
                                    NULL, NULL);

    id = RVAL2GRNID(rb_record, context, range, self);

    GRN_BULK_REWIND(new_value);
    RVAL2GRNBULK(rb_value, context, new_value);

    rb_grn_scan_options(rb_options,
                        "section", &rb_section,
                        NULL);

    if (NIL_P(rb_section)) {
        section = 1;
    } else {
        section = NUM2UINT(rb_section);
    }

    rc = grn_column_index_update(context, column, id, section, NULL, new_value);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return self;
}

/*
 * Deletes a record that has @value@ content from inverted
 * index. Normally, this method is not used explicitly. Inverted index
 * for fulltext search is updated automatically by using @:source@
 * option of {Groonga::Table#define_index_column}.
 *
 * @example Deletes sentences of an article to index
 *   articles = Groonga::Array.create(:name => "Articles")
 *   articles.define_column("title", "ShortText")
 *   articles.define_column("content", "Text")
 *
 *   terms = Groonga::Hash.create(:name => "Terms",
 *                                :key_type => "ShortText",
 *                                :default_tokenizer => "TokenBigram")
 *   content_index = terms.define_index_column("content", articles,
 *                                             :with_position => true,
 *                                             :with_section => true)
 *
 *   content = <<-CONTENT
 *   Groonga is a fast and accurate full text search engine based on
 *   inverted index. One of the characteristics of groonga is that a
 *   newly registered document instantly appears in search
 *   results. Also, groonga allows updates without read locks. These
 *   characteristics result in superior performance on real-time
 *   applications.
 *
 *   Groonga is also a column-oriented database management system
 *   (DBMS). Compared with well-known row-oriented systems, such as
 *   MySQL and PostgreSQL, column-oriented systems are more suited for
 *   aggregate queries. Due to this advantage, groonga can cover
 *   weakness of row-oriented systems.
 *
 *   The basic functions of groonga are provided in a C library. Also,
 *   libraries for using groonga in other languages, such as Ruby, are
 *   provided by related projects. In addition, groonga-based storage
 *   engines are provided for MySQL and PostgreSQL. These libraries
 *   and storage engines allow any application to use groonga. See
 *   usage examples.
 *   CONTENT
 *
 *   groonga = articles.add(:title => "groonga", :content => content)
 *
 *   content.split(/\n{2,}/).each_with_index do |sentence, i|
 *     content_index.add(groonga, sentence, :section => i + 1)
 *   end
 *
 *   content_index.search("engine").each do |record|
 *     p record.key["title"] # -> "groonga"
 *   end
 *
 *   content.split(/\n{2,}/).each_with_index do |sentence, i|
 *     content_index.delete(groonga, sentence, :section => i + 1)
 *   end
 *
 *   p content_index.search("engine").size # -> 0
 *
 * @overload delete(record, value, options={})
 *   @param [Groonga::Record, Integer] record
 *     The record that has a @value@ as its value. It can be Integer as
 *     record id.
 *   @param [String] value
 *     The value of the @record@.
 *   @param [::Hash] options
 *     The options.
 *   @option options [Integer] :section (1)
 *     The section number. It is one-origin.
 *
 *     You must specify @{:with_section => true}@ in
 *     {Groonga::Table#define_index_column} to use this option.
 *   @return [void]
 *
 * @since 3.0.2
 */
static VALUE
rb_grn_index_column_delete (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column, *range;
    grn_rc rc;
    grn_id id;
    unsigned int section;
    grn_obj *old_value;
    VALUE rb_record, rb_value, rb_options, rb_section;

    rb_scan_args(argc, argv, "21", &rb_record, &rb_value, &rb_options);

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    NULL, &old_value, NULL,
                                    NULL, &range,
                                    NULL, NULL);

    id = RVAL2GRNID(rb_record, context, range, self);

    GRN_BULK_REWIND(old_value);
    RVAL2GRNBULK(rb_value, context, old_value);

    rb_grn_scan_options(rb_options,
                        "section", &rb_section,
                        NULL);

    if (NIL_P(rb_section)) {
        section = 1;
    } else {
        section = NUM2UINT(rb_section);
    }

    rc = grn_column_index_update(context, column, id, section, old_value, NULL);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return self;
}

/*
 * Updates a record that has @new_value@ as new content and
 * @old_value@ as old content in inverted index. Normally, this method
 * is not used explicitly. Inverted index for fulltext search is
 * updated automatically by using @:source@ option of
 * {Groonga::Table#define_index_column}.
 *
 * @example Updates sentences of an article in index
 *   articles = Groonga::Array.create(:name => "Articles")
 *   articles.define_column("title", "ShortText")
 *   articles.define_column("content", "Text")
 *
 *   terms = Groonga::Hash.create(:name => "Terms",
 *                                :key_type => "ShortText",
 *                                :default_tokenizer => "TokenBigram")
 *   content_index = terms.define_index_column("content", articles,
 *                                             :with_position => true,
 *                                             :with_section => true)
 *
 *   old_sentence = <<-SENTENCE
 *   Groonga is a fast and accurate full text search engine based on
 *   inverted index. One of the characteristics of groonga is that a
 *   newly registered document instantly appears in search
 *   results. Also, groonga allows updates without read locks. These
 *   characteristics result in superior performance on real-time
 *   applications.
 *   SENTENCE
 *
 *   new_sentence = <<-SENTENCE
 *   Groonga is also a column-oriented database management system
 *   (DBMS). Compared with well-known row-oriented systems, such as
 *   MySQL and PostgreSQL, column-oriented systems are more suited for
 *   aggregate queries. Due to this advantage, groonga can cover
 *   weakness of row-oriented systems.
 *   SENTENCE
 *
 *   groonga = articles.add(:title => "groonga", :content => old_sentence)
 *
 *   content_index.add(groonga, old_sentence, :section => 1)
 *   p content_index.search("engine").size # -> 1
 *   p content_index.search("MySQL").size  # -> 0
 *
 *   groonga[:content] = new_sentence
 *   content_index.update(groonga, old_sentence, new_sentence, :section => 1)
 *   p content_index.search("engine").size # -> 0
 *   p content_index.search("MySQL").size  # -> 1
 *
 * @overload update(record, old_value, new_value, options={})
 *   @param [Groonga::Record, Integer] record
 *     The record that has a @new_value@ as its new value and
 *     @old_value@ as its old value. It can be Integer as record id.
 *   @param [String] old_value
 *     The old value of the @record@.
 *   @param [String] new_value
 *     The new value of the @record@.
 *   @param [::Hash] options
 *     The options.
 *   @option options [Integer] :section (1)
 *     The section number. It is one-origin.
 *
 *     You must specify @{:with_section => true}@ in
 *     {Groonga::Table#define_index_column} to use this option.
 *   @return [void]
 *
 * @since 3.0.2
 */
static VALUE
rb_grn_index_column_update (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column, *range;
    grn_rc rc;
    grn_id id;
    unsigned int section;
    grn_obj *old_value, *new_value;
    VALUE rb_record, rb_old_value, rb_new_value, rb_options, rb_section;

    rb_scan_args(argc, argv, "31",
                 &rb_record, &rb_old_value, &rb_new_value, &rb_options);

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    &new_value, &old_value, NULL,
                                    NULL, &range,
                                    NULL, NULL);

    id = RVAL2GRNID(rb_record, context, range, self);

    if (NIL_P(rb_old_value)) {
        old_value = NULL;
    } else {
        GRN_BULK_REWIND(old_value);
        RVAL2GRNBULK(rb_old_value, context, old_value);
    }

    if (NIL_P(rb_new_value)) {
        new_value = NULL;
    } else {
        GRN_BULK_REWIND(new_value);
        RVAL2GRNBULK(rb_new_value, context, new_value);
    }

    rb_grn_scan_options(rb_options,
                        "section", &rb_section,
                        NULL);

    if (NIL_P(rb_section)) {
        section = 1;
    } else {
        section = NUM2UINT(rb_section);
    }

    rc = grn_column_index_update(context, column, id, section,
                                 old_value, new_value);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return self;
}

/*
 * インデックス対象となっている {Groonga::Column} の配列を返す。
 *
 * @overload sources
 *   @return [::Array<Groonga::Column>]
 */
static VALUE
rb_grn_index_column_get_sources (VALUE self)
{
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_obj sources;
    grn_id *source_ids;
    VALUE rb_sources;
    int i, n;
    VALUE exception;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL);

    GRN_OBJ_INIT(&sources, GRN_BULK, 0, GRN_ID_NIL);
    grn_obj_get_info(context, column, GRN_INFO_SOURCE, &sources);
    rb_grn_context_check(context, self);

    n = GRN_BULK_VSIZE(&sources) / sizeof(grn_id);
    source_ids = (grn_id *)GRN_BULK_HEAD(&sources);
    rb_sources = rb_ary_new2(n);
    for (i = 0; i < n; i++) {
        grn_obj *source;
        VALUE rb_source;

        source = grn_ctx_at(context, *source_ids);
        exception = rb_grn_context_to_exception(context, self);
        if (!NIL_P(exception)) {
            grn_obj_unlink(context, &sources);
            rb_exc_raise(exception);
        }

        rb_source = GRNOBJECT2RVAL(Qnil, context, source, GRN_FALSE);
        rb_ary_push(rb_sources, rb_source);
        source_ids++;
    }
    grn_obj_unlink(context, &sources);

    return rb_sources;
}

static grn_id
resolve_source_id (grn_ctx *context, grn_obj *column, grn_id range_id,
                   VALUE rb_source)
{
    grn_id source_id;

    if (RVAL2CBOOL(rb_obj_is_kind_of(rb_source, rb_cInteger))) {
        source_id = NUM2UINT(rb_source);
    } else {
        grn_obj *source;
        grn_bool need_source_unlink = GRN_FALSE;

        if (TYPE(rb_source) == T_STRING) {
            grn_obj *table;
            const char *name;
            const char *dot_point;
            int length;

            table = grn_ctx_at(context, grn_obj_get_range(context, column));
            name = StringValueCStr(rb_source);
            length = RSTRING_LEN(rb_source);
            dot_point = strstr(name, ".");
            if (dot_point) {
                char table_name[4096];
                int table_name_length;

                table_name_length = grn_obj_name(context, table,
                                                 table_name, sizeof(table_name));
                table_name[table_name_length] = '\0';
                if (strncmp(table_name, name, dot_point - name) != 0) {
                    rb_raise(rb_eArgError,
                             "wrong table's column: <%s>: "
                             "expected table: <%s>",
                             name, table_name);
                }
                length -= (dot_point - name) + 1;
                name = dot_point + 1;
            }
            source = grn_obj_column(context, table, name, length);
            need_source_unlink = GRN_TRUE;
        } else {
            source = RVAL2GRNOBJECT(rb_source, &context);
        }
        rb_grn_context_check(context, rb_source);
        if (!source) {
            rb_raise(rb_eArgError, "couldn't find source: <%s>",
                     rb_grn_inspect(rb_source));
        }
        if (source->header.type == GRN_ACCESSOR) {
            char name[256];
            int length;
            length = grn_column_name(context, source, name, sizeof(name));
            name[length] = '\0';
            if (strcmp(name, "_key") != 0) {
                rb_raise(rb_eArgError,
                         "source accessor must be '_key': <%s>", name);
            }
            source_id = range_id;
        } else {
            source_id = grn_obj_id(context, source);
        }
        if (need_source_unlink) {
            grn_obj_unlink(context, source);
        }
    }

    return source_id;
}

/*
 * インデックス対象となる複数のカラムを配列で設定する。
 *
 * @overload sources=(columns)
 *   @param [::Array<Groonga::Column>] columns インデックス対象となるカラムの配列
 */
static VALUE
rb_grn_index_column_set_sources (VALUE self, VALUE rb_sources)
{
    VALUE exception;
    grn_ctx *context = NULL;
    grn_obj *column;
    int i, n;
    VALUE *rb_source_values;
    grn_id range_id;
    grn_id *sources;
    grn_rc rc;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    NULL, NULL, NULL,
                                    &range_id, NULL,
                                    NULL, NULL);

    n = RARRAY_LEN(rb_sources);
    rb_source_values = RARRAY_PTR(rb_sources);
    sources = ALLOCA_N(grn_id, n);
    for (i = 0; i < n; i++) {
        sources[i] = resolve_source_id(context, column, range_id,
                                       rb_source_values[i]);
    }

    {
        grn_obj bulk_sources;
        GRN_OBJ_INIT(&bulk_sources, GRN_BULK, 0, GRN_ID_NIL);
        GRN_TEXT_SET(context, &bulk_sources, sources, n * sizeof(grn_id));
        rc = grn_obj_set_info(context, column, GRN_INFO_SOURCE, &bulk_sources);
        exception = rb_grn_context_to_exception(context, self);
        grn_obj_unlink(context, &bulk_sources);
    }

    if (!NIL_P(exception))
        rb_exc_raise(exception);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

/*
 * インデックス対象となるカラムを設定する。
 *
 * @overload source=(column)
 *   @param [Groonga::Column] column インデックス対象とするカラム
 */
static VALUE
rb_grn_index_column_set_source (VALUE self, VALUE rb_source)
{
    if (!RVAL2CBOOL(rb_obj_is_kind_of(rb_source, rb_cArray)))
        rb_source = rb_ary_new_from_args(1, rb_source);

    return rb_grn_index_column_set_sources(self, rb_source);
}

/*
 * _object_ から _query_ に対応するオブジェクトを検索し、見つかっ
 * たオブジェクトのIDがキーになっている {Groonga::Hash} を返す。
 *
 * @overload search(query, options={})
 *   @param [::Hash] options The name and value
 *     pairs. Omitted names are initialized as the default value
 *   @option options [Groonga::Hash] :result
 *     結果を格納するGroonga::Hash。指定しない場合は新しく
 *     Groonga::Hashを生成し、それに結果を格納して返す。
 *   @option options :operator
 *     以下のどれかの値を指定する。 +nil+ , @"or"@ , @"||"@ ,
 *     @"and"@ , @"+"@ , @"&&"@ , @"but"@ ,
 *     @"not"@ , @"-"@ , @"adjust"@ , @">"@ 。
 *     それぞれ以下のようになる。（FIXME: 「以下」）
 *   @option options :exact
 *     +true+ を指定すると完全一致で検索する
 *   @option options :longest_common_prefix
 *     +true+ を指定すると _query_ と同じ接頭辞をもつエントリのう
 *     ち、もっとも長いエントリを検索する
 *   @option options :suffix
 *     +true+ を指定すると _query_ が後方一致するエントリを検索す
 *     る
 *   @option options :prefix
 *     +true+ を指定すると _query_ が前方一致するレコードを検索す
 *     る
 *   @option options :near
 *     +true+ を指定すると _query_ に指定した複数の語が近傍に含ま
 *     れるレコードを検索する
 *   @return [Groonga::Hash]
 */
static VALUE
rb_grn_index_column_search (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *column;
    grn_obj *range;
    grn_obj *query = NULL, *id_query = NULL, *string_query = NULL;
    grn_obj *result;
    grn_operator operator;
    grn_search_optarg options;
    grn_rc rc;
    VALUE rb_query, rb_options, rb_result, rb_operator, rb_mode, rb_weight;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    NULL, NULL, NULL, NULL, &range,
                                    &id_query, &string_query);

    rb_scan_args(argc, argv, "11", &rb_query, &rb_options);

    if (CBOOL2RVAL(rb_obj_is_kind_of(rb_query, rb_cInteger))) {
        grn_id id;
        id = NUM2UINT(rb_query);
        GRN_TEXT_SET(context, id_query, &id, sizeof(grn_id));
        query = id_query;
    } else {
        const char *_query;
        _query = StringValuePtr(rb_query);
        GRN_TEXT_SET(context, string_query, _query, RSTRING_LEN(rb_query));
        query = string_query;
    }

    rb_grn_scan_options(rb_options,
                        "result", &rb_result,
                        "operator", &rb_operator,
                        "mode", &rb_mode,
                        "weight", &rb_weight,
                        NULL);

    if (NIL_P(rb_result)) {
        result = grn_table_create(context, NULL, 0, NULL,
                                  GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
                                  range, 0);
        rb_grn_context_check(context, self);
        rb_result = GRNOBJECT2RVAL(Qnil, context, result, GRN_TRUE);
    } else {
        result = RVAL2GRNOBJECT(rb_result, &context);
    }

    operator = RVAL2GRNOPERATOR(rb_operator);

    memset(&options, 0, sizeof(grn_search_optarg));
    if (NIL_P(rb_mode)) {
        options.mode = GRN_OP_EXACT;
    } else {
        options.mode = RVAL2GRNOPERATOR(rb_mode);
    }
    options.similarity_threshold = 0;
    options.max_interval = 0;
    options.weight_vector = NULL;
    if (NIL_P(rb_weight)) {
        options.vector_size = 1;
    } else {
        options.vector_size = NUM2UINT(rb_weight);
    }
    options.proc = NULL;
    options.max_size = 0;

    rc = grn_obj_search(context, column, query, result, operator, &options);
    rb_grn_rc_check(rc, self);

    return rb_result;
}

/*
 * _column_ が段落情報も格納する場合は +true+ を返します。
 *
 * @overload with_section?
 */
static VALUE
rb_grn_index_column_with_section_p (VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL(flags & GRN_OBJ_WITH_SECTION);
}

/*
 * _column_ が位置情報も格納する場合は +true+ を返します。
 *
 * @overload with_position?
 */
static VALUE
rb_grn_index_column_with_position_p (VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL(flags & GRN_OBJ_WITH_POSITION);
}

/*
 * Checks whether the index column is small or not.
 *
 * @overload small?
 *   @return [Boolean] `true` if the index column is small,
 *     `false` otherwise.
 *
 * @since 6.1.0
 */
static VALUE
rb_grn_index_column_small_p (VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL(flags & GRN_OBJ_INDEX_SMALL);
}

/*
 * Checks whether the index column is medium or not.
 *
 * @overload medium?
 *   @return [Boolean] `true` if the index column is medium,
 *     `false` otherwise.
 *
 * @since 6.1.0
 */
static VALUE
rb_grn_index_column_medium_p (VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL(flags & GRN_OBJ_INDEX_MEDIUM);
}

/*
 * Checks whether the index column is large or not.
 *
 * @overload large?
 *   @return [Boolean] `true` if the index column size is large,
 *     `false` otherwise.
 *
 * @since 9.0.4
 */
static VALUE
rb_grn_index_column_large_p (VALUE self)
{
    grn_obj *column;
    grn_ctx *context;
    grn_column_flags flags;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL);

    flags = grn_column_get_flags(context, column);
    return CBOOL2RVAL(flags & GRN_OBJ_INDEX_LARGE);
}

static VALUE
call_close (VALUE object)
{
    return rb_funcall(object, rb_intern("close"), 0);
}

/*
 * Opens cursor to iterate posting in the index column.
 *
 * @example
 *   # TODO
 *
 * @overload open_cursor(table_cursor, options={})
 *   @param [TableCursor] The table cursor for table of the index column.
 *   @param [::Hash] options
 *   @option options [Boolean] :with_section (nil)
 *      Includes section info the posting. It is enabled by default if
 *      the index column is created with @:with_section@ flag.
 *   @option options [Boolean] :with_weight (nil)
 *      Includes weight info the posting. It is enabled by default if
 *      the index column is created with @:with_weight@ flag.
 *   @option options [Boolean] :with_position (nil)
 *      Includes position info the posting. It is enabled by default if
 *      the index column is created with @:with_position@ flag.
 */
static VALUE
rb_grn_index_column_open_cursor (int argc, VALUE *argv, VALUE self)
{
    grn_ctx          *context;
    grn_obj          *column;
    grn_column_flags  column_flags;
    grn_obj          *domain_object;
    grn_obj          *range_object;
    grn_table_cursor *table_cursor = NULL;
    grn_id            token_id = GRN_ID_NIL;
    grn_id            rid_min = GRN_ID_NIL;
    grn_id            rid_max = GRN_ID_MAX;
    int               flags   = 0;
    VALUE             rb_table_cursor_or_token;
    VALUE             options;
    VALUE             rb_with_section, rb_with_weight, rb_with_position;
    VALUE             rb_table;
    VALUE             rb_lexicon;
    VALUE             rb_cursor;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, &domain_object,
                                    NULL, NULL, NULL,
                                    NULL, &range_object,
                                    NULL, NULL);

    rb_scan_args(argc, argv, "11", &rb_table_cursor_or_token, &options);
    rb_grn_scan_options(options,
                        "with_section", &rb_with_section,
                        "with_weight", &rb_with_weight,
                        "with_position", &rb_with_position,
                        NULL);

    rb_table     = GRNOBJECT2RVAL(Qnil, context, range_object, GRN_FALSE);
    rb_lexicon   = rb_funcall(self, rb_intern("table"), 0);
    if (CBOOL2RVAL(rb_obj_is_kind_of(rb_table_cursor_or_token,
                                     rb_cGrnTableCursor))) {
        VALUE rb_table_cursor = rb_table_cursor_or_token;
        table_cursor = RVAL2GRNTABLECURSOR(rb_table_cursor, NULL);
    } else {
        VALUE rb_token = rb_table_cursor_or_token;
        token_id = RVAL2GRNID(rb_token, context, domain_object, self);
    }

    column_flags = grn_column_get_flags(context, column);

    if (NIL_P(rb_with_section)) {
        flags |= column_flags & GRN_OBJ_WITH_SECTION;
    } else if (RVAL2CBOOL(rb_with_section)) {
        flags |= GRN_OBJ_WITH_SECTION;
    }

    if (NIL_P(rb_with_weight)) {
        flags |= column_flags & GRN_OBJ_WITH_WEIGHT;
    } else if (RVAL2CBOOL(rb_with_weight)) {
        flags |= GRN_OBJ_WITH_WEIGHT;
    }

    if (NIL_P(rb_with_position)) {
        flags |= column_flags & GRN_OBJ_WITH_POSITION;
    } else if (RVAL2CBOOL(rb_with_position)) {
        flags |= GRN_OBJ_WITH_POSITION;
    }

    if (table_cursor) {
        grn_obj *index_cursor;
        index_cursor = grn_index_cursor_open(context, table_cursor,
                                             column, rid_min, rid_max, flags);
        rb_cursor = GRNINDEXCURSOR2RVAL(context,
                                        index_cursor,
                                        rb_table,
                                        rb_lexicon);
    } else {
        grn_ii *ii = (grn_ii *)column;
        grn_ii_cursor *ii_cursor;
        ii_cursor = grn_ii_cursor_open(context,
                                       ii,
                                       token_id,
                                       rid_min,
                                       rid_max,
                                       grn_ii_get_n_elements(context, ii),
                                       flags);
        rb_cursor = rb_grn_inverted_index_cursor_to_ruby_object(context,
                                                                ii_cursor,
                                                                token_id,
                                                                flags,
                                                                rb_table,
                                                                rb_lexicon);
    }

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_cursor, call_close, rb_cursor);
    else
        return rb_cursor;
}

/*
 * Estimates the number of documents found by the given token ID,
 * query or lexicon cursor.
 *
 * @example Token ID style
 *    # Define schema
 *    Groonga::Schema.define do |schema|
 *      schema.create_table("Articles") do |table|
 *        table.text("content")
 *      end
 *
 *      schema.create_table("Terms",
 *                          :type => :hash,
 *                          :key_type => "ShortText",
 *                          :default_tokenizer => "TokenBigram",
 *                          :normalizer => "NormalizerAuto") do |table|
 *        table.index("Articles.content",
 *                    :name => "articles_content",
 *                    :with_position => true,
 *                    :with_section => true)
 *      end
 *    end
 *    articles = Groonga["Articles"]
 *    terms = Groonga["Terms"]
 *    index = Groonga["Terms.articles_content"]
 *
 *    # Add data
 *    articles.add(:content => "Groonga is fast")
 *    articles.add(:content => "Rroonga is fast")
 *    articles.add(:content => "Mroonga is fast")
 *
 *    # Estimate the number of documents found by token ID
 *    p @index.estimate_size(@terms["fast"].id)    # => 7
 *    p @index.estimate_size(@terms["Groonga"].id) # => 1
 *
 * @example Token record style
 *    # Define schema
 *    Groonga::Schema.define do |schema|
 *      schema.create_table("Articles") do |table|
 *        table.text("content")
 *      end
 *
 *      schema.create_table("Terms",
 *                          :type => :hash,
 *                          :key_type => "ShortText",
 *                          :default_tokenizer => "TokenBigram",
 *                          :normalizer => "NormalizerAuto") do |table|
 *        table.index("Articles.content",
 *                    :name => "articles_content",
 *                    :with_position => true,
 *                    :with_section => true)
 *      end
 *    end
 *    articles = Groonga["Articles"]
 *    terms = Groonga["Terms"]
 *    index = Groonga["Terms.articles_content"]
 *
 *    # Add data
 *    articles.add(:content => "Groonga is fast")
 *    articles.add(:content => "Rroonga is fast")
 *    articles.add(:content => "Mroonga is fast")
 *
 *    # Estimate the number of documents found by token record
 *    p @index.estimate_size(@terms["fast"])    # => 7
 *    p @index.estimate_size(@terms["Groonga"]) # => 1
 *
 * @example Query style
 *    # Define schema
 *    Groonga::Schema.define do |schema|
 *      schema.create_table("Articles") do |table|
 *        table.text("content")
 *      end
 *
 *      schema.create_table("Terms",
 *                          :type => :hash,
 *                          :key_type => "ShortText",
 *                          :default_tokenizer => "TokenBigramSplitSymbolAlpha",
 *                          :normalizer => "NormalizerAuto") do |table|
 *        table.index("Articles.content",
 *                    :name => "articles_content",
 *                    :with_position => true,
 *                    :with_section => true)
 *      end
 *    end
 *    articles = Groonga["Articles"]
 *    index = Groonga["Terms.articles_content"]
 *
 *    # Add data
 *    articles.add(:content => "Groonga is fast")
 *    articles.add(:content => "Rroonga is fast")
 *    articles.add(:content => "Mroonga is fast")
 *
 *    # Estimate the number of documents found by query
 *    p @index.estimate_size("roonga") # => 6
 *
 * @example Lexicon cursor style
 *    # Define schema
 *    Groonga::Schema.define do |schema|
 *      schema.create_table("Memos",
 *                          :type => :hash,
 *                          :key_type => "ShortText") do |table|
 *        table.short_text("tags", :type => :vector)
 *      end
 *
 *      schema.create_table("Tags",
 *                          :type => :patricia_trie,
 *                          :key_type => "ShortText") do |table|
 *        table.index("Memos.tags",
 *                    :name => "memos_tags")
 *      end
 *    end
 *    memos = Groonga["Memos"]
 *    tags = Groonga["Tags"]
 *    index = Groonga["Tags.memos_tags"]
 *
 *    # Add data
 *    memos.add(:tags => ["Groonga"])
 *    memos.add(:tags => ["Rroonga", "Ruby"])
 *    memos.add(:tags => ["grndump", "Rroonga"])
 *
 *    # Estimate the number of documents found by lexicon cursor
 *    # Iterates tags that start with "R".
 *    tags.open_prefix_cursor("R") do |cursor|
 *      # The cursor iterates "Rroonga" and "Ruby".
 *      p index.estimate_size(cursor) # => 6
 *    end
 *
 * @overload estimate_size(token_id)
 *   @param token_id [Integer, Record] The token ID to be estimated.
 *   @return [Integer] The estimated number of documents found by the
 *     given token ID.
 *
 * @overload estimate_size(query, options={})
 *   @param query [String] The query to be estimated.
 *   @param options [::Hash] The options.
 *   @option options [Groonga::Operator, String, Symbol] :mode
 *     (Groonga::Operator::EXACT)
 *
 *     The operation mode for search. It must be one of the followings:
 *
 *       * `Groonga::Operator::EXACT`, `"exact"`, `:exact`
 *       * `Groonga::Operator::NEAR`, `"near"`, `:near`
 *       * `Groonga::Operator::NEAR2`, `"near2"`, `:near2`
 *       * `Groonga::Operator::SIMILAR`, `"similar"`, `:similar`
 *       * `Groonga::Operator::REGEXP`, `"regexp"`, `:regexp`
 *
 *   @return [Integer] The estimated number of documents found by the
 *     given query.
 *
 *   @since 5.0.1
 *
 * @overload estimate_size(lexicon_cursor)
 *   @param lexicon_cursor [Groonga::TableCursor] The cursor for lexicon.
 *   @return [Integer] The estimated number of documents found by term IDS
 *     in the given lexicon cursor.
 *
 *   @since 5.0.1
 *
 * @since 4.0.7
 */
static VALUE
rb_grn_index_column_estimate_size (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *column;
    grn_obj *domain_object;
    unsigned int size;
    VALUE rb_target;
    VALUE rb_options;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, &domain_object,
                                    NULL, NULL, NULL,
                                    NULL, NULL,
                                    NULL, NULL);

    rb_scan_args(argc, argv, "11", &rb_target, &rb_options);

    if (TYPE(rb_target) == T_STRING) {
        const char *query;
        unsigned int query_length;
        grn_search_optarg options;
        VALUE rb_mode;

        query = StringValueCStr(rb_target);
        query_length = RSTRING_LEN(rb_target);

        rb_grn_scan_options(rb_options,
                            "mode", &rb_mode,
                            NULL);

        memset(&options, 0, sizeof(grn_search_optarg));
        if (NIL_P(rb_mode)) {
            options.mode = GRN_OP_EXACT;
        } else {
            options.mode = RVAL2GRNOPERATOR(rb_mode);
        }
        switch (options.mode) {
        case GRN_OP_EXACT:
        case GRN_OP_NEAR:
        case GRN_OP_NEAR2:
        case GRN_OP_SIMILAR:
        case GRN_OP_REGEXP:
            /* valid */
            break;
        default:
            rb_raise(rb_eArgError,
                     ":mode must be one of "
                     "nil, :exact, :near, :near2, :similar or :regexp: <%s>",
                     rb_grn_inspect(rb_mode));
            break;
        }

        size = grn_ii_estimate_size_for_query(context, (grn_ii *)column,
                                              query, query_length, &options);
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(rb_target, rb_cGrnTableCursor))) {
        grn_table_cursor *lexicon_cursor;

        lexicon_cursor = RVAL2GRNTABLECURSOR(rb_target, &context);
        size = grn_ii_estimate_size_for_lexicon_cursor(context,
                                                       (grn_ii *)column,
                                                       lexicon_cursor);
    } else {
        grn_id token_id;
        token_id = RVAL2GRNID(rb_target, context, domain_object, self);
        size = grn_ii_estimate_size(context, (grn_ii *)column, token_id);
    }

    return UINT2NUM(size);
}

/*
 * Recreates the index column.
 *
 * This method is useful when you have a broken index column.
 *
 * You can use {Groonga::Database#reindex} to recreate all index
 * columns in a database.
 *
 * You can use {Groonga::TableKeySupport#reindex} to recreate all
 * index columns in a table.
 *
 * You can use {Groonga::FixSizeColumn#reindex} or
 * {Groonga::VariableSizeColumn#reindex} to recreate all index
 * columns. They use index columns of the data column as
 * reindex target index columns.
 *
 * @example How to recreate the index column.
 *   Groonga::Schema.define do |schema|
 *     schema.create_table("Memos") do |table|
 *       table.short_text("title")
 *       table.text("content")
 *     end
 *
 *     schema.create_table("BigramTerms",
 *                         :type => :patricia_trie,
 *                         :key_type => :short_text,
 *                         :normalizer => "NormalizerAuto",
 *                         :default_tokenizer => "TokenBigram") do |table|
 *       table.index("Memos.title")
 *       table.index("Memos.content")
 *     end
 *
 *     schema.create_table("MeCabTerms",
 *                         :type => :patricia_trie,
 *                         :key_type => :short_text,
 *                         :normalizer => "NormalizerAuto",
 *                         :default_tokenizer => "TokenMecab") do |table|
 *       table.index("Memos.title")
 *       table.index("Memos.content")
 *     end
 *   end
 *
 *   Groonga["MeCabTerms.Memos_content"].reindex
 *   # They aren't called:
 *   #   Groonga["BigramTerms.Memos_title"].reindex
 *   #   Groonga["BigramTerms.Memos_content"].reindex
 *   #   Groonga["MeCabTerms.Memos_title"].reindex
 *
 * @overload reindex
 *   @return [void]
 *
 * @see Groonga::Database#reindex
 * @see Groonga::TableKeySupport#reindex
 * @see Groonga::FixSizeColumn#reindex
 * @see Groonga::VariableSizeColumn#reindex
 *
 * @since 5.1.1
 */
static VALUE
rb_grn_index_column_reindex (VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *column;

    rb_grn_index_column_deconstruct(SELF(self), &column, &context,
                                    NULL, NULL,
                                    NULL, NULL, NULL,
                                    NULL, NULL,
                                    NULL, NULL);

    rc = grn_obj_reindex(context, column);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

void
rb_grn_init_index_column (VALUE mGrn)
{
    rb_cGrnIndexColumn =
        rb_define_class_under(mGrn, "IndexColumn", rb_cGrnColumn);

    rb_define_method(rb_cGrnIndexColumn, "inspect",
                     rb_grn_index_column_inspect, 0);

    rb_define_method(rb_cGrnIndexColumn, "[]",
                     rb_grn_index_column_array_reference, 1);
    rb_undef_method(rb_cGrnIndexColumn, "[]=");

    rb_define_method(rb_cGrnIndexColumn, "add",
                     rb_grn_index_column_add, -1);
    rb_define_method(rb_cGrnIndexColumn, "delete",
                     rb_grn_index_column_delete, -1);
    rb_define_method(rb_cGrnIndexColumn, "update",
                     rb_grn_index_column_update, -1);

    rb_define_method(rb_cGrnIndexColumn, "sources",
                     rb_grn_index_column_get_sources, 0);
    rb_define_method(rb_cGrnIndexColumn, "sources=",
                     rb_grn_index_column_set_sources, 1);
    rb_define_method(rb_cGrnIndexColumn, "source=",
                     rb_grn_index_column_set_source, 1);

    rb_define_method(rb_cGrnIndexColumn, "search",
                     rb_grn_index_column_search, -1);

    rb_define_method(rb_cGrnIndexColumn, "with_section?",
                     rb_grn_index_column_with_section_p, 0);
    rb_define_method(rb_cGrnIndexColumn, "with_position?",
                     rb_grn_index_column_with_position_p, 0);
    rb_define_method(rb_cGrnIndexColumn, "small?",
                     rb_grn_index_column_small_p, 0);
    rb_define_method(rb_cGrnIndexColumn, "medium?",
                     rb_grn_index_column_medium_p, 0);
    rb_define_method(rb_cGrnIndexColumn, "large?",
                     rb_grn_index_column_large_p, 0);

    rb_define_method(rb_cGrnIndexColumn, "open_cursor",
                     rb_grn_index_column_open_cursor, -1);

    rb_define_method(rb_cGrnIndexColumn, "estimate_size",
                     rb_grn_index_column_estimate_size, -1);

    rb_define_method(rb_cGrnIndexColumn, "reindex",
                     rb_grn_index_column_reindex, 0);
}
