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
    end

    @host = "127.0.0.1"
    @port = 12345
    @remote_database_path = @tmp_dir + "remote-database"
    @process_id = spawn(groonga,
                        "-i", @host,
                        "-p", @port.to_s,
                        "-s", "-n", @remote_database_path.to_s)
    sleep(1)
  end

  teardown
  def teardown_remote_connection
    Process.kill(:TERM, @process_id) if @process_id
  end

  def test_send
    _context = Groonga::Context.new
    _context.connect(:host => @host, :port => @port)
    assert_equal(0, _context.send("status"))
    id, result = _context.receive
    assert_equal(0, id)
    values = JSON.load(result)
    assert_equal([
                   "alloc_count",
                   "cache_hit_rate",
                   "command_version",
                   "default_command_version",
                   "max_command_version",
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
