#!/usr/bin/env ruby

base_dir = File.expand_path(File.join(File.dirname(__FILE__), ".."))
test_unit_dir = File.join(base_dir, "test-unit", "lib")
src_dir = File.join(base_dir, "src")
lib_dir = File.join(src_dir, "lib")
test_dir = File.join(base_dir, "test")

make = nil
if system("which gmake > /dev/null")
  make = "gmake"
elsif system("which make > /dev/null")
  make = "make"
end
if make
  system("cd #{base_dir.dump} && #{make} > /dev/null") or exit(1)
end

$LOAD_PATH.unshift(test_unit_dir)

require 'test/unit'

$LOAD_PATH.unshift(base_dir)
$LOAD_PATH.unshift(src_dir)
$LOAD_PATH.unshift(lib_dir)

$LOAD_PATH.unshift(test_dir)
require 'groonga-test-utils'

Dir.glob("test/**/test{_,-}*.rb") do |file|
  require file.sub(/\.rb$/, '')
end

exit Test::Unit::AutoRunner.run(false)
