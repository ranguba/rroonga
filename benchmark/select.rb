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

  class GroongaLogParser
    def initialize(log)
      @log = log
    end

    def parse
    end

    class << self
      def parse(log)
        new(log).parse
      end
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
    #@context.select(query.table_name, query.parameters)
    Result.new
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
    #table = @context[query.table_name]
    #table.select #...
    #if not query.drilldown_columns.empty?
    #  drilldown
    #end
    Result.new
  end

  def drilldown
  end
end

class Result
  def ==(other)
    true #XXX
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

query_log = "select --table Site"
query = Query.parse_groonga_query_log(query_log)
report = runner.run_once(query)
report.print
