# -*- coding: utf-8; mode: ruby -*-
#
# Copyright (C) 2009-2012  Kouhei Sutou <kou@clear-code.com>
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

require 'English'

require 'find'
require 'fileutils'
require 'pathname'
require 'erb'
require 'rubygems'
require 'rubygems/package_task'
require 'yard'
require 'bundler/gem_helper'
require 'rake/extensiontask'
require 'packnga'

if YAML.const_defined?(:ENGINE)
  begin
    YAML::ENGINE.yamler = "psych"
  rescue LoadError
  end
end

base_dir = File.join(File.dirname(__FILE__))

groonga_ext_dir = File.join(base_dir, "ext", "groonga")
groonga_lib_dir = File.join(base_dir, 'lib')
$LOAD_PATH.unshift(groonga_ext_dir)
$LOAD_PATH.unshift(groonga_lib_dir)
ENV["RUBYLIB"] = "#{groonga_lib_dir}:#{groonga_ext_dir}:#{ENV['RUBYLIB']}"

helper = Bundler::GemHelper.new(base_dir)
helper.install
spec = helper.gemspec

Gem::PackageTask.new(spec) do |pkg|
  pkg.need_tar_gz = true
end

document_task = Packnga::DocumentTask.new(spec) do |t|
  t.yard do |yard_task|
    yard_task.options = ["--use-cache", ".yardoc"]
  end
end

namespace :reference do
  namespace :publication do
    task :keep_compatible do
      File.open(document_task.htaccess, "a") do |file|
        file.puts("Redirect permanent /#{spec.name}/text/TUTORIAL_ja_rdoc.html " +
                  "#{spec.homepage}#{spec.name}/ja/file.tutorial.html")
      end
    end

    task :generate => :keep_compatible
  end
end

Packnga::ReleaseTask.new(spec) do |task|
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

relative_vendor_dir = "vendor"
relative_binary_dir = File.join("vendor", "local")
vendor_dir = File.join(base_dir, relative_vendor_dir)
binary_dir = File.join(base_dir, relative_binary_dir)

groonga_win32_i386_p = ENV["GROONGA64"] != "yes"

namespace :win32 do
  namespace :groonga do
    task :download do
      require "open-uri"
      require "rroonga-build"
      groonga_version = RroongaBuild::RequiredGroongaVersion::VERSION.join(".")
      base_name = "groonga-#{groonga_version}-"
      if groonga_win32_i386_p
        base_name << "x86"
      else
        base_name << "x64"
      end
      base_name << ".zip"
      base_url = "http://packages.groonga.org/windows/groonga/"
      Dir.chdir(base_dir) do
        unless File.exist?(base_name)
          open("#{base_url}#{base_name}", "rb") do |zip|
            File.open(base_name, "wb") do |output|
              output.print(zip.read)
            end
          end
        end
        sh("unzip", base_name)
        rm_rf(relative_binary_dir)
        mkdir_p(File.dirname(relative_binary_dir))
        mv(File.basename(base_name, ".*"), relative_binary_dir)
      end
    end
  end
end

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
        _spec.files += collect_binary_files(relative_binary_dir)
      end
    end
  end
end

file "Makefile" => ["extconf.rb", "ext/groonga/extconf.rb"] do
  ruby("extconf.rb")
end

desc "Configure"
task :configure => "Makefile"

desc "Build"
task :build => :configure do
  sh("make")
end

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

task :default => :test
