# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

require "fileutils"
require "pathname"
require "tempfile"
require "time"
require "erb"
require "stringio"
require "json"
require "pkg-config"

require "groonga"

module GroongaTestUtils
  class << self
    def included(base)
      base.setup :setup_sandbox, :before => :prepend
      base.teardown :teardown_sandbox, :after => :append
    end
  end

  def setup_sandbox
    setup_tmp_directory
    setup_log_path

    setup_tables_directory
    setup_columns_directory

    setup_encoding
    setup_context

    @database = nil
  end

  def setup_tmp_directory
    @base_tmp_dir = Pathname(File.dirname(__FILE__)) + "tmp"
    memory_file_system = "/run/shm"
    if File.exist?(memory_file_system)
      FileUtils.mkdir_p(@base_tmp_dir.parent.to_s)
      FileUtils.rm_f(@base_tmp_dir.to_s)
      FileUtils.ln_s(memory_file_system, @base_tmp_dir.to_s)
    else
      FileUtils.mkdir_p(@base_tmp_dir.to_s)
    end

    @tmp_dir = @base_tmp_dir + "rroonga"
    FileUtils.rm_rf(@tmp_dir.to_s)
    FileUtils.mkdir_p(@tmp_dir.to_s)
  end

  def setup_log_path
    @dump_log = false

    @log_path = @tmp_dir + "groonga.log"
    logger = Groonga::FileLogger.new(@log_path.to_s)
    Groonga::Logger.register(logger)

    @query_log_path = @tmp_dir + "groonga-query.log"
    query_logger = Groonga::FileQueryLogger.new(@query_log_path.to_s)
    Groonga::QueryLogger.register(query_logger, :all => true)
  end

  def setup_tables_directory
    @tables_dir = @tmp_dir + "tables"
    FileUtils.mkdir_p(@tables_dir.to_s)
  end

  def setup_columns_directory
    @columns_dir = @tmp_dir + "columns"
    FileUtils.mkdir_p(@columns_dir.to_s)
  end

  def setup_encoding
    Groonga::Encoding.default = nil
  end

  def setup_context
    Groonga::Context.default = nil
    Groonga::Context.default_options = nil
  end

  def context
    Groonga::Context.default
  end

  def encoding
    Groonga::Encoding.default
  end

  def setup_logger
    Groonga::Logger.register(:level => :dump) do |level, time, title, message, location|
      p [level, time, title, message, location]
    end
  end

  def setup_database
    @database_path = @tmp_dir + "database"
    @database = Groonga::Database.create(:path => @database_path.to_s)
  end

  def teardown_database
    return if @database.nil?

    @database.close
    @database = nil
  end

  def teardown_sandbox
    teardown_database
    Groonga::Context.default.close
    Groonga::Context.default = nil
    GC.start
    teardown_log_path
    teardown_tmp_directory
  end

  def teardown_log_path
    return unless @dump_log
    log_path = Groonga::Logger.log_path
    if File.exist?(log_path)
      header = "--- log: #{log_path} ---"
      puts(header)
      puts(File.read(log_path))
      puts("-" * header.length)
    end
    if @query_log_path.exist?
      header = "--- query log: #{@query_log_path} ---"
      puts(header)
      puts(@query_log_path.read)
      puts("-" * header.length)
    end
  end

  def teardown_tmp_directory
    FileUtils.rm_rf(@tmp_dir.to_s)
    FileUtils.rm_rf(@base_tmp_dir.to_s)
  end

  private
  def assert_equal_select_result(expected, actual, &normalizer)
    normalizer ||= Proc.new {|record| record.key}
    assert_equal(expected,
                 actual.collect(&normalizer),
                 actual.expression.inspect)
  end

  def ruby19?
    RUBY_VERSION >= "1.9"
  end

  def only_ruby19
    omit("Ruby 1.9 is needed.") unless ruby19?
  end

  def support_self_recursive_equal?
    self_recursive_hash1 = {}
    self_recursive_hash1["next"] = self_recursive_hash1
    self_recursive_hash2 = {}
    self_recursive_hash2["next"] = self_recursive_hash2
    self_recursive_hash1 == self_recursive_hash2
  end

  def need_self_recursive_equal
    omit("self recursive equal is needed.") unless support_self_recursive_equal?
  end

  def need_encoding
    omit("Encoding is needed.") unless defined?(::Encoding)
  end
end
