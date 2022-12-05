# NEWS

## 12.1.0: 2022-12-05 {#version-12-1-0}

### Fixes

  * [{Groonga::PatriciaTrie}] Fixed a bug that `Groonga::PatriciaTrie#scan` returned wrong offsets if there are many hit words. [GitHub#207][Patch by Naoya Murakami]

### Thanks

  * Naoya Murakami

## 12.0.8: 2022-09-28 {#version-12-0-8}

### Improvements

  * Add support for creating a {Groonga::PatriciaTrie} without database.

## 12.0.2: 2022-04-04 {#version-12-0-2}

### Improvements

  * Added support for `:weight_float32` for vector column. This
    requires Groonga 12.0.2 or later.

  * Removed needless `Groonga::IndexColumn#with_weight?`.

  * Added support for `:missing_mode` and `:invalid_mode` for scalar
    column and vector column. This requires Groonga 12.0.2 or later.

## 12.0.0: 2022-02-09 {#version-12-0-0}

### Improvements

  * Added support for Ruby 3.1.

## 11.0.6: 2021-09-24 {#version-11-0-6}

### Improvements

  * Installed Groonga automatically when native package isn't
    installed automatically.
    [GitHub#204][Patch by skawaji]

### Thanks

  * skawaji

## 11.0.0: 2021-02-09 {#version-11-0-0}

### Improvements

  * {Groonga::ConnectionReset} Added.

  * {Groonga::Context.open} Added.

  * Added support for Ractor.

  * {Groonga::Database} Accept path like object in .open/.create.

## 10.0.6: 2020-09-01 {#version-10-0-6}

### Fixes

  * Fixed a bug that failed `gem install rroonga` in Windows version.

## 10.0.2: 2020-04-29 {#version-10-0-2}

### Improvements

  * Added support for Float32 in Groonga 10.0.2.

  * [doc] Fixed markup of a image in tutorial [GitHub#192][Patched by takahashim]

### Thanks

  * takahashim

## 10.0.1: 2020-04-02 {#version-10-0-1}

### Improvements

  * Added support for creating a table with normalizer with options

  * Windows: Added Ruby 2.7 support.

  * Windows: Dropped Ruby 2.3.

    * This version has been EOL.

  * Dropped fat gem support.

## 9.0.7: 2019-08-29 {#version-9-0-7}

### Improvements

  * {Groonga::Object#lexicon?} Added.

  * {Groonga::IndexColumn} Added support "INDEX_LARGE".

  * {Groonga::IndexCursor#set_min?} Added.

  * {Groonga::IndexCursor#set_min=} Added.

  * {Groonga::Context#force_match_escalation?} Added.

  * {Groonga::Context#force_match_escalation=} Added.

  * {Groonga::Object#id_accessor?} Added.

  * {Groonga::Logger.flags} Added support for thread id option.

  * {Groonga::Object#bulk?} Added.

  * {Groonga::Object#value_accessor?} Added.

  * {Groonga::Table} Delegate support_score? from Groonga::Record to Groonga::Table.

  * Groonga::Flashable Added support dependent option.

  * {Groonga::Object#score_accessor?} Added.

  * {Groonga::Object#nsubrecs_accessor? } Added.

  * Translate a part of explanation of Groonga::Table into English.

## 9.0.3: 2019-05-10 {#version-9-0-3}

### Improvements

  * Windows: Added Ruby 2.6 support.

## 9.0.2: 2019-05-10 {#version-9-0-2}

### Improvements

  * Disable groonga-httpd by default.

  * Added support for "--with-groonga-configure-options".
    [GitHub#146][Reported by Tomás Pollak]

  * Groonga::RegexpExpressionBuilder: Added support (?-mix:XXX) for searching of regular expression.
    [groonga-dev,04694][Reported by Masatoshi SEKI]

  * Use "groonga-latest.tar.gz" for build Groonga.

  * {Groonga::Schema#create_lexicon}: Added.

### Thanks

  * Tomás Pollak

  * Masatoshi SEKI

## 7.1.1: 2018-01-30 {#version-7-1-1}

### Improvements

  * Added Groonga 7.1.1 support. Groonga 7.1.0 or older aren't supported.

  * Added Apache Arrow support:

    * {Groonga::Table#load_arrow}

    * {Groonga::Table#dump_arrow}

  * Windows: Added Ruby 2.4 or later support.
    [groonga-dev,04565][Reported by ongaeshi]

  * {Groonga::ColumnCache}: Added.

  * {Groonga::Expression#parse}: Added `:no_syntax_error` option.

  * {Groonga::Context#support_arrow?}: Added.

  * {Groonga::Procedure#stable?}: Added.

  * {Groonga::Array}: Removed queue support.

  * {Groonga::QueryLogger.flags}: Added.

  * {Groonga::QueryLogger.flags=}: Added.

  * {Groonga::Object#corrupt?}: Added.

  * {Groonga::Object#disk_usage}: Added.

### Thanks

  * naofumi-fujii

  * ongaeshi

## 7.0.2: 2017-04-29 {#version-7-0-2}

### Improvements

  * Supported Groonga 7.0.2. Groonga 7.0.1 or older aren't supported.

  * {Groonga::IndexColumn#open_cursor}: Supported opening a cursor by
    token.

  * {Groonga::IndexColumn#[]}: Supported a token.

  * {Groonga::InvertedIndexCursor#closed?}: Added.

  * {Groonga::Table::KeySupport#key?}: Added.

  * {Groonga::Table::KeySupport#has_key?}: Deprecated. Use
    {Groonga::Table::KeySupport#key?} instead.

  * {Groonga::DataColumn#apply_window_function}: Supported `:group_keys`.

  * {Groonga::Column#weight_vector?}: Added.

  * {Groonga::DataColumn#apply_expression}: Added.

  * {Groonga::Column#data?}: Added.

  * {Groonga::DefaultCache}: Added.

## 6.1.3: 2017-01-12 {#version-6-1-3}

### Improvements

  * Supported Groonga 6.1.3. Groonga 6.1.0 or older aren't supported.

  * Added {Groonga::Context#support_zstd?}.

  * Added {Groonga::ZstdError}.

  * Supported Zstandard compression.
    If Zstandard is available in your enviromemt, you can use Zstandard
    compression for columns. How to use:

          Groonga::Schema.define do |schema|
            schema.create_table("Posts") do |table|
              table.short_text("title", :compress => :zstandard)
              # shortened form
              # table.short_text("title", :compress => :zstd)
            end
          end

  * Added PID for {Groonga::Logger::Flags}.

  * Added {Groonga::Logger.flags}.

  * Added {Groonga::Logger.flags=}.

## 6.1.0: 2016-11-07 {#version-6-1-0}

### Improvements

  * {Groonga::Table#define_index_column}: Added `:size` option to
    support small or medium size of index column. Specify `:small` or
    `:medium` to customize it.

## 6.0.9: 2016-10-07 {#version-6-0-9}

### Improvements

  * Supported Groonga 6.0.9. Groonga 6.0.8 or older aren't supported.
  * {Groonga::Database#remove_force}: Added a new method that removes
    broken object.
  * {Groonga::ID.builtin_type?}: Added a new predicate method that checks
    whether the given ID is ID for builtin_type object or not.

## 6.0.7: 2016-08-13 {#version-6-0-7}

### Improvements

  * Supported Groonga 6.0.7. Groonga 6.0.5 or older aren't supported.

  * Improved integration with libraries that uses `at_exit` such as
    test-unit.

### Fixes

  * {Groonga::FixSizeColumn#inspect}: Fixed wrong flags.

  * {Groonga::VariableSizeColumn#inspect}: Fixed wrong flags.

## 6.0.5: 2016-07-15 {#version-6-0-5}

### Improvements

  * Supported Groonga 6.0.5. Groonga 6.0.4 or older aren't supported.

  * {Groonga::ID}: Added a module that provided object ID related
    features.

  * {Groonga::ID.builtin?}: Added a new predicate method that checks
    whether the given ID is ID for builtin object or not.

  * {Groonga::ID::NIL}: Added a new constant that represents invalid ID.

  * {Groonga::ID::MAX}: Added a new constant that represents the maximum ID.

  * {Groonga::Name}: Added a new module that provides object name
    related features.

  * {Groonga::Name.column?}: Added a new predicate method that checks
    whether the given name is column object name or not.

  * {Groonga::Object#column?}: Added a new predicate for column.

  * {Groonga::Object#reference_column?}: Added a new predicate for
    column that uses table as value type.

  * {Groonga::Object#index_column?}: Added a new predicate for
    index column.

  * {Groonga::Object#touch}: Added a new method that updates the last
    modified time of the object.

  * {Groonga::Object#last_modified}: Added a new method that returns
    the last modified time of the object.

  * {Groonga::Object#dirty?}: Added a new method that checks whether
    the object is changed after the last flush.

## 6.0.4: 2016-06-14 {#version-6-0-4}

### Improvements

  * Supported Groonga 6.0.4. Groonga 6.0.3 or older aren't supported.

  * {Groonga::WindowFunctionError}: Added a new error class.

  * {Groonga::ProcedureType::WINDOW_FUNCTION}: Added a new procedure
    type for window function.

  * {Groonga::Object#window_function_procedure?}: Added a new
    predicate for window function.

  * {Groonga::DataColumn#apply_window_function}: Added a new
    method to apply a window function.

## 6.0.2: 2016-05-18 {#version-6-0-2}

### Improvements

  * Supported Groonga 6.0.2. Groonga 6.0.1 or older aren't supported.

  * {Groonga::Table#select}: Supported nested array as conditions.

  * Supported HTTP proxy with authentication on install.
    [GitHub#129] [Patch by ngram]

  * {Groonga::Cancel}: Added a new error class.

  * {Groonga::Table#select}: Supported passing Hash as function argument.

  * {Groonga::Hash.create}: Added `:key_large` option to create hash
    table that supports 1TiB total key size.

  * {Groonga::Type#text_family?}: Added a new predicate that returns
    `true` for `ShortText`, `Text` and `LongText` types.

  * {Groonga::Type#number_family?}: Added a new predicate that returns
    `true` for `UInt8`, `Int8`, ..., `UInt64`, `Int64` and `Float`
    types.

  * {Groonga::Type#builtin?}: Added a new predicate that returns
    `true` for builtin types such as `Int32`.

  * {Groonga::Object#remove}: Added `:dependent` option to remove all
    tables and columns that depend on the object.

  * {Groonga::RequestCanceler}: Added an interface that cancels one or
    more requests.

  * {Groonga::RequestTimer}: Added an interface that provides request
    timeout feature.

## 6.0.0: 2016-03-06 {#version-6-0-0}

### Improvements

  * Supported Groonga 6.0.0. Groonga 5.1.2 or older aren't supported.

  * Supported `-` operator as {Groonga::Operator::AND_NOT}.

  * `grndump`: Added `:max_records` option.
    [Patch by Genki Takiuchi]

  * {Groonga::Table#select}: Supported integer and time as function
    call arguments.

  * {Groonga::Table#select}: Disabled auto conversion to record ID
    from number object in condition block. If you want to specify
    record ID as right hand side value, you need to pass
    {Groonga::Record}. The number object is treated as key not ID from
    this release. Because we couldn't specify Int/UInt family type key
    as number object. The number object was always treated as ID not
    key.

    It's backward incompatible change but we introduced the
    change. Because we marked this behavior as a bug.

  * Supported `require "rroonga"` for `Bundler.require`.
    [Patch by Fumiaki MATSUSHIMA]

  * Added {Groonga::Plugin.names}.

  * Added {Groonga::Config#delete}.

  * Added {Groonga::Config#each}.

  * Added {Groonga::Index}. It has index column and section.

  * Added {Groonga::Column#find_indexes}.
    It returns an array of {Groonga::Index}.

  * Mark {Groonga::Column#indexes} as deprecated.
    Use {Groonga::Column#find_indexes} instead.

  * Added {Groonga.error_message}.

  * Supported method style function call in {Groonga::Table#select}
    block. It's Rubyish API.

    Example:

        @shops.select do |record|
          record.location.geo_in_rectangle("35.7185,139.7912",
                                           "35.7065,139.8069")
        end

### Thanks

  * Genki Takiuchi

  * Fumiaki MATSUSHIMA

## 5.1.1: 2016-01-18 {#version-5-1-1}

### Improvements

  * Supported Groonga 5.1.1. Groonga 5.0.9 or older aren't supported.
  * Supported accessor in {Groonga::Table#geo_sort}.
  * Improved performance of searching with empty condition.
  * Added {Groonga::Object#accessor?}.
  * Added {Groonga::Object#key_accessor?}.
  * Added {Groonga::Database#reindex}.
  * Added {Groonga::Table::KeySupport#reindex}.
  * Added {Groonga::FixSizeColumn#reindex}.
  * Added {Groonga::VariableSizeColumn#reindex}.
  * Added {Groonga::IndexColumn#reindex}.
  * Added {Groonga.package_label}.
  * Renamed {Groonga::Conf} to {Groonga::Config}.
  * Renamed {Groonga::Context#conf} to {Groonga::Context#config}.

## 5.0.9: 2015-11-10 {#version-5-0-9}

### Improvements

  * Supported Groonga 5.0.9. Groonga 5.0.8 or older aren't supported.
  * Added {Groonga::Context#conf} that returns {Groonga::Conf}.
  * Added {Groonga::Conf#[]} that returns a database level configuration value.
  * Added {Groonga::Conf#[]=} that sets a database level configuration value.
  * Added {Groonga::Table#geo_sort=} that sorts by geo point.

## 5.0.8: 2015-10-07 {#version-5-0-8}

### Improvements

  * Changed to use `gmake` preferentially for auto Groonga build.
  * Supported Groonga 5.0.8. Groonga 5.0.7 or older aren't supported.
  * Added {Groonga::Context#opened?} that checks whether object with
    the specified ID is opened or not.
  * Supported calling a function in {Groonga::Table#select} by
    `record.call("function", arg1, arg2, ...)`.
  * Windows: Changed to cross compile system to
    [rake-compiler-dock](https://github.com/rake-compiler/rake-compiler-dock)
    from Vagrant based cross compile system.
    [GitHub#108] [Patch by Hiroshi Hatake]

### Thanks

  * Hiroshi Hatake

## 5.0.5: 2015-09-09 {#version-5-0-5}

### Improvements

  * Supported Groonga 5.0.7. Groonga 5.0.6 or older aren't supported.
  * Added {Groonga::Object#selector_only_procedure?}.
  * Supported {Groonga::Table#open_cursor} with `:order_by => :key`
    against {Groonga::DoubleArrayTrie}.
  * Added {Groonga::Database#unmap}.
  * Added {Groonga::Thread.limit}.
  * Added {Groonga::Thread.limit=}.
  * Added {Groonga::Thread.limit_getter=}.
  * Added {Groonga::Thread.limit_setter=}.
  * Added {Groonga::WindowsEventLogger.register}.
  * Added {Groonga::Logger.max_level}.
  * Added {Groonga::Logger.max_level=}.

### Fixes

  * `grndump`: Fixed an error when `--order-by=key` is specified
    against database that has `Groonga::Array` or `Groonga::Hash`.

## 5.0.4: 2015-07-13 {#version-5-0-4}

### Improvements

  * Supported Groonga 5.0.5. Groonga 5.0.4 or older aren't supported.
  * Added {Groonga::Flushable#flush} to bind `grn_obj_flush()` and `grn_obj_flush_recursive()`.
  * Included {Groonga::Flushable} module to {Groonga::Table}.
  * Included {Groonga::Flushable} module to {Groonga::Column}.
  * Included {Groonga::Flushable} module to {Groonga::Database}.

## 5.0.3: 2015-06-10 {#version-5-0-3}

### Improvements

  * Removed deprecated option "rubyforge_project" from .gemspec.
    [GitHub#95][Patch by takiy33]
  * [groonga-database-inspect] Added column value type.
  * [groonga-database-inspect] Added sources of index column.
  * [groonga-database-inspect] Added the number of sources of index column.
  * [groonga-database-inspect] Added `--log-path` option.
  * [groonga-database-inspect] Reduced memory usage.
  * [windows] Updated bundled Groonga version to 5.0.4.

### Thanks

  * takiy33

## 5.0.2: 2015-05-18 {#version-5-0-2}

### Improvements

  * Added {Groonga::Plugin.ruby_suffix}.
  * {Groonga::Hash} is used as the default table type when `:key_type`
    is specified in {Groonga::Schema}. {Groonga::Array} was used as
    the default table type. It's a backward incompatible change. But
    nobody will not stumped. Because `:key_type` is specified but
    `:type` isn't specified case is a bug of user's program.
  * Added {Groonga::Logger.log}.
  * Added {Groonga::Logger.rotate_threshold_size}.
  * Added {Groonga::Logger.rotate_threshold_size=}.
  * Added {Groonga::QueryLogger.log}.
  * Added {Groonga::QueryLogger.rotate_threshold_size}.
  * Added {Groonga::QueryLogger.rotate_threshold_size=}.
  * Implemented {Groonga::QueryLogger::Flags.parse}.

## 5.0.1: 2015-04-14 {#version-5-0-1}

### Improvements

  * Supported Groonga 5.0.2. Groonga 5.0.1 or older aren't supported.
  * Added {Groonga::Expression#estimate_size}.
  * Added closed check in `#encoding`. [GitHub#54] [Reported by yui-knk]
  * [windows] Supported Ruby 2.2.
  * Supported `Groonga::Normalizer.normalize("")`. [GitHub#55]
    [Reported by Tasuku SUENAGA]
  * Added {Groonga::ScorerError}.
  * Added shortcuts for `TokenRegexp` to `Groonga::Schema`.
  * Added {Groonga::Operator::REGEXP}.
  * Added {Groonga::Plugin.unregister}.
  * Added {Groonga::Context#unregister_plugin}.
  * Changed {Groonga::Operator} to class from module. It's a backward
    incompatible change but nobody will not be effected.
  * Removed unused `:id` option from the followings:
    * {Groonga::Column#clear_lock}
    * {Groonga::Column#locked?}
    * {Groonga::Table#clear_lock}
    * {Groonga::Table#locked?}
  * Added {Groonga::EqualOperator#exec}.
  * Added {Groonga::NotEqualOperator#exec}.
  * Added {Groonga::LessOperator#exec}.
  * Added {Groonga::GreaterOperator#exec}.
  * Added {Groonga::LessEqualOperator#exec}.
  * Added {Groonga::GreaterEqualOperator#exec}.
  * Added {Groonga::MatchOperator#exec}.
  * Added {Groonga::PrefixOperator#exec}.
  * Added {Groonga::RegexpOperator#exec}.
  * Added {Groonga::ProcedureType::TOKEN_FILTER}.
  * Added {Groonga::ProcedureType::SCORER}.
  * Added {Groonga::Operator#to_s}.
  * Supported {Groonga::IndexColumn#estimate_size} against query.
  * Supported {Groonga::IndexColumn#estimate_size} against lexicon cursor.
  * Added {Groonga::Object#table?}.
  * Added {Groonga::Object#procedure?}.
  * Added {Groonga::Object#function_procedure?}.
  * Added {Groonga::Object#selector_procedure?}.
  * Added {Groonga::Object#scorer_procedure?}.
  * Supported regular expression in {Groonga::Table#select} block.

### Thanks

  * yui-knk
  * Tasuku SUENAGA

## 5.0.0: 2015-02-16 {#version-5-0-0}

### Improvements

* Supported Groonga 5.0.0. Groonga 4.1.1 or older aren't supported.
* Added flags for {Groonga::Normalizer.normalize}.
  [GitHub#44] [Patch by Tasuku SUENAGA a.k.a. gunyarakun]

      Groonga::Normalizer.normalize("AbC Def　gh")                         #=> "abcdefgh"
      Groonga::Normalizer.normalize("AbC Def　gh", :remove_blank => true)  #=> "abcdefgh"
      Groonga::Normalizer.normalize("AbC Def　gh", :remove_blank => false) #=> "abc def gh"

* Supported drilldown by multiple keys in {Groonga::Table#group}.
* Supported calculation for drilldown in {Groonga::Table#group}.

### Thanks

* Tasuku SUENAGA a.k.a. gunyarakun

## 4.0.8: 2015-01-08 {#version-4-0-8}

### Improvements

* Supported Groonga 4.0.9. Groonga 4.0.8 or older aren't supported.
* Added {Groonga::Column#truncate}. [GitHub#41] [Patch by Hiroshi Hatake]
* Added {Groonga::Database#recover}.

### Fixes

* Fixed a typo in {Groonga::GeoPoint#==}.

### Thanks

* Hiroshi Hatake

## 4.0.7: 2014-12-12 {#version-4-0-7}

### Improvements

* Supported Groonga 4.0.8. Groonga 4.0.7 or older aren't supported.
* Added `:reuse_posting_object` option to
  {Groonga::IndexCursor#each}. The option improves performance by
  reducing memory allocation. The option is disabled by default because
  reusing the same object isn't Rubyish.
* Added {Groonga::IndexColumn#estimate_size}. [GitHub#38]
* Accepted string for integer, unsigned integer and float type key.

## 4.0.6: 2014-11-06 {#version-4-0-6}

### Improvements

* Supported Groonga 4.0.7. [GitHub#28]
  * Changed {Groonga::Context#support_lzo?} always returns `false`.
  * Added {Groonga::Context#support_lz4?}.
  * Supported `:lz4` in {Groonga::Table#define_column} options.
  * Supported `:lz4` in {Groonga::VariableSizeColumn#compressed?} options.
  * Added {Groonga::LZ4Error}.
  * [grndump] Supported `COMPRESS_ZLIB` and `COMPRESS_LZ4` flags.
  * Added {Groonga::Table::KeySupport#token_filters}.
  * Added {Groonga::Table::KeySupport#token_filters=}.
  * Supported `:token_filters` in {Groonga::Hash.create} options.
  * Supported `:token_filters` in {Groonga::PatriciaTrie.create} options.
  * Supported `:token_filters` in {Groonga::DoubleArrayTrie.create} options.
  * Supported `:token_filters` in {Groonga::Schema.create_table} options.
  * Added {Groonga::TokenFilterError}.
  * [grndump] Supported token filters.
* Added {Groonga::Expression#keywords}. [GitHub#30]
* Stopped to require logger object as the first argument of
  {Groonga::Logger.register} when block is specified.

### Fixes

* [doc] Removed deprecated example.
  [GitHub#26] [Patch by ongaeshi]
* Fixed a bug that a column assignment raises an error when
  you assign value with type A and then assign value type B again.
  [GitHub#27] [Patch by Daisuke Goto]
* Fixed a memory leak in {Groonga::PatriciaTrie#open_near_cursor}.
* Fixed a bug that you can access a column renamed by
  {Groonga::Column#rename} with old name.
  [GitHub#29] [Patch by Daisuke Goto]
* [doc] Fixed wrong option name of {Groonga::Logger.register}.

### Thanks

* ongaeshi
* Daisuke Goto

## 4.0.5: 2014-10-05 {#version-4-0-5}

### Improvements

* [windows] Added cross build task.
* Added {Groonga::DoubleArrayTrie#update}.
  [GitHub#24] [Patch by Masafumi Yokoyama]
* Added {Groonga::Record#rename}.
* [windows] Disabled debug flags.

### Fixes

* Added `:id => true` option to {Groonga::Table::KeySupport#delete}.
  `Groonga::Table::KeySupport#delete` accepts both ID and key. If passed
  value is integer, it is handled as ID. But we can use `Int32` as key.
  In the case, we can't delete a record by key. Therefore, we added
  `Groonga::Table::KeySupport#delete(id, :id => true)` API. It introduces
  a backward incompatibility, but it is OK because the current API is a
  bug.

### Thanks

* Masafumi Yokoyama

## 4.0.4: 2014-08-29 {#version-4-0-4}

### Improvements

* Supported Groonga 4.0.4 or later. Groonga 4.0.3 or older are not supported.
* Added {Groonga::ProcedureType} that has constants as GRN_PROC_XXX.
* Added {Groonga::Procedure#type} as grn_proc_get_type().

      procedure = Groonga["TokenBigram"]
      p procedure.type == Groonga::ProcedureType::TOKENIZER  #=> true

* Avoided a crash when {Groonga::Record#inspect} is called on exit.
  [GitHub#18] [Reported by Ippei Obayashi]
* Updated Free Software Foundation address. [Masafumi Yokoyama]
* Supported reference weight vector. It requires Groonga 4.0.4 or later.
* Updated homepage in gemspec to use ranguba.org. [Masafumi Yokoyama]
* Reduce memory usage without GC.

### Fixes

* Fixed a GC related crash when GC is run while {Groonga::Table#group}.

### Thanks

* Ippei Obayashi
* uu59
* cosmo0920

## 4.0.3: 2014-06-04 {#version-4-0-3}

### Fixes

* [windows] Fixed a bug that Rroonga reports load error by
  bundling the fixed version Groonga package.
  [groonga-dev,02398][Reported by Masafumi Yokoyama]

### Thanks

* Masafumi Yokoyama

## 4.0.2: 2014-05-29 {#version-4-0-2}

### Improvements

* Removed needless `--key_type` option in dump.
  [Reported by Genki Takiuchi]
* Added sources information to {Groonga::IndexColumn#inspect}.
* Enabled `with_section` flag for multiple column index by default.
  See {Groonga::Schema::TableDefinition#index}.
* Enabled `with_position` flag for index of a table that has a default
  tokenizer by default. You need to specify `:with_position => false`
  explicitly to disable `with_position` flag for `TokenDelimit`.
  `TokenDelimit` is a tokenizer that doesn't need `with_position` flag.
  See {Groonga::Schema.create_table}.

### Fixes

* [GitHub#16] Fixed a memory leak of {Groonga::Table#column}.
  [Reported by rutice]

### Thanks

* rutice
* Genki Takiuchi

## 4.0.1: 2014-04-04 {#version-4-0-1}

### Improvements

* Supported Groonga 4.0.1. Groonga 4.0.0 or older are not supported.
* Supported no weight match column case.

      table.select do |record|
        match_target = record.match_target do |target|
          target.column
        end
        match_target =~ keyword
      end

* Supported weight vector.
* grndump: Changed to use `--normalizer` instead of `KEY_NORMALIZE`.
  Old Groonga can't restore dumped database.
* {Groonga::IndexColumn#search}: Added `mode` option.
* {Groonga::IndexColumn#search}: Added `weight` option.
* Accepted String as option key.

### Fixes

* Fixed a bug that index dump drops index options in Ruby syntax.


## 4.0.0: 2014-02-09 {#version-4-0-0}

### Improvements

* Supported Groonga 4.0.0. Groonga 3.1.2 or older are not supported.
* Added install document. [Patch by ongaeshi]

### Thanks

* ongaeshi


## 3.1.2: 2014-01-29 {#version-3-1-2}

### Improvements

* Supported Groonga 3.1.2. Groonga 3.1.1 or older are not supported.
* Added {Groonga::Table#support_value?}.
* Added {Groonga::Record#support_value?}.
* Added `_value` value to {Groonga::Record#attributes} result.
  [groonga-dev,02046] [Suggested by ongaeshi]
* Added column values to {Groonga::Record#inspect} result.
  [groonga-dev,02048] [Suggested by ongaeshi]
* grndump: Added `--dump-indexes` option that controls schema for
  indexes output. It is useful to dump only schema for indexes.
* Added {Groonga.lock_timeout}.
* Added {Groonga.lock_timeout=}.

### Thanks

* ongaeshi

## 3.1.1: 2013-12-29 {#version-3-1-1}

### Improvements

* Supported Groonga 3.1.1.
* [groonga-database-inspect] Added `--no-tables` option for hiding
  all table information.
* [groonga-database-inspect] Added `--no-columns` option for hiding
  all column information.
* [doc] Updated English mailing list to use Groonga-talk.
  [GitHub#14] [Patch by Masafumi Yokoyama]
* {Groonga::Expression#append_operation} supports operator name.
* {Groonga::Expression#append_constant} supports operator name.
* {Groonga::Expression#append_object} supports operator name.

### Thanks

* Masafumi Yokoyama

## 3.1.0: 2013-11-29 {#version-3-1-0}

### Improvements

* Reduced build failure by automatically Groonga build.
  [Reported by SHIMADA Koji]
* Added `groonga-database-inspect` command that inspects passed database.
  [Suggested by Genki Takiuchi]
* {Groonga::Database#tables} ignores missing objects rather than raising
  an exception. Because the method is interested in only existing tables.

### Fixes

* Fixed a bug that {Groonga::Expression#parse} doesn't accept all
  mode and operators.

### Thanks

* SHIMADA Koji
* Genki Takiuchi

## 3.0.9: 2013-10-29 {#version-3-0-9}

### Improvements

* Supported Groonga 3.0.9.
* Supported showing error message on error in {Groonga::Object#remove}.

### Fixes

* Fixed a crash bug that is caused by assigning `nil` as source of
  index column.

## 3.0.8: 2013-09-29 {#version-3-0-8}

### Improvements

* Supported groonga 3.0.8.
* Supported x64-mingw32 Ruby.
* Supported Ruby 2.1.0 preview1.

## 3.0.7: 2013-09-19 {#version-3-0-7}

### Improvements

* Added {Groonga::Table::KeySupport#tokenize}. It requires groonga
  3.0.8. Groonga 3.0.8 is not released yet.
* Changed return object of {Groonga::Context#select} to groonga-client gem's
  return object. It is a backward imcompatible change. Some APIs are changed.
  For example, `drill_down` is renamed to `drilldowns`. See
  "Groonga::Client::Response::Select":http://rubydoc.info/gems/groonga-client/Groonga/Client/Response/Select
  about rerturn object details.
** Added groonga-client gem, groogna-command gem, gqtp gem dependencies.

### Fixes

* Fixed a bug that auto groonga installation is failed.
  [GitHub#12][Patch by Keita Haga]

### Thanks

* Keita Haga

## 3.0.6: 2013-09-13 {#version-3-0-6}

### Improvements

* Supported accessing reference column that referes a table that uses
  Int8/UInt8/Int16/UInt16 key.

## 3.0.5: 2013-07-29 {#version-3-0-5}

### Improvements

* [dumper] supported dumping of DoubleArrayTrie.
* Supported Int8/UInt8/Int16/UInt16 to Ruby conversion.
  [groonga-dev,01524][Reported by Masaharu YOSHIOKA]
* Added memory pool mechanism to reduce GC easily.
  {Groonga::Context#push_memory_pool} and {Groonga::Context#pop_memory_pool}
  are added.
  You can close temporary table objects automatically:

      context.push_memory_pool do
        # create tempoeray table objects by select, group, sort and so on...
      end
      # created tempoeray table objects are closed automatically

* Supported max int32 over Fixnum
  [Reported by xtuaok]

### Fixes

* [dumper] fixed a bug that no column table isn't dumped.

### Thanks

* Masaharu YOSHIOKA
* xtuaok

## 3.0.4: 2013-07-04 {#version-3-0-4}

### Fixes

* Fixed a rroogna 3.0.3 gem package for Windows.

## 3.0.3: 2013-07-04 {#version-3-0-3}

### Improvements

* Required groonga >= 3.0.5.
* Added an error check for creating a result table of {Groonga::Table#select}.
* Added {Groonga::Operator::AND_NOT}.
* Deprecated {Groonga::Operator::BUT} because groonga deprecated it.
  Use {Groonga::Operator::AND_NOT} instead.
* Added {Groonga::Array#unblock}.
* Added `:max_n_sub_records` option to {Groonga::Table#group}.
* Added {Groonga::Table#each_sub_record}.
* Supported groonga to Ruby conversion for vector value.
* Added `:reference => true` option to {Groonga::Expression#define_variable}
  that defines reference type variable.
* Added {Groonga::Record#sub_records} that returns sub records for the record.
  Sub records is a {Groonga::SubRecords} instance.

### Fixes

* Fixed {Groonga::Expression#[]} return type. It returns {Groonga::Variable}
  instead of value itself. CAUTION: It is a backward incompatible change.

## 3.0.2: 2013-05-29 {#version-3-0-2}

### Improvements

* Required groonga >= 3.0.4.
* Supported set/get a vector of Time.
* [grndump] Stopped to dump index only tables. They are needless.
* Added {Groonga::Record#to_json}.
* Added {Groonga::IndexColumn#add}.
* Added {Groonga::IndexColumn#delete}.
* Added {Groonga::IndexColumn#update}.
* Deprecated `Groonga::IndexColumn#[]=`. Use {Groonga::IndexColumn#add},
  {Groonga::IndexColumn#delete} or {Groonga::IndexColumn#update} instead.
* Added {Groonga::Table#have_n_sub_records_space?}.
* [grndump] Don't dump "register PLUGIN" when schema dump is disabled.

### Fixes

* [grndump]
  Fixed a bug that reference tables may be dumpped before referenced tables.

## 3.0.1: 2013-05-01 {#version-3-0-1}

### Improvements

* Required groonga >= 3.0.3.
* Supported assigning weight to value. See {Groonga::Table#set_column_value},
  {Groonga::Record#initialize} and {Groonga::Record#[]=} for details.
* Renamed to {Groonga::QueryLogger.path} from {Groonga::Logger.query_log_path}.
* Renamed to {Groonga::QueryLogger.path=} from {Groonga::Logger.query_log_path=}.
* Renamed to {Groonga::Logger.path} from {Groonga::Logger.log_path}.
* Renamed to {Groonga::Logger.path=} from {Groonga::Logger.log_path=}.
* Added missing "Packnga >= 0.9.7" requirement. [Reported by takkanm]

### Fixes

* Fixed a memory leak on error.

### Thanks

* takkanm

## 3.0.0: 2013-03-29 {#version-3-0-0}

### Improvements

* Required groonga >= 3.0.2.
* Added block support to {Groonga::Context#create_database}. If a
  block is given, created database is closed on block exit.
* [experimental] Added {Groonga::Array#push}.
* [experimental] Added {Groonga::Array#pull}.
* Added more closed object checks.
  [GitHub #8][Reported by KITAITI Makoto]
* Added block support to {Groonga::Context#restore}. If a block is
  given, command and its response are yielded.

### Thanks

* KITAITI Makoto

## 2.1.3: 2013-01-29 {#version-2-1-3}

### Improvements

* Removed Groonga::View removed in groonga 2.1.2.
* [doc] Added tutorial in English.
** for English:http://ranguba.org/rroonga/en/file.tutorial.html
** for Japanese:http://ranguba.org/rroonga/ja/file.tutorial.html
* [context] Added Context#restore. This method restores command dumped
  by "grndump" command. Please see example below:

      dumped_commands = File.read("dump.grn")
      context.restore(dumped_commands)

* Supported new logger API in groonga. Old API isn't used anymore.
* For installing groonga with this gem:
** Stopped to install documentation about groonga. See "Web site":http://groonga.org/docs/ instead.
** Stopped to build static library because it isn't used by rroonga.

### Fixes

* [dumper] Reduced needless new lines in dumped commands.

* For ranguba project:
** [template] Removed needless block for sponsor by rubyforge.
** [template] Removed needless piwik tag.
** [template] Fixed URLs of references in Japanese.

## 2.1.2: 2013-01-04 {#version-2-1-2}

### Fixes

* Fixed GC related crash. Table's domain and range are also marked.

## 2.1.1: 2012-12-29 {#version-2-1-1}

### Improvements

* Required groonga 2.1.1 because groonga 2.1.0 has a serious bug
  related to key normalization.

## 2.1.0: 2012-12-29 {#version-2-1-0}

### Improvements

* Required groonga 2.1.0.
* Supported mass record deletion with block.
  [groonga-dev,01138][Suggested by ongaeshi]
* Added Groonga::Normalizer.normalize (experimental). It normalize string.
  e.g.)

      Groonga::Normalizer.normalize("AbC") # => "abc"

  Now, it removes any spaces by default, but it will be customized soon.
* Supported :normalizer option in DoubleArrayTrie, PatriciaTrie, Hash,
  Schema when creating tables.
* [table] Added using normalizer accessor.
* [table] Used normalizer for checking key normalization is enabled or not.
* Added groonga-index-dump tool (experimental).
  This tool dumps infomations for each index from DB.
  Dumped informations are contained at "INDEX_NAME.dump" files in
  "TARGET_TABLE_NAME.index_INDEX_COLUMN_NAME".
  e.g.)

      $ cat index-dump.sdf
        table_create --name Video --flags TABLE_HASH_KEY --key_type UInt32
        table_create --name Tag --flags TABLE_HASH_KEY --key_type ShortText
        column_create --table Video --name title --flags COLUMN_SCALAR --type ShortText
        column_create --table Video --name tags --flags COLUMN_VECTOR --type Tag
        column_create --table Tag --name index_tags --flags COLUMN_INDEX --type Video --source tags
        load --table Video
        [
        {"_key":1,"title":"groonga Demo","tags":["IT","Server","groonga"]},
        {"_key":2,"title":"Ultra Baseball","tags":["Sports","Baseball"]},
        ]
      $ groonga --file index-dump.sdf -n index-dump.db
      $ groonga-index-dump --output-directory=path/to/index/ index-dump.db
      $ cd path/to/index/
      $ ls Tag.index_tags
        Baseball.dump  IT.dump       Server.dump  Sports.dump  groonga.dump
      $ cat path/to/index/Tag.index_tags/groonga.dump
        index: Tag.index_tags	term: <groonga>	domain: Tag	range: Video	have_section: false	have_weight: false	have_position: false
         weight	position        term_frequency	record
         0    2     1   Video[ 1 ].tags

* Added flag options to Groonga::IndexColumn#open_cursor.
  The flag options are :with_section, :with_weight, :with_position.
  They are enabled by default if each option is specified in creating
  a index column.
* [Snippet] Showed the error message when specified context wasn't
  associated with a database by Groonga::Database#open or #create.
* [inspect] Supported to display index column related flags for index
  column only.
  "index column related flags" are WITH_SECTION, WITH_WEIGHT and
  WITH_POSITION.
* [inspect] Added default tokenizer and normalizer infomation.
* Supported Groonga::QueryLogger. This class is used to log query log.
  Please see its reference for detail.

### Changes

* Move groonga-query-log-extract to groonga-query-log.
  Please install groogna-query-log gem to use this tool.
  how to install:

      gem install groonga-query-log

* Returned Groonga::Array instead of Array by Table#sort.
  [GitHub: #8][Suggested by Genki Takiuchi]
  CAUTION: This is backward incompatible change. You need to use
  record.value to get the original record.
  The following code shows how to use old style:

      result_since_2_1_0 = table.sort(["sort_key"])
      result_before_2_1_0 = result_since_2_1_0.collect do |record|
        record.value
      end

### Thanks

* ongaeshi
* Genki Takiuchi

## 2.0.8: 2012-12-02 {#version-2-0-8}

### Improvements

* Required groonga 2.0.9.

## 2.0.7: 2012-11-29 {#version-2-0-7}

### Improvements

* Added `Groonga::Posting#record`. This method returns the record for
  the record ID in table.
* Added `Groonga::Posting#term`. This method returns the record for the
  term ID in table.
* Supported `GRN_OBJ_KEY_VAR_SIZE` for `Groonga::Type#inspect`.
  `GRN_OBJ_KEY_CAR_SIZE` specifies its column is variable size.
* [Type] Added reader methods to `Groonga::Type` (`#size` and `#flags`).
* [Type] Added predicate methods to `Groonga::Type`.
  Added methods are `#fixed_size?`, `#variable_size?`, `#unsigned_integer?`,
  `#integer?`, `#float?` and `#geo_point?`.

### Changes

* Removed query log related codes.
  This feature moved to groonga-query-log gem and it is relased soon.

## 2.0.6: 2012-10-25 {#version-2-0-6}

### Improvements

* [dump] Put index columns at the bottom (after loads).
  It is for using offline index construction.
* Added term_extract in Table#select by `table.select {|record|
  record.key.term_extract(text)}` syntax.
* Supported `:allow_leading_not` as a option of `#select`.
  You should use this option carefully. It takes a long time to search
  all records which doesn't include a word. In addition, when the
  number of records is large, to search them with this option is
  slowly.

### Changes

* Changed option name for debug build in extconf.rb from `--with-debug`
  to `--enable-debug-log`.

### Fixes

* Fixed display collapse in the references.

## 2.0.5: 2012-09-28 {#version-2-0-5}

### Improvements

* Supported groonga 2.0.7.
* Removed the workaround to install rroonga with old groonga.
* Added more error checks.
* Added Database#tables. [Suggested by @temitan]
* Supported Enumerator for #each on Database, Table, TableCursor and
  IndexCursor.
* Added WGS84GeoPoint and TokyoGeoPoint.
* [dumper] supported dumping of WGS84GeoPoint and TokyoGeoPoint.
* [dumper] worked on non UTF-8 extenal output encoding environment.
* [dumper] sorted table and column names.
* [dumper] ignored invalid byte.
* [grndump] showed the warning for invalid character.
* [grndump] supported database that is created by old groonga.
* Added `Groonga::Context#ruby_encoding`.
* Added many documents in codes but showed no references.

### Fixes

* Added missing backslash escape for groonga command.

### Thanks

* @temitan

## 2.0.4: 2012-05-02 {#version-2-0-4}

### Fixes

* Fixed a bug that weight of match target is ignored.

## 2.0.3: 2012-05-02 {#version-2-0-3}

### Improvements

* Supported groonga 2.0.2.
* `Groonga::Table#each` supports options that are same as
  `Groonga::Table#open_cursor`'s one.
* [grndump] added `--order-by=id` option. With this option, dumped
  records are sorted by ID instead of key. You can restore records
  without ID change if you don't delete any records. [#1341]
* Supported building on Windows with RubyInstaller for Windows with DevKit.
  [GitHub#6] [Patch by @ongaeshi]
* Supported similar search by `table.select {|record|
  record.column.similar_search(text)}` syntax.

### Fixes

* Fixed a GC related crach bug.

### Thanks

* @ongaeshi

## 2.0.2: 2012-03-29 {#version-2-0-2}

### Improvements

* Supported groonga 2.0.1.
* Added "logos":http://groonga.rubyforge.org/#logo .

### Fixes

* Fixed a Groonga::Snipet related crach bug caused by GC.

## 2.0.0: 2012-03-22 {#version-2-0-0}

### Improvements

* Supported groonga 2.0.0.
* [gem][windows] Removed mswin packages.

### Fixes

* [test] Fixed version test failure. [GitHub#4] [Reported by @takkanm]
* Fixed a Groonga::Expression related crach bug caused by GC.
* [doc] Fixed broken HTML output. [groonga-dev,00699]
  [Reported by Hirano]

### Thanks

* @takkanm
* Hirano

## 1.3.1: 2012-01-29 {#version-1-3-1}

### Improvements

* Supported groonga 1.3.0.
* [schema] Supported Int8, Int16, UInt8 and UInt16.
* [schema] Supported TokyoGeoPoint and WGS84GeoPoint.
* [schema][dumper] Supported Boolean and more built-in types.
  [Reported by @mashiro]
* [schema] Supported type object as column type. [#1002]
* Added `Groonga::VariableSizeColumn#compressed?`. [#1004]
* Added `Groonga::Record#score=`.
* Improve performance for encoded string.
* Added `Groonga::Command::Builder.escape_value`.

### Thanks

* @mashiro

## 1.3.0: 2011-11-29 {#version-1-3-0}

### Improvements

* [schema] Remove also needless db.tables/ directory if it is empty.
* [schema] Remove also needless db.tables/table.columns/ directory if it is empty.
* Added query log parser.
* Added groonga-query-log-extract command.
* Added grntest log analyzer.
* Added JSON gem dependency.
* Supported groonga 1.2.8.
* Dropped groonga 1.2.7 or former support.
* Added Groonga::Table#defrag.
* Added Groonga::Table#rename.
* Added Groonga::Column#rename.
* Added Groonga::DoubleArrayTrie.
* [schema] Supported table rename.
* [schema] Supported column rename.
* [schema] Supported double array trie.

### Changes

* [schema] Don't use named path by default for location aware DB.

### Fixes

* Fixed a crash problem on GC.

## 1.2.9: 2011-09-16

### Fixes

* deleted unneed object files.

## 1.2.8: 2011-09-16

### Improvements

* supported `!=` expression for column in block of `Groonga::Table#select`.
* accepted Hash like object as options.
* supported vector in dump in Ruby syntax.
* supported `GRN_CTX_PER_DB` environment variables.
  (NOTE: You should pay attention to use this variables.)

## 1.2.7: 2011-08-29

### Improvements

* Added Groonga::Snippet#close that frees resource.

### Fixes

* Fixed build error on Ruby 1.8.7.

## 1.2.6: 2011-08-29

### Improvements

* Supported groonga 1.2.5.
* Added Groonga::Record#added? that returns true when the record is just added.
* Added Groonga::VariableSizeColumn#defrag? that defrags the column.
* Added Groonga::Database#defrag that defrags the all variable size columns.
* Supported column name specification by symbol.

### Fixes

* Fixed install *.rb failure by gem install.
* Fixed some memory leaks.
* Fixed crash bug on exit.

## 1.2.5: 2011-08-05

### Improvements

* Re-supported tar.gz distribution.
* Added Groonga::Context#close.
* Added Groonga::Context#closed?.
* Deprecated Groonga::ObjectClosed. Use Groonga::Closed instead.
* grndump: Added --exclude-table option that specifies not dumped tables.
* dump: Removed path equality check.

### Fixes

* dump: Fixed wrong index table type.
* Re-supported auto groonga install.

## 1.2.4: 2011-06-29

### Improvements

* Supported groonga 1.2.3.

### Fixes

* Re-supported auto groonga install.
* Added missing pkg-config gem dependency.

## 1.2.3: 2011-06-27

### Fixes

* remove object files in gem packages.
* fix charactor corruption in reference.

## 1.2.2: 2011-06-27

### Improvements

* created "Developers" page in English.
* added description for tasks of "html:publish" and "publish".

### Changes

* Groonga::Record#attributes return same attributes object for duplicate records.
* added document for Groonga::Record#attributes.
* changed tool name in document page for creating document.
* moved NEWS*.rdoc and tutorial.texttile to doc/text/.

### Fixes

* fixed the tutorial path in index page.
* fixed the path of tutorial in index page in English.
* follow the groonga downlowd URL change. [mallowlabs]

### Thanks

* mallowlabs

## 1.2.1: 2011-06-07

### Improvements

* added document of Groonga::Table#pagination.
* added grndump in package.
* corresponded recursive reference of Records by Groonga::Record#attributes.
  (experimental) [mooz]
* Groonga::Record#attributes supported data including _score.
* corresponded Windows for 64-bit.
  (but there's not 64-bit ruby, so rroonga for 64-bit windows cannot run.)
* added Groonga::Posting.
* added :delimit, :token_delimiter for alias of TokenDelimit.
* Groonga::DatabaseDumper#dump corresponded lexicon table.
* Groonga::DatabaseDumper#dump corresponded data including plugin.
* added Groonga::IndexColumn#open_cursor. [yoshihara]
* added Groonga::IndexCursor. [yoshihara]
* added Groonga::Object#builtin?. [yoshihara]

### Changes

* check existence of column before removing it.
* removed grn expression document.

### Thanks

* mooz
* yoshihara

## 1.2.0: 2011-04-01

### Improvements

* Supported groonga 1.2.0.
* Added `Groonga::Accessor#local_name`.
* Added `Groonga::IndexColumn#with_section?`.
* Added `Groonga::IndexColumn#with_weight?`.
* Added `Groonga::IndexColumn#with_position?`.
* `Groonga::Schema.dump` supported groonga command format dump.
* Added grndump command.
* `Groonga::Database#each` supports order customize.
* Added `Groonga::Context#match_escalation_threshold`.
* Added `Groonga::Context#match_escalation_threshold=`.
* Improved error message.
* Supported Rubyish name like `:short_text` instead of the
  official type name like "ShortText" in `Groonga::Schema`.

## 1.1.0: 2011-02-09

### Improvements

* Supported groonga 1.1.0.
* Added Groonga::Plugin.register.

## 1.0.9: 2011-01-29

### Improvements

* Supported gem creation on Windows. [Patch by ongaeshi]
* Supported generated directory that is created by Groonga::Schema removal
  when table or column is removed.
* Added Groonga::Context#create_database.
* Added Groonga::Context#open_database.
* Added Groonga::Column#indexes.
* Supported a notation for specifying index column as match target in
  Groonga::Table#select:

      table.select do |record|
        record.match("query") do |match_record|
          (match_record.index("Terms.title") * 1000) |
            (match_record.index("Terms.description") * 100)
            match_record.content
        end
      end

* Supported prefix search in Groonga::Table#select:

      table.select do |record|
        record.name.prefix_search("groo")
      end

* Supported suffix search in Groonga::Table#select:

      table.select do |record|
        record.name.suffix_search("nga")
      end

* Supported :default_tokenizer schema dump.
* Supported :key_normalize schema dump.
* Supported pseudo columns by Groonga::Table#have_column?.
* Supported pseudo columns by Groonga::Record#have_column?.

### Changes

* Renamed Groonga::Operatoion to Groonga::Operator.
  (Groonga::Operation is deprecated but still usable.)

### Fixes

* Fixed a crash bug when not default Groonga::Context is used in
  Groonga::Table#select.
* Fixed a crash bug when an exception is occurred.

### Thanks

* ongaeshi

## 1.0.8: 2010-12-25

### Improvements

* Improved Groonga::Schema's n-gram tokenizer detect process.

### Fixes

* Fixed GC problem caused by match_target in select.

## 1.0.7: 2010-12-19

### Fixes

* Supported pkg-config installed by RubyGems on Ruby 1.8. [Reported by @kamipo]
* Fixed a memory leak in Groonga::Table#columns.

### Thanks

* @kamipo

## 1.0.5: 2010-11-29

### Improvements

* Added snail_case type name aliases for built-in groonga types
  to Groonga::Schema.

### Fixes

* Fixed a crash bug on GC. [Ryo Onodera]

## 1.0.4: 2010-11-29

### Improvements

* Supported groonga 1.0.4.
  * Added Groonga::UnsupportedCommandVersion.
* Added Groonga::Record#support_sub_records?.
* Added Groonga::Record#eql?とGroonga::Record#hash.
  (treat two the same table and the same record ID object as the same Hash key.)
* Supported pkg-config gem.
* Supported generic #record_id object handle for custom record object
  in Groonga::Table#select.
* Added Groonga::Record#record_id.
* Added Groonga::Table#support_key?.
* Added Groonga::Record#support_key?.
* Added Groonga::Record#support_key?.
* Added Groonga::Column#reference_key?.
* Added Groonga::Column#index_column?.
* Added Groonga::Schema#dump.
* Supported multi columns index creation in Groonga::Schema.
* Supported meaningful path in Groonga::Schema.
* Made reference table omissible when index column definition
  in Groonga::Schema.
* Added Groonga::Schema.remove_column.
* Added convenience timestamps methods to define "created_at" and
  "updated_at" columns in Groonga::Schema.
* Added Groonga::Context#support_zlib?.
* Added Groonga::Context#support_lzo?.
* Added Groonga::Database#touch.
* Added Groonga::Table#exist?.
* Added Groonga::Record#valid?.
* Added Groonga::Column#vector?.
* Added Groonga::Column#scalar?.
* Added Groonga::Record#vector_column?.
* Added Groonga::Record#scalar_column?.
* Accepted any object that has record_raw_id method for record ID required
  location. Groonga::Record isn't required.
* Added Groonga::Record#record_raw_id.
* Accepted any object that as to_ary method for reference vector column value.

## Changes

* Used :key as the default value of `:order_by` of
  `Groonga::PatriciaTrie#open_cursor`.
* Removed a deprecated Groonga::Table::KeySupport#find.
* Used ShortText as the default key type of
  Groonga::Hash#create and Groonga::PatriciaTrie#create.
* Renamed Groonga::Schema#load to Groonga::Schema#restore.
* Supported pkg-config 1.0.7.
* Added Groonga::Column#index? and deprecated Groonga::Column#index_column?.
* Added Groonga::Column#reference? and deprecated
  Groonga::Column#reference_column?.

### Fixes

* Fixed index for key isn't be able to define.
* Fixed a crash problem on GC.

## 1.0.1: 2010-09-12

### Fixes

* Fixed wrong flag used on creating a table. [Reported by ono matope]

### Thanks

* ono matope

## 1.0.0: 2010-08-29

* Supported groonga 1.0.0.
  * Added Groonga::CASError.
  * Added :order_by option to Groonga::Table#open_cursor.
  * Added Groonga::PatriciaTrie#open_prefix_cursor that creates a cursor
    to retrieve each records by prefix search.
  * Added Groonga::PatriciaTrie#open_rk_cursor that creats a cursor to
    retrieve katakana keys from roman letters and/or hiragana.
  * Added Groonga::PatriciaTrie#open_near_cursor that creates a cursor to
    retrieve records order by distance from key.
* Supported _key as index source.

## 0.9.5: 2010-07-20

* Supported groonga 0.7.4.
* Imporoved Groonga::Table#select:
  * Supported weight match:

    Here is an example to match source column or title column and
    title column has high score:

        table.select do |record|
          (record.title * 10 | record.source) =~ "query"
        end

  * Supported and representation for and conditions:

    Here are examples that represents the same condition:

        table.select do |record|
          conditions = []
          conditions << record.title =~ "query"
          conditions << record.updated_at > Time.parse("2010-07-29T21:14:29+09:00")
          conditions
        end

        table.select do |record|
          (record.title =~ "query") &
            (record.updated_at > Time.parse("2010-07-29T21:14:29+09:00"))
        end

* Provided groonga runtime version: Groonga::VERSION
* Added Groonga::Table#support_sub_records?
* Supported pagination: Groonga::Table#paginate, Groonga::Pagination

## 0.9.4: 2010-04-22

* Fixed release miss.

## 0.9.3: 2010-04-22

* Fixed release miss.

## 0.9.2: 2010-04-22

* Supported groonga 0.1.9.
* Many.

## 0.9.1: 2010-02-09

* Supported groonga 0.1.6

## 0.9.0: 2010-02-09

* Supported groonga 0.1.5
* Added API
  * Groonga::Object#context
  * Groonga::Record#n_sub_records
  * Groonga::Context#send
  * Groonga::Context#receive
  * Groonga::PatriciaTrie#prefix_search [Tasuku SUENAGA]
  * Groonga::Object#path [Ryo Onodera]
  * Groonga::Object#lock [Tasuku SUENAGA]
  * Groonga::Object#unlock [Tasuku SUENAGA]
  * Groonga::Object#locked? [Tasuku SUENAGA]
  * Groonga::Object#temporary?
  * Groonga::Object#persistent?
  * Groonga::ObjectClosed
  * Groonga::Context.[]
  * Groonga::Table#column_value
  * Groonga::Table#set_column_value
* Changed API
  * Groonga::Table#select, Groonga::Column#select
    * They also accept Groonga::Expression
    * Added :syntax option that specifies grn expression syntax
  * Groonga::Table#open_cursor
    * Added :offset option that specifies offset.
    * Added :limit option that specifies max number of records.
  * Changed Groonga::Expression.parse options:
    * (nil (default) -> :column) -> (nil (default) -> :query)
    * :column -> removed
    * :table -> :query
    * :table_query -> :query
    * :expression -> :script
    * :language -> :script
  * Groonga::Table#define_column, Groonga::Table#define_index_column
    * Defined column becomes persistent table by default
  * Groonga::Table#[] -> Groonga::Table#value
  * Groonga::Table#[]= -> Groonga::Table#set_value
  * Groonga::Table#find -> Groonga::Table#[]
  * Groonga::Table#find -> obsolete
  * Groonga::Table#[]= -> removed
  * Groonga::Table::KeySupport#[]= is alias of Groonga::Table::KeySupport#add
  * Changed exception class to Groonga::NoSuchColumn from
    Groonga::InvalidArgument when Groonga::Record accesses nonexistent
    a column.
* Bug fixes
  * Fixed a bug that context isn't passed to schema [dara]
  * Fixed a bug that Groonga::PatriciaTrie#tag_keys doesn't return
    that last text. [Ryo Onodera]
* Added --with-debug option to extconf.rb for debug build.
* Fixed a bug that Ruby 1.9.1 may fail extconf.rb.

### Thanks

* dara
* Ryo Onodera
* Tasuku SUENAGA

## 0.0.7: 2009-10-02

* Supported groonga 0.1.4
* Added API
  * Groonga::PatriciaTrie#scan
  * Groonga::PatriciaTrie#tag_keys
  * Groonga::Expression#snippet
  * Groonga::Object#append
  * Groonga::Object#prepend

## 0.0.6: 2009-07-31

* Supported groonga 0.1.1.
* Fixed documents [id:mat_aki]
* Supported groonga expression for searching.
* Added API
  * Groonga::Table#union!
  * Groonga::Table#intersect!
  * Groonga::Table#differene!
  * Groonga::Table#merge!
* Provided tar.gz [id:m_seki]
* Fixed memory leaks

## 0.0.3: 2009-07-18

* Added Groonga::Table::KeySupport#has_key? [#26145] [Tasuku SUENAGA]
* Groonga::Record#[] raises an exception for nonexistent
  column name. [#26146] [Tasuku SUENAGA]
* Supported 32bit environment [niku]
* Added a test for N-gram index search [dara]
* Added APIs
  * Groonga::Record#incemrent!
  * Groonga::Record#decemrent!
  * Groonga::Record#lock
  * Groonga::Table#lock
  * Groonga::Schema: A DSL for schema definition
  * Groonga::Expression

## 0.0.2: 2009-06-04

* Supported groonga 0.0.8 [mori]
* Improved preformance: cache key, value, domain and range
* Improved API
* Added documents
* Supported Ruby 1.9
* Bug fixes:
  * Fixed install process [Tasuku SUENAGA]
  * Fixed memory leaks

## 0.0.1: 2009-04-30

* Initial release!
