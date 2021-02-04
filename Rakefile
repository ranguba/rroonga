# -*- ruby -*-
#
# Copyright (C) 2009-2020  Sutou Kouhei <kou@clear-code.com>
# Copyright (C) 2017  Masafumi Yokoyama <yokoyama@clear-code.com>
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

require "bundler/gem_helper"
require "packnga"
require "yard"

base_dir = File.join(File.dirname(__FILE__))

groonga_ext_dir = File.join(base_dir, "ext", "groonga")
groonga_lib_dir = File.join(base_dir, "lib")
$LOAD_PATH.unshift(groonga_ext_dir)
$LOAD_PATH.unshift(groonga_lib_dir)
ENV["RUBYLIB"] = "#{groonga_lib_dir}:#{groonga_ext_dir}:#{ENV['RUBYLIB']}"

helper = Bundler::GemHelper.new(base_dir)
def helper.version_tag
  version
end

helper.install
spec = helper.gemspec

Packnga::DocumentTask.new(spec) do |task|
  task.original_language = "en"
  task.translate_language = "ja"
end

ranguba_org_dir = Dir.glob("{..,../../www}/ranguba.org").first
Packnga::ReleaseTask.new(spec) do |task|
  task.index_html_dir = ranguba_org_dir
end

file "Makefile" => ["extconf.rb", "ext/groonga/extconf.rb"] do
  extconf_args = []
  if ENV["TRAVIS"]
    extconf_args << "--enable-debug-build"
  end
  ruby("extconf.rb", *extconf_args)
end

desc "Configure"
task :configure => "Makefile"

desc "Run test"
task :test => :configure do
  ruby("test/run-test.rb")
end

namespace :test do
  task :install => "install" do
    rroonga_specs = Gem.source_index.find_name("rroonga")
    target_rroonga_spec = rroonga_specs.find do |rroonga_spec|
      rroonga_spec.version == helper.gemspec.version
    end
    installed_path = target_rroonga_spec.full_gem_path
    ENV["NO_MAKE"] = "yes"
    ruby("#{installed_path}/test/run-test.rb")
  end
end

def update_version(new_version)
  splitted_new_version = new_version.split(".")
  new_major_version = splitted_new_version[0]
  new_minor_version = splitted_new_version[1]
  new_micro_version = splitted_new_version[2]

  rroonga_version = {
    "RB_GRN_MAJOR_VERSION" => new_major_version,
    "RB_GRN_MINOR_VERSION" => new_minor_version,
    "RB_GRN_MICRO_VERSION" => new_micro_version
  }

  File.open("./ext/groonga/rb-grn.h", "rb+") do |file|
    rb_grn_header = file.read

    rroonga_version.each do |key, value|
      replacement = "#{key} #{value}"
      rb_grn_header.gsub!(/#{key}\s\d{1,}/, replacement)
    end
    file.rewind()
    file.write(rb_grn_header)
  end
end

namespace :release do
  desc "Bump version"
  task :version, ['new_version'] do |_, args|
    update_version(args[:new_version])
  end
end

task :default => :test
