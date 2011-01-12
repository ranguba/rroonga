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
  def select(query)
    raise "implement"
  end
end

class SelectorByCommand < Selector #XXX spawn groonga server by itself, using Configuration
  def select(query)
    #@context.send(query.to_s)
    Result.new
  end
end

class SelectorByMethod < Selector
  def select(query)
    #query.table.select #...
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

select_command = SelectorByCommand.new("/tmp/foo/bar.db")
select_method = SelectorByMethod.new("localhost", 10041)

runner = Runner.new(:method => [:measure_time])
runner.add_profile(Profile.new("select by commnd", select_command))
runner.add_profile(Profile.new("select by method", select_method))

# at this point, setup is done

#query = Query.new("select foo bar...")
query = Query.parse_groonga_query_log("select foo bar...")
report = runner.run_once(query)
report.print
