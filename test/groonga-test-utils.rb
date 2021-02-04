# Copyright (C) 2015  Masafumi Yokoyama <yokoyama@clear-code.com>
# Copyright (C) 2009-2019  Kouhei Sutou <kou@clear-code.com>
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

require "erb"
require "fileutils"
require "json"
require "pathname"
require "stringio"
require "tempfile"
require "time"
require "timeout"

require "groonga/client"
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
    name_for_path = escape_path(name)
    @database_path = @tmp_dir + "#{name_for_path}.db"
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
    Groonga::Logger.path = @log_path.to_s
    Groonga::Logger.reopen

    @query_log_path = @tmp_dir + "groonga-query.log"
    Groonga::QueryLogger.path = @query_log_path.to_s
    Groonga::QueryLogger.reopen
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
    Groonga::Logger.register(:max_level => :dump) do |*args|
      p args
    end
  end

  def collect_query_log
    @query_log_path.open do |file|
      file.seek(0, IO::SEEK_END)
      yield
      file.read
    end
  end

  def setup_database
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
    teardown_log_path
    teardown_tmp_directory
  end

  def teardown_log_path
    Groonga::Logger.path = nil
    Groonga::QueryLogger.path = nil

    return unless @dump_log
    if @log_path.exist?(log_path)
      header = "--- log: #{@log_path} ---"
      puts(header)
      puts(@log_path.read)
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

  def escape_path(path)
    path.gsub(/[: ?!\(\)\[\]]/) do
      "_"
    end
  end

  def linux?
    /linux/ =~ RUBY_PLATFORM
  end

  def only_linux
    omit("This test is only for Linux system.") unless linux?
  end

  def windows?
    /cygwin|mingw|mswin/ === RUBY_PLATFORM
  end

  def only_not_windows
    omit("This test is only for non Windows system.") if windows?
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

  def check_mecab_availability
    omit("MeCab isn't available") if context["TokenMecab"].nil?
  end

  def need_groonga(major, minor, micro)
    if (Groonga::BUILD_VERSION[0, 3] <=> [major, minor, micro]) < 0
      omit("Groonga #{major}.#{minor}.#{micro} or later is required")
    end
  end
end
