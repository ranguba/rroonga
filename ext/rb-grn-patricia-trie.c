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

#define SELF(object) ((RbGrnTableKeySupport *)DATA_PTR(object))

VALUE rb_cGrnPatriciaTrie;

static VALUE
rb_grn_patricia_trie_s_create (int argc, VALUE *argv, VALUE self)
{
    grn_ctx *context;
    grn_obj *key_type = NULL, *value_type = NULL, *table;
    const char *name = NULL, *path = NULL;
    unsigned name_size = 0;
    grn_obj_flags flags = GRN_TABLE_PAT_KEY;
    VALUE rb_table;
    VALUE options, rb_context, rb_name, rb_path, rb_persistent;
    VALUE rb_key_normalize, rb_key_with_sis, rb_key_type;
    VALUE rb_value_type;
    VALUE rb_default_tokenizer, rb_sub_records;

    rb_scan_args(argc, argv, "01", &options);

    rb_grn_scan_options(options,
			"context", &rb_context,
			"name", &rb_name,
                        "path", &rb_path,
			"persistent", &rb_persistent,
			"key_normalize", &rb_key_normalize,
			"key_with_sis", &rb_key_with_sis,
			"key_type", &rb_key_type,
			"value_type", &rb_value_type,
			"default_tokenizer", &rb_default_tokenizer,
			"sub_records", &rb_sub_records,
			NULL);

    context = rb_grn_context_ensure(&rb_context);

    if (!NIL_P(rb_name)) {
        name = StringValuePtr(rb_name);
	name_size = RSTRING_LEN(rb_name);
	flags |= GRN_OBJ_PERSISTENT;
    }

    if (!NIL_P(rb_path)) {
        path = StringValueCStr(rb_path);
	flags |= GRN_OBJ_PERSISTENT;
    }

    if (RVAL2CBOOL(rb_persistent))
	flags |= GRN_OBJ_PERSISTENT;

    if (RVAL2CBOOL(rb_key_normalize))
	flags |= GRN_OBJ_KEY_NORMALIZE;

    if (RVAL2CBOOL(rb_key_with_sis))
	flags |= GRN_OBJ_KEY_WITH_SIS;

    if (NIL_P(rb_key_type)) {
	flags |= GRN_OBJ_KEY_VAR_SIZE;
    } else {
	key_type = RVAL2GRNOBJECT(rb_key_type, &context);
    }

    if (!NIL_P(rb_value_type))
	value_type = RVAL2GRNOBJECT(rb_value_type, &context);

    if (RVAL2CBOOL(rb_sub_records))
	flags |= GRN_OBJ_WITH_SUBREC;

    table = grn_table_create(context, name, name_size, path,
			     flags, key_type, value_type);
    if (!table)
	rb_grn_context_check(context, rb_ary_new4(argc, argv));
    rb_table = rb_grn_object_alloc(self);
    rb_grn_object_assign(Qnil, rb_table, rb_context, context, table);
    rb_grn_context_check(context, rb_table);

    if (!NIL_P(rb_default_tokenizer))
	rb_funcall(rb_table, rb_intern("default_tokenizer="), 1,
		   rb_default_tokenizer);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, rb_table, rb_grn_object_close, rb_table);
    else
        return rb_table;
}

static VALUE
rb_grn_patricia_trie_search (int argc, VALUE *argv, VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *table;
    grn_id domain_id;
    grn_obj *key, *domain, *result;
    grn_operator operator;
    grn_search_optarg search_options;
    rb_grn_boolean search_options_is_set = RB_GRN_TRUE;
    VALUE rb_key, options, rb_result, rb_operator, rb_type;

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 &key, &domain_id, &domain,
					 NULL, NULL, NULL);

    rb_scan_args(argc, argv, "11", &rb_key, &options);

    RVAL2GRNKEY(rb_key, context, key, domain_id, domain, self);

    rb_grn_scan_options(options,
			"result", &rb_result,
			"operator", &rb_operator,
			"type", &rb_type,
			NULL);

    if (NIL_P(rb_result)) {
	result = grn_table_create(context, NULL, 0, NULL,
				  GRN_OBJ_TABLE_HASH_KEY | GRN_OBJ_WITH_SUBREC,
				  table, 0);
	rb_grn_context_check(context, self);
	rb_result = GRNOBJECT2RVAL(Qnil, context, result, RB_GRN_TRUE);
    } else {
	result = RVAL2GRNOBJECT(rb_result, &context);
    }

    operator = RVAL2GRNOPERATOR(rb_operator);

    rc = grn_obj_search(context, table, key,
			result, operator,
			search_options_is_set ? &search_options : NULL);
    rb_grn_rc_check(rc, self);

    return rb_result;
}

static VALUE
rb_grn_patricia_trie_scan (VALUE self, VALUE rb_string)
{
    grn_ctx *context;
    grn_obj *table;
    VALUE rb_result = Qnil;
    grn_pat_scan_hit hits[1024];
    const char *string;
    long string_length;
    rb_grn_boolean block_given;

    string = StringValuePtr(rb_string);
    string_length = RSTRING_LEN(rb_string);

    rb_grn_table_key_support_deconstruct(SELF(self), &table, &context,
					 NULL, NULL, NULL,
					 NULL, NULL, NULL);

    block_given = rb_block_given_p();
    if (!block_given)
	rb_result = rb_ary_new();

    while (string_length > 0) {
	const char *rest;
	int i, n_hits;
	long previous_offset = 0;

	n_hits = grn_pat_scan(context, (grn_pat *)table,
			      string, string_length,
			      hits, sizeof(hits) / sizeof(*hits),
			      &rest);
	for (i = 0; i < n_hits; i++) {
	    VALUE record, term, matched_info;

	    if (hits[i].offset < previous_offset)
		continue;

	    record = rb_grn_record_new(self, hits[i].id, Qnil);
	    term = rb_str_new(string + hits[i].offset,
			      hits[i].length);
	    matched_info = rb_ary_new3(4,
				       term,
				       record,
				       UINT2NUM(hits[i].offset),
				       UINT2NUM(hits[i].length));
	    if (block_given) {
		rb_yield(matched_info);
	    } else {
		rb_ary_push(rb_result, matched_info);
	    }
	    previous_offset = hits[i].offset;
	}
	string_length -= rest - string;
	string = rest;
    }

    return rb_result;
}

void
rb_grn_init_patricia_trie (VALUE mGrn)
{
    rb_cGrnPatriciaTrie =
	rb_define_class_under(mGrn, "PatriciaTrie", rb_cGrnTable);

    rb_include_module(rb_cGrnPatriciaTrie, rb_mGrnTableKeySupport);
    rb_define_singleton_method(rb_cGrnPatriciaTrie, "create",
			       rb_grn_patricia_trie_s_create, -1);

    rb_define_method(rb_cGrnPatriciaTrie, "search",
		     rb_grn_patricia_trie_search, -1);
    rb_define_method(rb_cGrnPatriciaTrie, "scan",
		     rb_grn_patricia_trie_scan, 1);
}
