# -*- coding: utf-8 -*-

require "benchmark"
require "shellwords"
require "optparse"

require "groonga"

Groonga::Logger.query_log_path = "/tmp/query.log"

module ColumnTokenizer
  def tokenize_column_list(column_list)
    tokens = column_list.split(/[\s,]/)
    tokens.reject!(&:empty?)
    tokens.select! do |token|
      token == "*" || token =~ /[A-Za-z0-9_]/
    end
    tokens.each do |token|
      unless token == "*"
        token.sub!(/[^A-Za-z0-9_]\z/, "")
      end
    end
  end
end

class Query
  attr_reader :options
  attr_accessor :original_log_entry
  def initialize(options)
    @options = options
  end

  def table_name
    @options[:table]
  end

  def match_columns
    #raise "unsupported: #{@options[:match_columns].inspect}" if @options[:match_columns] =~ /\b/ # XXX

    @options[:match_columns]
  end

  def filter
    if match_columns and @options[:query]
      #raise "unsupported" if @options[:filter]
      @options[:query]
    else
      @options[:filter]
    end
  end

  def limit
    if @options[:limit]
      @options[:limit].to_i
    else
      nil
    end
  end

  def offset
    if @options[:offset]
      @options[:offset].to_i
    else
      nil
    end
  end

  def sort_by
    if @options[:sortby]
      @options[:sortby]
    else
      nil
    end
  end

  def output_columns
    if @options[:output_columns]
      @options[:output_columns]
    else
      nil
    end
  end

  def drilldown_columns
    if @options[:drilldown]
      @options[:drilldown]
    else
      nil
    end
  end

  def drilldown_limit
    if @options[:drilldown_limit]
      @options[:drilldown_limit].to_i
    else
      nil
    end
  end

  def drilldown_offset
    if @options[:drilldown_offset]
      @options[:drilldown_offset].to_i
    else
      nil
    end
  end

  def drilldown_sort_by
    if @options[:drilldown_sortby]
      @options[:drilldown_sortby]
    else
      nil
    end
  end

  def drilldown_output_columns
    if @options[:drilldown_output_columns]
      @options[:drilldown_output_columns]
    else
      nil
    end
  end

  def parameters
    @options.dup.tap do |options|
      options.delete(:table)
    end
  end

  class GroongaLogParser
    def initialize(log)
      @log = log
      @tokens = []
      @parameter_list = []
      @parameters = {}
    end

    def parse
      tokenize
      build_parameter_list
      build_parameters
      create_query
    end

    class << self
      def parse(log)
        new(log).parse
      end
    end

    private
    def next_token(token)
      @tokens << token
      ""
    end

    def tokenize
      escape = nil
      litaral = nil
      token = ""

      @tokens = Shellwords.split(@log)
      @tokens = @tokens.reject(&:empty?)
    end

    IMPLICIT_PARAMETER_ORDER = [
      :table,
      :match_columns,
      :query,
      :filter,
      :scorer,
      :sortby,
      :output_columns,
      :offset,
      :limit,
      :drilldown,
      :drilldown_sortby,
      :drilldwon_output_columns,
      :drilldown_offset,
      :drilldown_limit,
      :cache,
      :match_escalation_threshold,
    ]

    NAMED_PARAMETER_PREFIX = /\A--/

    def build_parameter_list
      command, parameter_tokens = @tokens.shift, @tokens
      raise "command is not \"select\": #{command.inspect}" unless command == "select"

      parameter_name = nil
      parameter_tokens.each do |token|
        if token =~ NAMED_PARAMETER_PREFIX
          raise "bad" unless parameter_name.nil?
          parameter_name = token
        elsif parameter_name
          @parameter_list << [parameter_name, token]
          parameter_name = nil
        else
          @parameter_list << token
        end
      end
    end

    def build_parameters
      index = 0
      @parameter_list.each do |parameter|
        case parameter
        when Array
          name, value = parameter
          @parameters[to_parameter_symbol(name)] = value
        else
          @parameters[IMPLICIT_PARAMETER_ORDER[index]] = parameter
          index += 1
        end
      end
    end

    def to_parameter_symbol(name)
      name.sub(NAMED_PARAMETER_PREFIX, "").to_sym
    end

    def create_query
      query = Query.new(@parameters)
      query.original_log_entry = @log
      query
    end
  end

  class << self
    def parse_groonga_query_log(log)
      GroongaLogParser.parse(log)
    end
  end
end

class Configuration
  attr_accessor :database_path
end

class Selector
  attr_reader :context, :database_path
  def initialize(context, database_path)
    @context = context
    @database_path = database_path
    @database = @context.open_database(@database_path)
  end

  def select(query)
    raise "implement"
  end
end

class SelectorByCommand < Selector
  def select(query)
    parameters = query.parameters.merge(:cache => :no)
    parameters[:sortby] ||= :_id
    parameters[:drilldown_sortby] ||= :_key
    result = @context.select(query.table_name, parameters)
    CommandResult.new(result)
  end
end

class SelectorByMethod < Selector
  include ColumnTokenizer

  def select(query)
    table = @context[query.table_name]
    filter = query.filter
    if query.match_columns
      default_column = table.column(query.match_columns)
    end

    result = do_select(filter, table, default_column)
    sorted_result = sort(query, result)
    formatted_result = format(query, sorted_result || result)
    drilldown_results = drilldown(query, result)

    MethodResult.new(result, sorted_result, formatted_result, drilldown_results)
  end

  def do_select(filter, table, default_column)
    if filter
      options = {
        :syntax => :script
      }
      if default_column
        options[:default_column] = default_column
        options[:syntax] = :query
      end

      table.select(filter, options)
    else
      table.select
    end
  end

  DEFAULT_LIMIT = 10
  DEFAULT_DRILLDOWN_LIMIT = DEFAULT_LIMIT

  def sort(query, result)
    if needs_sort?(query)
      sort_key = sort_key(query.sort_by)
      limit = query.limit || DEFAULT_LIMIT
      offset = query.offset

      window_options = create_window_options(limit, offset)
      sorted_result = result.sort(sort_key, window_options).collect do |record|
        record.key
      end
    end
  end

  def drilldown_sort(query, result)
    sort_key = sort_key(query.drilldown_sort_by || "_key")
    limit = query.drilldown_limit || DEFAULT_DRILLDOWN_LIMIT
    offset = query.drilldown_offset

    window_options = create_window_options(limit, offset)

    sorted_result = result.sort(sort_key, window_options).collect do |record|
      record
    end
  end

  DEFAULT_OUTPUT_COLUMNS = "_id, _key, *"
  DEFAULT_DRILLDOWN_OUTPUT_COLUMNS = "_key, _nsubrecs"

  def format(query, result)
    columns = query.output_columns || DEFAULT_OUTPUT_COLUMNS
    format_result(result, columns)
  end

  def drilldown_format(query, result)
    columns = query.drilldown_output_columns || DEFAULT_DRILLDOWN_OUTPUT_COLUMNS
    format_result(result, columns)
  end

  def drilldown(query, result)
    if needs_drilldown?(query)
      drilldown_results = drilldown_result(result, query.drilldown_columns, query)
    end
  end

  def drilldown_result(result, drilldown_columns, query)
    columns = tokenize_column_list(drilldown_columns).uniq
    columns.collect do |column|
      drilldown_result = do_group(result, column)
      sorted_drilldown_result = drilldown_sort(query, drilldown_result)
      formatted_drilldown_result = drilldown_format(query, sorted_drilldown_result || drilldown_result)

      {
        :column => column,
        :result => drilldown_result,
        :sort => sorted_drilldown_result,
        :format => formatted_drilldown_result,
      }
    end
  end

  def do_group(result, column)
    result.group(column)
  end

  def needs_sort?(query)
    (query.limit.nil? or query.limit >= 0) or query.offset or query.sort_by
  end

  def needs_drilldown?(query)
    query.drilldown_columns
  end

  DESCENDING_ORDER_PREFIX = /\A-/
  def sort_key(sort_by)
    if sort_by
      build_sort_key(sort_by)
    else
      default_sort_key
    end
  end

  def build_sort_key(sort_by)
    tokens = tokenize_column_list(sort_by)

    tokens.collect do |token|
      key = token.sub(DESCENDING_ORDER_PREFIX, "")
      if token =~ DESCENDING_ORDER_PREFIX
        descending_order_sort_key(key)
      else
        ascending_order_sort_key(key)
      end
    end
  end

  def descending_order_sort_key(key)
    {
      :key => key,
      :order => "descending",
    }
  end

  def ascending_order_sort_key(key)
    {
      :key => key,
      :order => "ascending",
    }
  end

  def default_sort_key #XX use #ascending_order_sort_key("_id")
    [
      {
        :key => "_id",
        :order => "ascending",
      }
    ]
  end

  def create_window_options(limit, offset)
    window_options = {}
    if limit
      window_options[:limit] = limit
    end
    if offset
      window_options[:offset] = offset
    end
    window_options
  end

  def access_column(table, column)
    columns = column.split(".")
    columns.each do |name|
      table = table.column(name).range
    end
    table
  end

  def format_result(result, output_columns)
    if result.empty?
      return []
    end

    column_tokens = tokenize_column_list(output_columns)
    column_list = build_column_list(result, column_tokens)

    result.collect do |record|
      format_record(column_list, record)
    end
  end

  def format_record(column_list, record)
    column_list.collect do |column, access_column|
      value = record[column]
      to_json(value, access_column)
    end
  end

  def to_json(value, column)
    case value
    when ::Time
      value.to_f
    when nil
      if column.name =~ /Int/
        0
      else
        ""
      end
    when Groonga::Record
      value["_key"]
    when Array
      value.collect do |element|
        to_json(element, value)
      end
    else
      value
    end
  end

  def column_included_in_record?(column, record)
    if record.respond_to?(:have_column?)
      record.have_column?(column)
    else
      record.include?(column)
    end
  end

  def build_column_list(result, columns)
    access_table = result.first.table

    table = result.first.key
    unless table.is_a?(Groonga::Table)
      table = result.first.table
    end
    columns = columns.collect do |column|
      if column == "*"
        table.columns.collect(&:name).collect do |name|
          name.sub(/\A[A-Za-z0-9_]+\./, "")
        end
      else
        column if column_included_in_record?(column, result.first)
      end
    end.flatten.compact

    columns.collect do |column|
      [column, access_column(access_table, column)]
    end
  end
end

class Result
  def ==(other) # XXX needs more strict/rigid check
    results = [
      hit_count == other.hit_count,
      result_count == other.result_count,
      formatted_result == other.formatted_result,
      drilldown_results == other.drilldown_results,
    ]

    results.all?
  end
end

class CommandResult < Result
  def initialize(result)
    @result = result
  end

  def hit_count
    @result.n_hits
  end

  def result_count
    @result.records.size
  end

  def formatted_result
    @result.values
  end

  def drilldown_results
    @result.drill_down.values.collect(&:values)
  end
end

class MethodResult < Result
  def initialize(result, sorted_result, formatted_result, drilldown_results)
    @result = result
    @sorted_result = sorted_result
    @formatted_result = formatted_result
    @drilldown_results = drilldown_results
  end

  def hit_count
    @result.size
  end

  def result_count
    sorted_result.size
  end

  def formatted_result
    @formatted_result
  end

  def drilldown_results
    if @drilldown_results.nil?
      []
    else
      @drilldown_results.collect do |result|
        result[:format]
      end
    end
  end

  private
  def sorted_result
    @sorted_result || @result
  end
end

class BenchmarkResult
  attr_accessor :result
  attr_accessor :profile
  attr_reader :benchmark_result

  class Time < BenchmarkResult
    def initialize(profile, target_object, query, &block)
      @intercepted_method_times = {}
      @profile = profile
      @target_object = target_object
      @query = query
      each_intercepted_methods(@profile.intercepted_methods) do |klass, method_name, depth|
        intercept_method(klass, method_name, depth)
      end

      measure_time(&block)

      each_intercepted_methods(@profile.intercepted_methods) do |klass, method_name, depth|
        reset_intercepted_method(klass, method_name, depth)
      end
    end

    def lines
      super + intercepted_method_lines
    end

    def padding(depth)
      "  " * (depth + 1)
    end

    def intercepted_method_lines
      lines = []
      @intercepted_method_times.each do |method_name, status|
        depth = status[:depth]
        count = status[:benchmark_results].size
        results = status[:benchmark_results]

        if count == 1
          result = results.first
          lines << single_line(method_name, result, depth)
        elsif count == 0
          # do nothing
        else
          total = results.inject do |result, _total|
            result + _total
          end

          total_result = ["#{padding(depth)}#{method_name}", total]
          lines << total_result

          lines += multile_lines(method_name, results, depth + 1)
        end
      end

      lines
    end

    def single_line(method_name, result, depth)

      ["#{padding(depth)}#{method_name}", result]
    end

    def multile_lines(method_name, results, depth)
      index = 0

      results.collect do |result|
        index += 1
        if @profile.respond_to?(:guess_invocation_label)
          label = @profile.guess_invocation_label(@query, method_name, index)
        end
        label ||= index

        ["#{padding(depth)}#{label}", result]
      end
    end

    def +(other)
      intercepted_method_times = other.instance_variable_get(:@intercepted_method_times)
      intercepted_method_times.each do |method_name, time|
        time[:benchmark_results].each_with_index do |result, index|
          @intercepted_method_times[method_name][:benchmark_results][index] += result
        end
      end
      super(other)
    end

    private
    def measure_time
      @benchmark_result = Benchmark.measure do
        @result = yield
      end
    end

    def each_intercepted_methods(intercepted_methods, depth=0, &block)
      intercepted_methods.each do |method|
        case method
        when Symbol
          yield(@target_object.class, method, depth)
        when Method
          if method.receiver.is_a?(Class)
            yield(method.owner, method.name, depth)
          else
            yield(method.receiver.class, method.name, depth)
          end
        when Array
          each_intercepted_methods(method, depth + 1, &block)
        else
          raise "bad"
        end
      end
    end

    def intercept_method(klass, method_name, depth)
      intercepted_method_times = @intercepted_method_times
      original_method_name = original_method_name(method_name)

      intercepted_method_times[method_name] = {}
      intercepted_method_times[method_name][:benchmark_results] = []
      intercepted_method_times[method_name][:depth] = depth

      klass.class_exec do
        alias_method original_method_name, method_name
        define_method method_name do |*arguments, &block|
          returned_object = nil
          benchmark_result = Benchmark.measure do
            returned_object = __send__(original_method_name, *arguments, &block)
          end
          intercepted_method_times[method_name][:benchmark_results] << benchmark_result
          returned_object
        end
      end
    end

    def reset_intercepted_method(klass, method_name, depth)
      original_method_name = original_method_name(method_name)

      klass.class_exec do
        alias_method method_name, original_method_name
      end
    end

    def original_method_name(method_name)
      :"__intercepted__#{method_name}"
    end
  end

  def lines
    [["#{name} (#{result.hit_count} hits)", @benchmark_result]]
  end

  def name
    profile.name
  end

  def +(other)
    @benchmark_result += other.benchmark_result
    self
  end
end

class Profile
  include ColumnTokenizer

  attr_accessor :mode
  attr_reader :name, :intercepted_methods
  def initialize(name, selector, intercepted_methods=[])
    @name = name
    @selector = selector
    @intercepted_methods = intercepted_methods
  end

  def take_benchmark(query)
    if mode == :measure_time
      measure_time(query)
    else
      raise "bad"
    end
  end

  def guess_invocation_label(query, method_name, index)
    if method_name.to_s =~ /drilldown|do_group/
      columns = tokenize_column_list(query.drilldown_columns).uniq
      columns[index - 1]
    else
      raise "bad: #{method_name}"
    end
  end

  private
  def measure_time(query)
    BenchmarkResult::Time.new(self, @selector, query) do
      result = @selector.select(query)
      result
    end
  end
end

class BenchmarkRunner
  attr_accessor :context
  DEFAULT_MODE = :measure_time # :mesure_memory, :mesure_io, :mesure_???

  def initialize(options={})
    @options = options
    @profiles = []
    @queries = []
  end

  def benchmark_mode
    @options[:mode] || DEFAULT_MODE
  end

  def add_profile(profile)
    profile.mode = benchmark_mode
    @profiles << profile
  end

  def add_query(query, label=nil)
    @queries << [query, label]
  end

  LOCK_TIMEOUT_SECONDS = 10
  def lock
    @context.database.lock(:timeout => LOCK_TIMEOUT_SECONDS * 1000) do
      yield
    end
  end

  def collect_benchmarks(query)
    lock do
      @profiles.collect do |profile|
        profile.take_benchmark(query)
      end
    end
  end

  def debug_benchmarks(query, benchmarks)
    if ENV["DEBUG"]
      pp query
      pp benchmarks
    end
  end

  def run_once(query)
    benchmarks = do_run_once(query)
    report_benchmarks(benchmarks, query)
  end

  def do_run_once(query)
    benchmarks = collect_benchmarks(query)

    debug_benchmarks(query, benchmarks)
    verify_results(benchmarks)

    benchmarks
  end

  def report_benchmarks(benchmarks, query, label)
    report = create_report(benchmarks, query, label)
    report.print
  end

  DEFAULT_REPEAT_COUNT = 5
  def repeat_count
    @options[:repeat_count] || DEFAULT_REPEAT_COUNT
  end

  def run(query=nil)
    if query
      do_run(query)
    else
      raise "no query" if @queries.empty?

      index = 0
      @queries.each do |query, label|
        index += 1
        do_run(query, "#{index}. #{label}")
        puts
        puts
      end
    end
  end

  def do_run(query, label=nil)
    benchmarks_set = repeat_count.times.collect do
      do_run_once(query)
    end
    total_benchmarks = benchmarks_set.shift
    benchmarks_set.each do |benchmarks|
      benchmarks.each_with_index do |benchmark, index|
        total_benchmarks[index] += benchmark
      end
    end
    report_benchmarks(total_benchmarks, query, label)
  end

  def verify_results(benchmarks)
    return if ENV["NO_VERIFY"]
    benchmarks = benchmarks.dup

    expected_result = benchmarks.shift.result
    benchmarks.each do |benchmark|
      raise "bad" unless assert_equivalent_to(expected_result, benchmark.result)
    end
  end

  def assert_equivalent_to(first_result, second_result)
    first_result == second_result
  end

  def create_report(benchmarks, query, label=nil)
    Report.new(query, label, benchmarks, repeat_count)
  end

  DEFAULT_WIKIPEDIA_DATABASE_LOCATION = "/tmp/wikipedia-db/db"
  class << self
    def select_benchmark_default_setup(runner, options=nil)
      options ||= {}

      configuration = Configuration.new
      configuration.database_path = options[:database_path] || DEFAULT_WIKIPEDIA_DATABASE_LOCATION
      ensure_database(configuration)

      context = Groonga::Context.new
      select_command = SelectorByCommand.new(context, configuration.database_path)
      select_method = SelectorByMethod.new(context, configuration.database_path)
      select_command_profile = command_selector_profile(select_command)
      select_method_profile = method_selector_profile(select_method)

      runner.context = context
      runner.add_profile(select_command_profile)
      runner.add_profile(select_method_profile)
    end

    def ensure_database(configuration)
      unless File.exist?(configuration.database_path)
        puts 'you must create wikipedia database to use, or specify it via "DATABASE_PATH" environment variable'
        puts
        puts 'how to create wikipedia database'
        puts '1. download wikipedia dump.'
        puts '   $ wget -c http://download.wikimedia.org/jawiki/latest/jawiki-latest-pages-articles.xml.bz2'
        puts '2. create groonga database from the dump'
        puts '   $ cat jawiki-latest-pages-articles.xml.bz2 | bunzip2 | ruby1.9.1 ./benchmark/create-wikipedia-database.rb'
        exit 1
      end
    end

    def command_selector_profile(select_command)
      Profile.new("select by command",
                  select_command,
                  [select_command.context.method(:send),
                   Groonga::Context::SelectResult.method(:parse)])
    end

    def method_selector_profile(select_method)
      Profile.new("select by method",
                  select_method,
                  [:do_select,
                   :sort,
                   :format,
                   :drilldown, [:do_group,
                                :drilldown_sort,
                                :drilldown_format]])
    end

    def output_columns_without_content
      "--output_columns '_id _key year wday timestamp month hour date last_contributor'"
    end

    def output_columns_with_content
      "--output_columns '_id _key year wday timestamp month hour date last_contributor content'"
    end

    def predefined_queries
      [
        ["select Documents",
         "minimum command"],
        ["select Documents --filter true",
         "select all"],
        ["select Documents --filter false",
         "select none"],
        ["select Documents content アルミ #{output_columns_without_content}",
         "full text search"],
        ["select Documents content アルミ #{output_columns_without_content} --limit 0",
         "full text search with no limit"],
        ["select Documents content アルミ #{output_columns_with_content}",
         "full text search output long text column"],
        ["select Documents content アルミ #{output_columns_without_content} --limit 1000",
         "full text search with large limit"],
        ["select Documents --filter true --limit 0 --drilldown last_contributor --drilldown_sortby _nsubrecs",
         "drilldown"],
        ["select Documents --filter true --limit 0 --drilldown last_contributor --drilldown_sortby _nsubrecs --drilldown_limit 10000",
         "drilldown with large drilldown_limit"],
        ["select Documents --sortby _key",
         "sort"],
        ["select Documents --sortby _key --drilldown 'year month date wday hour, last_contributor links' --drilldown_sortby _nsubrecs",
         "sort with drilldown"],
      ]
    end

    def load_predefined_queries(runner, options)
      predefined_queries.each do |command, label|
        query = Query.parse_groonga_query_log(command)
        runner.add_query(query, label)
      end
    end
  end
end

class Report
  def initialize(query, query_label, benchmarks, repeat_count)
    @query = query
    @query_label = query_label
    @benchmarks = benchmarks
    @repeat_count = repeat_count
  end

  def compare
  end

  def print
    puts "select command: #{@query_label}"
    puts "  #{@query.original_log_entry}"
    puts
    puts "repeated #{@repeat_count} time(s). Average times are:"

    lines = []
    @benchmarks.each do |benchmark|
      lines += benchmark.lines
    end
    width = lines.collect(&:first).collect(&:size).max

    puts(" " * (width - 1) + Benchmark::Tms::CAPTION.rstrip)
    lines.each do |label, result|
      puts "#{label.ljust(width)} #{(result / @repeat_count).to_s.strip}"
    end
  end
end

options = {
  :method => [:measure_time],
}

OptionParser.new do |parser|
  parser.on("--repeat=COUNT",
            "repeat each query COUNT times",
            "(default: #{BenchmarkRunner::DEFAULT_REPEAT_COUNT})") do |count|
    options[:repeat_count] = count.to_i
  end

  parser.on("--command=COMMAND",
            "use COMMAND instead of default predefined ones") do |command|
    options[:query] = Query.parse_groonga_query_log(command)
  end

  parser.on("--database=PATH",
            "use database located at PATH",
            "(default: #{BenchmarkRunner::DEFAULT_WIKIPEDIA_DATABASE_LOCATION})") do |command|
    options[:database_path] = command
  end
end.parse!(ARGV)

runner = BenchmarkRunner.new(options).tap do |runner|
  BenchmarkRunner.select_benchmark_default_setup(runner, options)
  if options[:query].nil?
    BenchmarkRunner.load_predefined_queries(runner, options)
  end
end

if options[:query]
  runner.run(options[:query])
else
  runner.run
end
