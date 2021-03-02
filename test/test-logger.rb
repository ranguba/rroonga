# Copyright (C) 2010-2019  Kouhei Sutou <kou@clear-code.com>
# Copyright (C) 2016  Masafumi Yokoyama <yokoyama@clear-code.com>
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

class LoggerTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    @default_log_path = Groonga::Logger.path
    @default_log_max_level = Groonga::Logger.max_level
    @default_rotate_threshold_size = Groonga::Logger.rotate_threshold_size
    @default_flags = Groonga::Logger.flags
  end

  def teardown
    Groonga::Logger.path = @default_log_path
    Groonga::Logger.max_level = @default_log_max_level
    Groonga::Logger.rotate_threshold_size = @default_rotate_threshold_size
    Groonga::Logger.flags = @default_flags
  end

  def test_reopen
    only_not_windows
    Groonga::Logger.unregister
    Groonga::Logger.path = @log_path.to_s
    if @log_path.exist?
      FileUtils.mv(@log_path, "#{@log_path}.old")
    end
    assert_false(@log_path.exist?)
    # Reopen log uses debug level since Groonga 11.0.1.
    Groonga::Logger.max_level = :debug
    Groonga::Logger.reopen
    assert_true(@log_path.exist?)
  end

  def test_max_level
    assert_equal(:notice, Groonga::Logger.max_level)
    Groonga::Logger.max_level = :debug
    assert_equal(:debug, Groonga::Logger.max_level)
  end

  sub_test_case ".flags" do
    def test_default
      assert_equal(Groonga::Logger::Flags::TIME |
                   Groonga::Logger::Flags::MESSAGE,
                   Groonga::Logger.flags)
    end

    def test_location
      Groonga::Logger.flags = Groonga::Logger::Flags::LOCATION
      assert_equal(Groonga::Logger::Flags::LOCATION, Groonga::Logger.flags)
    end

    def test_thread_id
      Groonga::Logger.flags = Groonga::Logger::Flags::THREAD_ID
      assert_equal(Groonga::Logger::Flags::THREAD_ID, Groonga::Logger.flags)
    end
  end

  sub_test_case ".log" do
    setup do
      GC.disable
    end

    teardown do
      Groonga::Logger.unregister
      GC.enable
    end

    test "no options" do
      messages = []
      Groonga::Logger.register do |event, level, time, title, message, location|
        messages << message
      end
      Groonga::Logger.log("1")
      Groonga::Logger.log("2")
      Groonga::Logger.log("3")
      assert_equal(["1", "2", "3"],
                   messages)
    end

    test ":level" do
      levels = []
      Groonga::Logger.register(:max_level => :dump) do |event, level, *rest|
        levels << level
      end
      Groonga::Logger.log("default")
      Groonga::Logger.log("debug", :level => :debug)
      assert_equal([:notice, :debug],
                   levels)
    end

    test "default location" do
      locations = []
      Groonga::Logger.register(thread_id: false) do |*args|
        location = args[5]
        locations << location
      end
      Groonga::Logger.log("message"); line = __LINE__
      function = caller_locations(0, 1)[0].label
      assert_equal([
                     "#{Process.pid} #{__FILE__}:#{line} #{function}()",
                   ],
                   locations)
    end

    test ":file" do
      locations = []
      Groonga::Logger.register(thread_id: false) do |*args|
        location = args[5]
        locations << location
      end
      Groonga::Logger.log("message", :file => "file.rb")
      locations = locations.collect do |location|
        location.gsub(/\A(\d+) (.*?):(\d+) (.*?)\(\)\z/,
                      "0 \\2:0 function()")
      end
      assert_equal([
                     "0 file.rb:0 function()",
                   ],
                   locations)
    end

    test ":line" do
      locations = []
      Groonga::Logger.register(thread_id: false) do |*args|
        location = args[5]
        locations << location
      end
      Groonga::Logger.log("message", :line => 100)
      locations = locations.collect do |location|
        location.gsub(/\A(\d+) (.*?):(\d+) (.*?)\(\)\z/,
                      "0 test.rb:\\3 function()")
      end
      assert_equal([
                     "0 test.rb:100 function()",
                   ],
                   locations)
    end

    test ":function" do
      locations = []
      Groonga::Logger.register(thread_id: false) do |*args|
        location = args[5]
        locations << location
      end
      Groonga::Logger.log("message", :function => "method_name")
      locations = locations.collect do |location|
        location.gsub(/\A(\d+) (.*?):(\d+) (.*?)\(\)\z/,
                      "0 test.rb:0 \\4()")
      end
      assert_equal([
                     "0 test.rb:0 method_name()",
                   ],
                   locations)
    end
  end

  sub_test_case ".register" do
    setup do
      GC.disable
    end

    teardown do
      Groonga::Logger.unregister
      GC.enable
    end

    test ":thread_id" do
      locations = []
      Groonga::Logger.register(location: false,
                               thread_id: true) do |*args|
        location = args[5]
        locations << location
      end
      Groonga::Logger.log("message")
      locations = locations.collect do |location|
        location.gsub(/\A([a-f\d]+)\z/, "THREAD_ID")
      end
      assert_equal([
                     "THREAD_ID",
                   ],
                   locations)
    end
  end

  def test_rotate_threshold_size
    Groonga::Logger.unregister
    Groonga::Logger.path = @log_path.to_s
    Groonga::Logger.reopen
    Groonga::Logger.rotate_threshold_size = 10
    assert_equal([], Dir.glob("#{@log_path}.*"))
    Groonga::Logger.log("Hello")
    assert_not_equal([], Dir.glob("#{@log_path}.*"))
  end

  sub_test_case "Flags" do
    sub_test_case "#label" do
      test "pid" do
        flags = Groonga::Logger::Flags::PID
        assert_equal("pid", Groonga::Logger::Flags.label(flags))
      end
    end
  end
end
