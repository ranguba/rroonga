require 'benchmark'

require 'groonga'

module Groonga
  class Record
    def include?(column)
      not table.column(column).nil?
    end
  end
end

class Query
  attr_reader :options
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
      "#{match_columns} @ #{@options[:query]}" #XXX correct escape
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

      @log.each_char do |character|
        #puts "#{character.inspect} #{escape.inspect}"
        case character
        when / /
          if escape.nil? and litaral.nil?
            token = next_token(token)
          elsif escape == "\\"
            escape = nil
            token << character
          else
            token << character
          end
        when /[\"\']/
          if escape.nil? and litaral.nil?
            litaral = character
          elsif escape == "\\"
            escape = nil
            token << character
          elsif character == litaral
            litaral = nil
            token = next_token(token)
          else
            token << character
          end
        when "\\"
          if escape.nil?
            escape = "\\"
          else
            escape = nil
            token << character
          end
        else
          if escape == "\\"
            escape = nil
          end

          token << character
        end
        #puts "#{token.inspect} #{escape.inspect}"
      end

      raise "parse error (terminated with escape: #{escape.inspect})" unless escape.nil?
      @tokens << token
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
      name.sub(NAMED_PARAMETER_PREFIX, '').to_sym
    end

    def create_query
      Query.new(@parameters.merge(:original_log_entry => @log))
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
  def initialize(database_path)
    @context = Groonga::Context.new
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
    result = @context.select(query.table_name, parameters)
    CommandResult.new(result)
  end
end

class SelectorByMethod < Selector
  def select(query)
    table = @context[query.table_name]
    filter = query.filter
    default_column = table.column(query.match_columns)

    result = do_select(filter, table, default_column)
    sorted_result = sort(query, result)
    formatted_result = format(query, sorted_result || result)
    drilldown_results = drilldown(query, result)

    MethodResult.new(result, sorted_result, formatted_result, drilldown_results)
  end

  def do_select(filter, table, default_column)
    table.select(filter, :default_column => default_column, :syntax => :script)
  end

  def sort(query, result)
    if needs_sort?(query)
      sort_key = sort_key(query.sort_by)
      window_options = window_options(query.limit, query.offset)
      sorted_result = result.sort(sort_key, window_options).collect do |record|
        record.key
      end
    end
  end

  def drilldown_sort(query, result)
    if needs_drilldown_sort?(query)
      sort_key = sort_key(query.drilldown_sort_by)
      window_options = window_options(query.drilldown_limit, query.drilldown_offset)

      sorted_result = result.sort(sort_key, window_options).collect do |record|
        record
      end
    end
  end

  def format(query, result)
    if needs_format?(query)
      formatted_result = format_result(result, query.output_columns)
    end
  end

  def drilldown_format(query, result)
    format_result(result, query.drilldown_output_columns || "_key, _nsubrecs")
  end

  def drilldown(query, result)
    if needs_drilldown?(query)
      drilldown_results = drilldown_result(result, query.drilldown_columns, query)
    end
  end

  def drilldown_result(result, drilldown_columns, query)
    columns = tokenize_column_list(drilldown_columns)
    columns.uniq.collect do |column|
      drilldown_result = result.group(column)
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

  def needs_sort?(query)
    query.limit or query.offset or query.sort_by
  end

  def needs_format?(query)
    query.output_columns
  end

  def needs_drilldown?(query)
    query.drilldown_columns
  end

  def needs_drilldown_sort?(query)
    query.drilldown_limit or query.drilldown_offset or query.drilldown_sort_by
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
      key = token.sub(DESCENDING_ORDER_PREFIX, '')
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

  def window_options(limit, offset)
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
    columns = tokenize_column_list(output_columns)
    table = result.first.key
    columns = columns.collect do |column|
      if column == "*"
        table.columns.collect(&:name).collect do |name|
          name.sub(/\A[A-Za-z0-9_]+\./, '')
        end
      else
        column if result.first.include?(column)
      end
    end.flatten.compact

    table = result.first.table
    result.collect do |record|
      columns.collect do |column|
        value = record[column]
        to_json(value, access_column(table, column))
      end
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

  def tokenize_column_list(column_list)
    tokens = column_list.split(/[\s,]/)
    tokens.reject!(&:empty?)
    tokens.select! do |token|
      token == "*" || token =~ /[A-Za-z0-9_]/
    end
    tokens.each do |token|
      unless token == "*"
        token.sub!(/[^A-Za-z0-9_]\z/, '')
      end
    end
  end
end

class Result
  def ==(other) # XXX needs more strict/rigid check
    if ENV["DEBUG"] or ENV["DEBUG_VERIFY"]
      pp "#{hit_count} == #{other.hit_count} and #{result_count} == #{other.result_count} and "
      #pp "#{formatted_result} == #{other.formatted_result}"
    end

    [
      hit_count == other.hit_count,
      result_count == other.result_count,
      formatted_result == other.formatted_result,
      drilldown_results == other.drilldown_results,
    ].all?
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
    @drilldown_results.collect do |result|
      result[:format]
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
    def initialize(profile, target_object, &block)
      @intercepted_method_times = {}
      @profile = profile
      @target_object = target_object
      setup_intercepted_methods

      measure_time(&block)
    end

    def lines
      super + @intercepted_method_times.collect do |method_name, status|
        ["  #{method_name}", status[:benchmark_result]]
      end
    end

    private
    def measure_time
      @benchmark_result = Benchmark.measure do
        @result = yield
      end
    end

    def setup_intercepted_methods
      @profile.intercepted_methods.each do |method|
        case method
        when Symbol
          intercept_method(@target_object.class, method)
        when Method
          if method.receiver.is_a?(Class)
            intercept_method(method.owner, method.name)
          else
            intercept_method(method.receiver.class, method.name)
          end
        else
          raise "bad"
        end
      end
    end

    def intercept_method(klass, method_name)
      intercepted_method_times = @intercepted_method_times
      original_method_name = :"__intercepted__#{method_name}"

      klass.class_exec do
        alias_method original_method_name, method_name
        define_method method_name do |*arguments, &block|
          returned_object = nil
          benchmark_result = Benchmark.measure do
            returned_object = __send__(original_method_name, *arguments, &block)
          end
          intercepted_method_times[method_name] = { # XXX include klass into key # XXX support multiple invocations
            :benchmark_result => benchmark_result,
          }
          returned_object
        end
      end
    end
  end

  def lines
    [[name, @benchmark_result]]
  end

  def name
    profile.name
  end
end

class Profile
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

  private
  def measure_time(query)
    BenchmarkResult::Time.new(self, @selector) do
      result = @selector.select(query)
      result
    end
  end
end

class Runner
  DEFAULT_MODE = :measure_time # :mesure_memory, :mesure_io, :mesure_???

  def initialize(options={})
    @options = options
    @profiles = []
  end

  def benchmark_mode
    @options[:mode] || DEFAULT_MODE
  end

  def add_profile(profile)
    profile.mode = benchmark_mode
    @profiles << profile
  end

  def run_once(query)
    benchmarks = @profiles.collect do |profile|
      profile.take_benchmark(query)
    end
    if ENV["DEBUG"]
      pp query
      pp benchmarks
    end
    verify_results(benchmarks) unless ENV["NO_VERIFY"]
    create_report(query, benchmarks)
  end

  def verify_results(benchmarks)
    benchmarks = benchmarks.dup

    expected_result = benchmarks.shift.result
    benchmarks.each do |benchmark|
      raise "bad" unless assert_equivalent_to(expected_result, benchmark.result)
    end
  end

  def assert_equivalent_to(first_result, second_result)
    first_result == second_result
  end

  def create_report(query, benchmarks)
    Report.new(query, benchmarks)
  end
end

class Report
  def initialize(query, benchmarks)
    @query = query
    @benchmarks = benchmarks
  end

  def compare
  end

  def print
    lines = []

    puts "select command:"
    puts "  #{@query.options[:original_log_entry]}"

    @benchmarks.each do |benchmark|
      lines += benchmark.lines
    end
    width = lines.collect(&:first).collect(&:size).max
    #pp lines#.collect(&:first)
    #puts "#{benchmark.name}: time: #{benchmark.time} #{benchmark.intercepted_method_times}"

    puts(" " * (width - 1) + Benchmark::Tms::CAPTION.rstrip)
    lines.each do |label, result|
      puts "#{label.ljust(width)} #{result.to_s.strip}"
    end
  end
end

configuration = Configuration.new
configuration.database_path = ENV["DATABASE_PATH"] || "/tmp/tutorial.db"

select_command = SelectorByCommand.new(configuration.database_path)
select_method = SelectorByMethod.new(configuration.database_path)

runner = Runner.new(:method => [:measure_time])
runner.add_profile(Profile.new("select by commnd", select_command, [select_command.context.method(:send), Groonga::Context::SelectResult.method(:parse)]))
runner.add_profile(Profile.new("select by method", select_method, [:do_select, :sort, :format, :drilldown]))

query_log = ENV["QUERY_LOG"] || "select --table Site --limit 3 --offset 2 --sortby '-title, _id' --output_columns title --drilldown title,_id,_key --drilldown_limit 7 --drilldown_offset 3 --drilldown_sortby _key"

begin
  query = Query.parse_groonga_query_log(query_log)
  report = runner.run_once(query)
  report.print
rescue Exception => error
  pp error
  pp error.back_trace
end
