require 'groonga'

module Groonga
  class Context
    def open_database(database_path, options = {})
      Database.open(database_path, options.merge(:context => self))
    end
  end
end

class Query
  class Parameter
  end

  class MatchParameter < Parameter
  end

  class FilterParameter < Parameter
  end

  class MatchColumnParameter < Parameter
  end

  class OutputColumnsPrameter < Parameter
  end

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
    result = @context.select(query.table_name, query.parameters)
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
    #result = table.select do |record|#("_key:@example.net")
    #  record["_key"] =~ "example.net"
    #end
    #if not query.drilldown_columns.empty?
    #  drilldown
    #end
    MethodResult.new(result)
  end

  def drilldown
  end
end

class Result
  def ==(other)
    #p "#{hit_count} != #{other.hit_count}"
    hit_count == other.hit_count #XXX
  end
end

class CommandResult < Result
  def initialize(result)
    @result = result
  end

  def hit_count
    @result.n_hits
  end
end

class MethodResult < Result
  def initialize(result)
    @result = result
  end

  def hit_count
    @result.size
  end
end

class Benchmark
  attr_accessor :result
  attr_accessor :profile

  class Time < Benchmark
    attr_reader :start_time, :end_time
    def initialize(profile)
      @profile = profile
      @start_time = ::Time.now
      @result = yield
      @end_time = ::Time.now
    end
  end
end

class Profile
  attr_accessor :mode
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
    #pp query
    #pp benchmarks
    verify_results(benchmarks)
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
    puts "fast or slow.."
  end
end

configuration = Configuration.new
configuration.database_path = "/tmp/tutorial.db"

select_command = SelectorByCommand.new("localhost", 10041)
select_method = SelectorByMethod.new(configuration.database_path)

runner = Runner.new(:method => [:measure_time])
runner.add_profile(Profile.new("select by commnd", select_command))
runner.add_profile(Profile.new("select by method", select_method))

# at this point, setup is done
puts "setup is completed!"
puts

query_log = "select --table Site --match_columns title --query f"
puts "select command:"
puts "  #{query_log}"
query = Query.parse_groonga_query_log(query_log)
report = runner.run_once(query)
puts
report.print
