require "fileutils"
require "shellwords"

require "groonga"

$LOAD_PATH.unshift(File.dirname(__FILE__))
require "create-wikipedia-database"

class SampleRecords
  def initialize(record_count)
    @record_count = record_count
    @current_count = 0
    @records = []

    initialize_sample_records
  end

  def initialize_sample_records
    extractor = WikipediaExtractor.new(WikipediaImporter.new(self))
    #@records = @record_count.times.collect do
    #  create_random_item
    #end
    catch(:stop_extract) do
      extractor.extract(ARGF)
    end
  end

  def load(page)
    @current_count += 1
    record = {
      :_key => page[:title],
      :content => page[:content],
    }
    #pp record
    @records << record
    throw :stop_extract if @current_count == @record_count
  end

  def values(count=nil)
    count ||= @record_count

    if count == 1
      [first_record]
    else
      @records[0, count - 1] + [first_record]
    end
  end

  def first_record
    @records.first
  end

  def each(*arguments, &block)
    values.each(*arguments, &block)
  end

  def n_records(count)
    values(count)
  end

  def create_random_item
    {"_key" => "ryoqun"}
  end
end

class RepeatLoadRunner
  DATABASE_DIRECTORY = "/tmp/repeat-overwrite"

  def initialize(sample_records, options=nil)
    @options = options || {}
    @context = Groonga::Context.new(:encoding => :none)
    @sample_records = sample_records
  end

  DEFAULT_REPEAT_COUNT = 1
  DEFAULT_RECORD_COUNT = 1
  def repeat_count
    @options[:repeat_count] || DEFAULT_REPEAT_COUNT
  end

  def record_count
    @options[:record_count] || DEFAULT_RECORD_COUNT
  end

  def with_index?
    @options[:with_index]
  end

  def database_directory
    DATABASE_DIRECTORY
  end

  def database_path
    "#{database_directory}/db"
  end

  def setup_database
    FileUtils.rm_rf(database_directory)
    FileUtils.mkdir_p(database_directory)

    @context.create_database(database_path)
    Groonga::Schema.define(:context => @context) do |schema|
      schema.create_table("Contents", :type => :hash, :key_type => "ShortText") do |table|
        table.long_text("content")
      end

      if with_index?
        schema.create_table("Terms", :type => :hash, :default_tokenizer => "TokenBigram") do |table|
          table.index("Contents._key")
          table.index("Contents.content")
        end
      end
    end
  end

  def run
    setup_database

    before_load
    add_record
    after_load
  end

  private
  def add_record
    puts "loading..."
    repeat_count.times do |count|
      add_record_via_load_command
      if repeat_count != 1 and count.zero?
        after_first_load
      end
    end
    puts "... (#{repeat_count} times repeated)"
    puts
  end

  def add_record_via_load_command
    puts "iteration: loading #{record_count} records"
    count = 0
    @sample_records.n_records(record_count).each do |record|
      mangle_record(record)

      #pp record
      command = "load --table Contents --input_type json --values '#{JSON.generate(record).gsub(/\'/, '').gsub(/\"/, '\\\"')}'"
      #puts command
      @context.send(command)
      if record_count != 1 and count.zero?
        after_first_load
      end
      count += 1
    end
  end

  def mangle_record(record)
    record[:content] = record[:content][0, 400]
    record[:_key] = :FIXED_KEY
  end

  def before_load
    puts "before load:"
    measure_database_size
    puts
  end

  def after_first_load
    puts "after first load:"
    measure_database_size
    puts
  end

  def after_load
    puts "after load:"
    measure_database_size
    puts
  end

  def measure_database_size
    #measure_apparent_size
    measure_actual_size
  end

  def measure_apparent_size
    puts "apparent disk usage:"
    puts execute_du("--apparent-size")
  end

  def measure_actual_size
    puts "actual disk usage:"
    puts execute_du
  end

  def execute_du(options=nil)
    `sync`
    `find #{database_directory} -type f -print0 | xargs -0 du --human-readable #{options.to_s} | sort -k 2 | uniq`
  end
end

sample_records = SampleRecords.new(1000)

#puts "load one record, repeat one time"
#RepeatLoadRunner.new(sample_records).run

#puts "load one record, repeat 100 time"
#RepeatLoadRunner.new(sample_records, :repeat_count => 100).run

#puts "load one record, repeat 100 time with index column defined"
#RepeatLoadRunner.new(sample_records, :repeat_count => 100, :with_index => true).run

puts "load 100 records, repeat 1 time"
RepeatLoadRunner.new(sample_records, :record_count => 100).run

puts "load 100 records, repeat 1 time with index column defined"
RepeatLoadRunner.new(sample_records, :record_count => 100, :with_index => true).run
