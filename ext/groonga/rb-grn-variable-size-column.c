/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2009-2022  Sutou Kouhei <kou@clear-code.com>
  Copyright (C) 2014-2016  Masafumi Yokoyama <yokoyama@clear-code.com>

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

#define SELF(object) ((RbGrnVariableSizeColumn *)RTYPEDDATA_DATA(object))

VALUE rb_cGrnVariableSizeColumn;

void
rb_grn_variable_size_column_bind (RbGrnVariableSizeColumn *rb_column,
                                  grn_ctx *context, grn_obj *column)
{
    RbGrnObject *rb_grn_object;
    grn_column_flags flags;
    grn_column_flags column_type;
    unsigned char value_type;

    rb_grn_object = RB_GRN_OBJECT(rb_column);
    rb_grn_column_bind(RB_GRN_COLUMN(rb_column), context, column);

    rb_column->element_value = NULL;
    flags = grn_column_get_flags(context, column);
    column_type = (flags & GRN_OBJ_COLUMN_TYPE_MASK);
    if (column_type != GRN_OBJ_COLUMN_VECTOR) {
        return;
    }

    switch (rb_grn_object->range->header.type) {
    case GRN_TABLE_HASH_KEY:
    case GRN_TABLE_PAT_KEY:
    case GRN_TABLE_DAT_KEY:
    case GRN_TABLE_NO_KEY:
        value_type = GRN_UVECTOR;
        break;
    default:
        value_type = GRN_VECTOR;
        break;
    }
    if (flags & GRN_OBJ_WITH_WEIGHT) {
        rb_column->element_value = grn_obj_open(context, value_type, 0,
                                                rb_grn_object->range_id);
    }
}

void
rb_grn_variable_size_column_finalizer (grn_ctx *context, grn_obj *grn_object,
                                       RbGrnVariableSizeColumn *rb_column)
{
    rb_grn_column_finalizer(context, grn_object,
                            RB_GRN_COLUMN(rb_column));
    if (context && rb_column->element_value)
        grn_obj_unlink(context, rb_column->element_value);
    rb_column->element_value = NULL;
}

static void
rb_grn_variable_size_column_deconstruct (RbGrnVariableSizeColumn *rb_column,
                                         grn_obj **column,
                                         grn_ctx **context,
                                         grn_id *domain_id,
                                         grn_obj **domain,
                                         grn_obj **value,
                                         grn_obj **element_value,
                                         grn_id *range_id,
                                         grn_obj **range)
{
    RbGrnColumn *rb_grn_column;

    rb_grn_column = RB_GRN_COLUMN(rb_column);
    rb_grn_column_deconstruct(rb_grn_column, column, context,
                              domain_id, domain, value,
                              range_id, range);

    if (element_value)
        *element_value = rb_column->element_value;
}

/*
 * Document-class: Groonga::VariableSizeColumn < Groonga::Column
 *
 * A column for variable size data like text family types and vector
 * column.
 */

/*
 * It gets a value of variable size column value for the record that
 * ID is _id_.
 *
 * @example Gets weight vector value
 *    Groonga::Schema.define do |schema|
 *      schema.create_table("Products",
 *                          :type => :patricia_trie,
 *                          :key_type => "ShortText") do |table|
 *        # This is weight vector.
 *        # ":with_weight => true" is important to store weight value.
 *        table.short_text("tags",
 *                         :type => :vector,
 *                         :with_weight => true)
 *      end
 *    end
 *
 *    products = Groonga["Products"]
 *    rroonga = products.add("Rroonga")
 *    rroonga.tags = [
 *      {
 *        :value  => "ruby",
 *        :weight => 100,
 *      },
 *      {
 *        :value  => "groonga",
 *        :weight => 10,
 *      },
 *    ]
 *
 *    p rroonga.tags
 *    # => [
 *    #      {:value => "ruby",    :weight => 100},
 *    #      {:value => "groonga", :weight => 10}
 *    #    ]
 *
 * @overload [](id)
 *   @param [Integer, Record] id The record ID.
 *   @return [Array<Hash<Symbol, String>>] An array of value if the column
 *     is a weight vector column.
 *     Each value is a Hash like the following form:
 *
 *     <pre>
 *     {
 *       :value  => [KEY],
 *       :weight => [WEIGHT],
 *     }
 *     </pre>
 *
 *     @[KEY]@ is the key of the table that is specified as range on
 *     creating the weight vector.
 *
 *     @[WEIGHT]@ is a positive integer.
 *
 *   @return [::Object] See {Groonga::Object#[]} for columns except
 *     weight vector column.
 *
 * @since 4.0.1.
 */
static VALUE
rb_grn_variable_size_column_array_reference (VALUE self, VALUE rb_id)
{
    grn_ctx *context = NULL;
    grn_obj *column, *range;
    grn_column_flags flags;
    grn_id id;
    grn_obj *value;
    VALUE rb_value;
    VALUE rb_range;
    unsigned int i, n;

    rb_grn_variable_size_column_deconstruct(SELF(self), &column, &context,
                                            NULL, NULL, &value, NULL,
                                            NULL, &range);

    flags = grn_column_get_flags(context, column);
    if (!(flags & GRN_OBJ_WITH_WEIGHT)) {
        return rb_call_super(1, &rb_id);
    }

    id = RVAL2GRNID(rb_id, context, range, self);

    grn_obj_reinit(context, value,
                   value->header.domain,
                   value->header.flags | GRN_OBJ_VECTOR);
    grn_obj_get_value(context, column, id, value);
    rb_grn_context_check(context, self);

    rb_range = GRNTABLE2RVAL(context, range, GRN_FALSE);

    n = grn_vector_size(context, value);
    rb_value = rb_ary_new2(n);
    for (i = 0; i < n; i++) {
        VALUE rb_element_value;
        float weight = 0.0;
        grn_id domain;
        VALUE rb_element;

        if (value->header.type == GRN_UVECTOR) {
            grn_id id;
            id = grn_uvector_get_element_record(context, value, i, &weight);
            rb_element_value = rb_grn_record_new(rb_range, id, Qnil);
        } else {
            const char *element_value;
            unsigned int element_value_length;
            element_value_length =
                grn_vector_get_element_float(context,
                                             value,
                                             i,
                                             &element_value,
                                             &weight,
                                             &domain);
            rb_element_value = rb_str_new(element_value, element_value_length);
        }

        rb_element = rb_hash_new();
        rb_hash_aset(rb_element,
                     RB_GRN_INTERN("value"),
                     rb_element_value);
        if (flags & GRN_OBJ_WEIGHT_FLOAT32) {
            rb_hash_aset(rb_element,
                         RB_GRN_INTERN("weight"),
                         rb_float_new(weight));
        } else {
            rb_hash_aset(rb_element,
                         RB_GRN_INTERN("weight"),
                         UINT2NUM((uint32_t)weight));
        }

        rb_ary_push(rb_value, rb_element);
    }

    return rb_value;
}

typedef struct {
    VALUE self;
    grn_ctx *context;
    grn_obj *vector;
    grn_obj *element_value;
    grn_obj *range;
} HashElementToVectorElementData;

static int
hash_element_to_vector_element(VALUE key, VALUE value, VALUE user_data)
{
    HashElementToVectorElementData *data =
        (HashElementToVectorElementData *)user_data;
    float weight;

    weight = (float)NUM2DBL(value);

    if (data->vector->header.type == GRN_UVECTOR) {
        grn_id id = RVAL2GRNID(key, data->context, data->range, data->self);
        grn_uvector_add_element_record(data->context, data->vector, id, weight);
    } else {
        GRN_BULK_REWIND(data->element_value);
        RVAL2GRNBULK(key, data->context, data->element_value);

        grn_vector_add_element_float(data->context, data->vector,
                                     GRN_BULK_HEAD(data->element_value),
                                     GRN_BULK_VSIZE(data->element_value),
                                     weight,
                                     data->element_value->header.domain);
    }

    return ST_CONTINUE;
}

/*
 * It updates a value of variable size column value for the record
 * that ID is _id_.
 *
 * Weight vector column is a special variable size column. This
 * description describes only weight vector column. Other variable
 * size column works what you think.
 *
 * @example Use weight vector as matrix search result weight
 *    Groonga::Schema.define do |schema|
 *      schema.create_table("Products",
 *                          :type => :patricia_trie,
 *                          :key_type => "ShortText") do |table|
 *        # This is weight vector.
 *        # ":with_weight => true" is important for matrix search result weight.
 *        table.short_text("tags",
 *                         :type => :vector,
 *                         :with_weight => true)
 *      end
 *
 *      schema.create_table("Tags",
 *                          :type => :hash,
 *                          :key_type => "ShortText") do |table|
 *        # This is inverted index. It also needs ":with_weight => true".
 *        table.index("Products.tags", :with_weight => true)
 *      end
 *    end
 *
 *    products = Groonga["Products"]
 *    groonga = products.add("Groonga")
 *    groonga.tags = [
 *      {
 *        :value  => "groonga",
 *        :weight => 100,
 *      },
 *    ]
 *    rroonga = products.add("Rroonga")
 *    rroonga.tags = [
 *      {
 *        :value  => "ruby",
 *        :weight => 100,
 *      },
 *      {
 *        :value  => "groonga",
 *        :weight => 10,
 *      },
 *    ]
 *
 *    result = products.select do |record|
 *      # Search by "groonga"
 *      record.match("groonga") do |match_target|
 *        match_target.tags
 *      end
 *    end
 *
 *    result.each do |record|
 *      p [record.key.key, record.score]
 *    end
 *    # Matches all records with weight.
 *    # => ["Groonga", 101]
 *    #    ["Rroonga", 11]
 *
 *    # Increases score for "ruby" 10 times
 *    products.select(# The previous search result. Required.
 *                    :result => result,
 *                    # It just adds score to existing records in the result. Required.
 *                    :operator => Groonga::Operator::ADJUST) do |record|
 *      record.match("ruby") do |target|
 *        target.tags * 10 # 10 times
 *      end
 *    end
 *
 *    result.each do |record|
 *      p [record.key.key, record.score]
 *    end
 *    # Weight is used for increasing score.
 *    # => ["Groonga", 101]  <- Not changed.
 *    #    ["Rroonga", 1021] <- 1021 (= 101 * 10 + 1) increased.
 *
 * @overload []=(id, elements)
 *   This description is for weight vector column.
 *
 *   @param [Integer, Record] id The record ID.
 *   @param [Array<Hash<Symbol, String>>] elements An array of values
 *     for weight vector.
 *     Each value is a Hash like the following form:
 *
 *     <pre>
 *     {
 *       :value  => [KEY],
 *       :weight => [WEIGHT],
 *     }
 *     </pre>
 *
 *     @[KEY]@ must be the same type of the key of the table that is
 *     specified as range on creating the weight vector.
 *
 *     @[WEIGHT]@ must be an positive integer. Note that search
 *     becomes @weight + 1@. It means that You want to get 10 as
 *     score, you should set 9 as weight.
 *
 * @overload []=(id, value)
 *   This description is for variable size columns except weight
 *   vector column.
 *
 *   @param [Integer, Record] id The record ID.
 *   @param [::Object] value A new value.
 *   @see Groonga::Object#[]=
 *
 * @since 4.0.1
 */
static VALUE
rb_grn_variable_size_column_array_set (VALUE self, VALUE rb_id, VALUE rb_value)
{
    grn_ctx *context = NULL;
    grn_obj *column, *range;
    grn_column_flags column_flags;
    grn_rc rc;
    grn_id id;
    grn_obj *value, *element_value;
    int flags = GRN_OBJ_SET;

    rb_grn_variable_size_column_deconstruct(SELF(self), &column, &context,
                                            NULL, NULL, &value, &element_value,
                                            NULL, &range);

    column_flags = grn_column_get_flags(context, column);
    if (!(column_flags & GRN_OBJ_WITH_WEIGHT)) {
        VALUE args[2];
        args[0] = rb_id;
        args[1] = rb_value;
        return rb_call_super(2, args);
    }

    id = RVAL2GRNID(rb_id, context, range, self);

    grn_obj_reinit(context, value,
                   value->header.domain,
                   value->header.flags | GRN_OBJ_VECTOR);
    value->header.flags |= GRN_OBJ_WITH_WEIGHT;
    if (RVAL2CBOOL(rb_obj_is_kind_of(rb_value, rb_cArray))) {
        int i, n;
        n = RARRAY_LEN(rb_value);
        for (i = 0; i < n; i++) {
            float weight = 0;
            VALUE rb_element_value, rb_weight;

            rb_grn_scan_options(RARRAY_PTR(rb_value)[i],
                                "value", &rb_element_value,
                                "weight", &rb_weight,
                                NULL);

            if (!NIL_P(rb_weight)) {
                weight = (float)NUM2DBL(rb_weight);
            }

            if (value->header.type == GRN_UVECTOR) {
                grn_id id = RVAL2GRNID(rb_element_value, context, range, self);
                grn_uvector_add_element_record(context, value, id, weight);
            } else {
                GRN_BULK_REWIND(element_value);
                if (!NIL_P(rb_element_value)) {
                    RVAL2GRNBULK(rb_element_value, context, element_value);
                }

                grn_vector_add_element_float(context, value,
                                             GRN_BULK_HEAD(element_value),
                                             GRN_BULK_VSIZE(element_value),
                                             weight,
                                             element_value->header.domain);
            }
        }
    } else if (RVAL2CBOOL(rb_obj_is_kind_of(rb_value, rb_cHash))) {
        HashElementToVectorElementData data;
        data.self = self;
        data.context = context;
        data.vector = value;
        data.element_value = element_value;
        data.range = range;
        rb_hash_foreach(rb_value, hash_element_to_vector_element, (VALUE)&data);
    } else {
        rb_raise(rb_eArgError,
                 "<%s>: "
                 "weight vector value must be an array of index value or "
                 "a hash that key is vector value and value is vector weight: "
                 "<%s>",
                 rb_grn_inspect(self),
                 rb_grn_inspect(rb_value));
    }

    rc = grn_obj_set_value(context, column, id, value, flags);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return rb_value;
}

/*
 * Returns whether the column is compressed or not. If
 * @type@ is specified, it returns whether the column is
 * compressed by @type@ or not.
 * @overload compressed?
 *   @return [Boolean] whether the column is compressed or not.
 * @overload compressed?(type)
 *   @param [:zlib, :lz4, :zstd, :zstandard] type (nil)
 *   @return [Boolean] whether specified compressed type is used or not.
 * @since 1.3.1
 */
static VALUE
rb_grn_variable_size_column_compressed_p (int argc, VALUE *argv, VALUE self)
{
    RbGrnVariableSizeColumn *rb_grn_column;
    grn_ctx *context = NULL;
    grn_obj *column;
    grn_column_flags flags;
    VALUE type;
    grn_bool compressed_p = GRN_FALSE;
    grn_bool accept_any_type = GRN_FALSE;
    grn_bool need_zlib_check = GRN_FALSE;
    grn_bool need_lz4_check = GRN_FALSE;
    grn_bool need_zstd_check = GRN_FALSE;

    rb_scan_args(argc, argv, "01", &type);

    if (NIL_P(type)) {
        accept_any_type = GRN_TRUE;
    } else {
        if (rb_grn_equal_option(type, "zlib")) {
            need_zlib_check = GRN_TRUE;
        } else if (rb_grn_equal_option(type, "lzo")) {
            /* TODO: for backward compatibility */
            need_lz4_check = GRN_TRUE;
        } else if (rb_grn_equal_option(type, "lz4")) {
            need_lz4_check = GRN_TRUE;
        } else if (rb_grn_equal_option(type, "zstd")) {
            need_zstd_check = GRN_TRUE;
        } else if (rb_grn_equal_option(type, "zstandardd")) {
            need_zstd_check = GRN_TRUE;
        } else {
            rb_raise(rb_eArgError,
                     "compressed type should be "
                     "<:zlib>, <:lz4>, <:zstd> or <:zstandard>: <%s>",
                     rb_grn_inspect(type));
        }
    }

    rb_grn_column = SELF(self);
    rb_grn_object_deconstruct(RB_GRN_OBJECT(rb_grn_column), &column, &context,
                              NULL, NULL,
                              NULL, NULL);

    flags = grn_column_get_flags(context, column);
    switch (flags & GRN_OBJ_COMPRESS_MASK) {
      case GRN_OBJ_COMPRESS_ZLIB:
        if (accept_any_type || need_zlib_check) {
            grn_obj support_p;
            GRN_BOOL_INIT(&support_p, 0);
            grn_obj_get_info(context, NULL, GRN_INFO_SUPPORT_ZLIB, &support_p);
            compressed_p = GRN_BOOL_VALUE(&support_p);
        }
        break;
      case GRN_OBJ_COMPRESS_LZ4:
        if (accept_any_type || need_lz4_check) {
            grn_obj support_p;
            GRN_BOOL_INIT(&support_p, 0);
            grn_obj_get_info(context, NULL, GRN_INFO_SUPPORT_LZ4, &support_p);
            compressed_p = GRN_BOOL_VALUE(&support_p);
        }
        break;
      case GRN_OBJ_COMPRESS_ZSTD:
        if (accept_any_type || need_zstd_check) {
            grn_obj support_p;
            GRN_BOOL_INIT(&support_p, 0);
            grn_obj_get_info(context, NULL, GRN_INFO_SUPPORT_ZSTD, &support_p);
            compressed_p = GRN_BOOL_VALUE(&support_p);
        }
        break;
    }

    return CBOOL2RVAL(compressed_p);
}

/*
 * Defrags the column.
 *
 * @overload defrag(options={})
 *   @param options [::Hash] The name and value
 *     pairs. Omitted names are initialized as the default value.
 *   @option options [Integer] :threshold (0) the threshold to
 *     determine whether a segment is defraged. Available
 *     values are -4..22. -4 means all segments are defraged.
 *     22 means no segment is defraged.
 * @return [Integer] the number of defraged segments
 * @since 1.2.6
 */
static VALUE
rb_grn_variable_size_column_defrag (int argc, VALUE *argv, VALUE self)
{
    RbGrnVariableSizeColumn *rb_grn_column;
    grn_ctx *context = NULL;
    grn_obj *column;
    int n_segments;
    VALUE options, rb_threshold;
    int threshold = 0;

    rb_scan_args(argc, argv, "01", &options);
    rb_grn_scan_options(options,
                        "threshold", &rb_threshold,
                        NULL);
    if (!NIL_P(rb_threshold)) {
        threshold = NUM2INT(rb_threshold);
    }

    rb_grn_column = SELF(self);
    rb_grn_object_deconstruct(RB_GRN_OBJECT(rb_grn_column), &column, &context,
                              NULL, NULL,
                              NULL, NULL);
    n_segments = grn_obj_defrag(context, column, threshold);
    rb_grn_context_check(context, self);

    return INT2NUM(n_segments);
}

/*
 * Recreates all index columns for the column.
 *
 * This method is useful when you have any broken index columns for
 * the column. You don't need to specify each index column. But this
 * method spends more time rather than you specify only reindex
 * needed index columns.
 *
 * You can use {Groonga::Database#reindex} to recreate all index
 * columns in a database.
 *
 * You can use {Groonga::TableKeySupport#reindex} to recreate all
 * index columns in a table.
 *
 * You can use {Groonga::IndexColumn#reindex} to specify the reindex
 * target index column.
 *
 * @example How to recreate all index columns for the column
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
 *   Groonga["Memos.content"].reindex
 *   # They are called:
 *   #   Groonga["BigramTerms.Memos_content"].reindex
 *   #   Groonga["MeCabTerms.Memos_content"].reindex
 *   #
 *   # They aren't called:
 *   #   Groonga["BigramTerms.Memos_title"].reindex
 *   #   Groonga["MeCabTerms.Memos_title"].reindex
 *
 * @overload reindex
 *   @return [void]
 *
 * @see Groonga::Database#reindex
 * @see Groonga::TableKeySupport#reindex
 * @see Groonga::FixSizeColumn#reindex
 * @see Groonga::IndexColumn#reindex
 *
 * @since 5.1.1
 */
static VALUE
rb_grn_variable_size_column_reindex (VALUE self)
{
    grn_rc rc;
    grn_ctx *context;
    grn_obj *column;

    rb_grn_variable_size_column_deconstruct(SELF(self), &column, &context,
                                            NULL, NULL, NULL, NULL,
                                            NULL, NULL);

    rc = grn_obj_reindex(context, column);
    rb_grn_context_check(context, self);
    rb_grn_rc_check(rc, self);

    return Qnil;
}

void
rb_grn_init_variable_size_column (VALUE mGrn)
{
    rb_cGrnVariableSizeColumn =
        rb_define_class_under(mGrn, "VariableSizeColumn", rb_cGrnDataColumn);

    rb_define_method(rb_cGrnVariableSizeColumn, "[]",
                     rb_grn_variable_size_column_array_reference, 1);
    rb_define_method(rb_cGrnVariableSizeColumn, "[]=",
                     rb_grn_variable_size_column_array_set, 2);

    rb_define_method(rb_cGrnVariableSizeColumn, "compressed?",
                     rb_grn_variable_size_column_compressed_p, -1);
    rb_define_method(rb_cGrnVariableSizeColumn, "defrag",
                     rb_grn_variable_size_column_defrag, -1);

    rb_define_method(rb_cGrnVariableSizeColumn, "reindex",
                     rb_grn_variable_size_column_reindex, 0);
}
