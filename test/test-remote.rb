# Copyright (C) 2009-2022  Sutou Kouhei <kou@clear-code.com>
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

class RemoteTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :before => :append
  def setup_remote_connection
    @process_id = nil

    vendor_local_bin = File.join(__dir__, "..", "vendor", "local", "bin")
    if File.exist?(vendor_local_bin)
      groonga = File.join(vendor_local_bin, "groonga")
    else
      package_config = PKGConfig.package_config("groonga")
      groonga = package_config.variable("groonga")
      groonga = normalize_groonga_path(groonga)
      groonga = "groonga" unless File.exist?(groonga)
    end

    @host = "127.0.0.1"
    @port = 12345
    @remote_database_path = @tmp_dir + "remote-database"
    @process_id = spawn(groonga,
                        "-i", @host,
                        "-p", @port.to_s,
                        "-s",
                        "-n", @remote_database_path.to_s)
    Timeout.timeout(5) do
      loop do
        begin
          open_client do |client|
            client.status
          end
        rescue Groonga::Client::Error
        else
          break
        end
      end
    end
  end

  def normalize_groonga_path(groonga)
    return groonga unless groonga
    return groonga unless Object.const_defined?(:RubyInstaller)

    msys2_installation = RubyInstaller::Runtime.msys2_installation
    mingw_prefix = msys2_installation.mingw_prefix
    mingw_bin_path = "#{mingw_prefix}/bin/"
    mingw_bin_path_windows = "#{msys2_installation.mingw_bin_path}\\"
    groonga.gsub(/\A#{Regexp.escape(mingw_bin_path)}/) do
      mingw_bin_path_windows
    end
  end

  teardown
  def teardown_remote_connection
    if @process_id
      begin
        open_client do |client|
          client.shutdown
        end
      rescue Groonga::Client::Error
      end
      Process.waitpid(@process_id)
    end
  end

  def open_client(&block)
    Groonga::Client.open(host: @host,
                         port: @port,
                         protocol: :gqtp,
                         &block)
  end

  def test_send
    _context = Groonga::Context.new
    _context.connect(:host => @host, :port => @port)
    assert_equal(0, _context.send("status"))
    id, result = _context.receive
    assert_equal(0, id)
    values = JSON.load(result)
    values.delete("apache_arrow")
    values.delete("features")
    assert_equal([
                   "alloc_count",
                   "cache_hit_rate",
                   "command_version",
                   "default_command_version",
                   "max_command_version",
                   "memory_map_size",
                   "n_jobs",
                   "n_queries",
                   "start_time",
                   "starttime",
                   "uptime",
                   "version",
                 ],
                 values.keys.sort)
  end

  def test_invalid_select
    context.connect(:host => @host, :port => @port)

    assert_raise(Groonga::InvalidArgument) do
      context.select("bogus", :query => "()()")
    end
  end
end
