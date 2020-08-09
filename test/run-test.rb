#!/usr/bin/env ruby
#
# Copyright (C) 2009-2016  Kouhei Sutou <kou@clear-code.com>
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

$VERBOSE = true

base_dir = File.expand_path(File.join(File.dirname(__FILE__), ".."))
ext_dir = File.join(base_dir, "ext", "groonga")
lib_dir = File.join(base_dir, "lib")
test_dir = File.join(base_dir, "test")

make = nil
if ENV["NO_MAKE"] != "yes"
  if system("which gmake > /dev/null")
    make = "gmake"
  elsif system("which make > /dev/null")
    make = "make"
  end
end
if make
  system("cd #{base_dir.dump} && #{make} > /dev/null") or exit(false)
end

require "test-unit"
require "test/unit/priority"

Test::Unit::Priority.enable unless ENV["CI"]


groonga_command_dir = File.join(base_dir, "..", "groonga-command")
if File.exist?(groonga_command_dir)
  $LOAD_PATH.unshift(File.join(groonga_command_dir, "lib"))
end

groonga_client_dir = File.join(base_dir, "..", "groonga-client")
if File.exist?(groonga_client_dir)
  $LOAD_PATH.unshift(File.join(groonga_client_dir, "lib"))
end


$LOAD_PATH.unshift(ext_dir)
$LOAD_PATH.unshift(lib_dir)
$LOAD_PATH.unshift(base_dir)

$LOAD_PATH.unshift(test_dir)
require "groonga-test-utils"

pkg_config = File.join(base_dir, "vendor", "local", "lib", "pkgconfig")
PKGConfig.add_path(pkg_config)

Dir.glob("#{base_dir}/test/**/test{_,-}*.rb") do |file|
  require file.sub(/\.rb$/, "")
end

ENV["TEST_UNIT_MAX_DIFF_TARGET_STRING_SIZE"] ||= "5000"

exit Test::Unit::AutoRunner.run
