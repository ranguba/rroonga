# -*- coding: utf-8; mode: ruby -*-
#
# Copyright (C) 2009-2018  Kouhei Sutou <kou@clear-code.com>
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

require "find"
require "fileutils"
require "shellwords"
require "pathname"
require "erb"
require "yard"
require "bundler/gem_helper"
require "rake/extensiontask"
require "packnga"

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

module YARD
  module CodeObjects
    class Proxy
      alias_method :initialize_original, :initialize
      def initialize(namespace, name, type=nil)
        name = name.to_s.gsub(/\AGrn(.*)\z/) do
          suffix = $1
          case suffix
          when ""
            "Groonga"
          when "TableKeySupport"
            "Groonga::Table::KeySupport"
          else
            "Groonga::#{suffix}"
          end
        end
        initialize_original(namespace, name, type)
      end
    end
  end
end

def windows?(platform=nil)
  platform ||= RUBY_PLATFORM
  platform =~ /mswin(?!ce)|mingw|cygwin|bccwin/
end

def collect_binary_files(binary_dir)
  binary_files = []
  Find.find(binary_dir) do |name|
    next unless File.file?(name)
    next if /\.zip\z/i =~ name
    binary_files << name
  end
  binary_files
end

def windows_gem_name(spec, architecture)
  "#{spec.name}-#{spec.version}-#{architecture}-mingw32.gem"
end

relative_vendor_dir = "vendor"
relative_binary_dir = File.join("vendor", "local")
vendor_dir = File.join(base_dir, relative_vendor_dir)
binary_dir = File.join(base_dir, relative_binary_dir)

groonga_win32_i386_p = ENV["RROONGA_USE_GROONGA_X64"].nil?

Rake::ExtensionTask.new("groonga", spec) do |ext|
  if groonga_win32_i386_p
    ext.cross_platform = ["x86-mingw32"]
  else
    ext.cross_platform = ["x64-mingw32"]
  end
  if windows?
    ext.gem_spec.files += collect_binary_files(relative_binary_dir)
  else
    ext.cross_compile = true
    ext.cross_compiling do |_spec|
      if windows?(_spec.platform.to_s)
        binary_files = collect_binary_files(relative_binary_dir)
        _spec.files += binary_files
      end
    end
  end
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
  ruby("-rubygems", "test/run-test.rb")
end

namespace :test do
  task :install do
    gemspec_helper = Rake.application.jeweler.gemspec_helper
    ruby("-S gem install --user-install #{gemspec_helper.gem_path}")

    gem_spec = Gem.source_index.find_name("rroonga").last
    installed_path = gem_spec.full_gem_path
    ENV["NO_MAKE"] = "yes"
    ruby("-rubygems", "#{installed_path}/test/run-test.rb")
  end
end

desc "Remove Groonga binary directory"
namespace :clean do
  task :groonga do
    rm_rf binary_dir
  end
end

windows_architectures = [:x86, :x64]

namespace :build do
  namespace :windows do
    ruby_versions = "2.1.6:2.2.2:2.3.0:2.4.0:2.5.0"

    windows_architectures.each do |architecture|
      desc "Build gem for Windows #{architecture}"
      task architecture do
        build_dir = "tmp/windows"
        rm_rf build_dir
        mkdir_p build_dir

        commands = [
          ["sudo", "apt-get", "-y", "install", "software-properties-common"],
          ["sudo", "add-apt-repository", "-y", "universe"],
          ["sudo", "add-apt-repository", "-y", "ppa:groonga/ppa"],
          ["sudo", "apt-get", "update"],
          ["sudo", "apt-get", "install", "-y", "libgroonga-dev"],
          ["git", "clone", "file://#{Dir.pwd}/.git", build_dir],
          ["cd", build_dir],
          ["gem", "install", "json"],
          ["bundle"],
          ["rake", "cross", "native", "gem", "RUBY_CC_VERSION=#{ruby_versions}"],
        ]
        if architecture == :x64
          commands.unshift(["export", "RROONGA_USE_GROONGA_X64=true"])
        end
        raw_commands = commands.collect do |command|
          Shellwords.join(command)
        end
        raw_command_line = raw_commands.join(" && ")

        require "rake_compiler_dock"
        RakeCompilerDock.sh(raw_command_line)

        cp("#{build_dir}/pkg/#{windows_gem_name(spec, architecture)}",
           "pkg/")
      end
    end
  end

  desc "Build gems for Windows"
  build_tasks = windows_architectures.collect do |architecture|
    "windows:#{architecture}"
  end
  task :windows => build_tasks
end

namespace :release do
  desc "Push gems for Windows to RubyGems.org"
  task :windows do
    windows_architectures.each do |architecture|
      ruby("-S", "gem", "push", "pkg/#{windows_gem_name(spec, architecture)}")
    end
  end
end

task :default => :test
