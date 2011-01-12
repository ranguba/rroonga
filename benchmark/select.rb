require 'groonga'

module Groonga
  class Context
    def open_database(database_path, options = {})
      Database.open(database_path, options.merge(:context => self))
    end
  end
end

class Query
  def initialize(options)
    @options = options
  end

  def table_name
    @options[:table]
  end

  def filter
    if @options[:match_columns] and @options[:query]
      #raise "unsupported" if @options[:filter]
      "#{@options[:match_columns]}:@#{@options[:query]}"
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
      Query.new(@parameters)
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
  attr_reader :context
  def initialize
    @context = Groonga::Context.new
  end

  def select(query)
    raise "implement"
  end

  class << self
    def setup(configuration)
      raise "implement"
    end
  end
end

class SelectorByCommand < Selector
  attr_reader :host, :port
  def initialize(host, port)
    super()

    @host = host
    @port = port
    @context.connect(:host => @host, :port => @port)
  end

  def select(query)
    parameters = query.parameters.merge(:cache => :no)
    result = @context.select(query.table_name, parameters)
    CommandResult.new(result)
  end

  class << self
    def setup(configuration)
      #XXX spawn groonga server by itself, using Configuration
      raise "implement"
    end
  end
end

class SelectorByMethod < Selector
  attr_reader :database_path
  def initialize(database_path)
    super()

    @database_path = database_path
    @database = @context.open_database(@database_path)
  end

  def select(query)
    table = @context[query.table_name]
    result = table.select(query.filter)


    sorted_result = sort(query, result)
    formatted_result = format(query, sorted_result || result)
    drilldown_results = drilldown(query, result)

    MethodResult.new(result, sorted_result, formatted_result, drilldown_results)
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
    columns.collect do |column|
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

  def format_result(result, output_columns)
    columns = tokenize_column_list(output_columns)
    result.collect do |record|
      columns.collect do |column|
        record[column]
      end
    end
  end

  def tokenize_column_list(column_list)
    tokens = column_list.split(/[\s,]/)
    tokens.reject!(&:empty?)
    tokens.select! do |token|
      token =~ /[A-Za-z0-9_]/
    end
    tokens.each do |token|
      token.sub!(/[^A-Za-z0-9_]\z/, '')
    end
  end
end

class Result
  def ==(other) # XXX needs more strict/rigid check
    if ENV["DEBUG"]
      pp "#{hit_count} == #{other.hit_count} and #{result_count} == #{other.result_count} and "
      pp "#{formatted_result} == #{other.formatted_result}"
    end

    hit_count == other.hit_count and result_count == other.result_count and formatted_result == other.formatted_result
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

  private
  def sorted_result
    @sorted_result || @result
  end
end

class Benchmark
  attr_accessor :result
  attr_accessor :profile

  class Time < Benchmark
    attr_reader :start_time, :end_time
    def initialize(profile, &block)
      @profile = profile

      measure_time(&block)
    end

    def time
      @end_time - @start_time
    end

    private
    def measure_time
      @start_time = ::Time.now
      @result = yield
      @end_time = ::Time.now
    end
  end

  def name
    profile.name
  end
end

class Profile
  attr_accessor :mode
  attr_reader :name
  def initialize(name, selector)
    @name = name
    @selector = selector
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
    Benchmark::Time.new(self) do
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
    create_report(benchmarks)
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

  def create_report(benchmarks)
    Report.new(benchmarks)
  end
end

class Report
  def initialize(benchmarks)
    @benchmarks = benchmarks
  end

  def compare
  end

  def print
    @benchmarks.each do |benchmark|
      puts "#{benchmark.name}: time: #{benchmark.time}"
    end
  end
end

configuration = Configuration.new
configuration.database_path = ENV["DATABASE_PATH"] || "/tmp/tutorial.db"

select_command = SelectorByCommand.new("localhost", 10041)
select_method = SelectorByMethod.new(configuration.database_path)

runner = Runner.new(:method => [:measure_time])
runner.add_profile(Profile.new("select by commnd", select_command))
runner.add_profile(Profile.new("select by method", select_method))

# at this point, setup is done
puts "setup is completed!"
puts

query_log = ENV["QUERY_LOG"] || "select --table Site --limit 3 --offset 2 --sortby '-title, _id' --output_columns title --drilldown title,_id,_key --drilldown_limit 7 --drilldown_offset 3 --drilldown_sortby _key"
puts "select command:"
puts "  #{query_log}"
puts

query = Query.parse_groonga_query_log(query_log)
report = runner.run_once(query)
report.print
