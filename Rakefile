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

require "find"
require "fileutils"
require "pathname"
require "erb"
require "rubygems"
require "rubygems/package_task"
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

Gem::PackageTask.new(spec) do |pkg|
  pkg.need_tar_gz = true
end

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

def collect_gcc_related_dll_files
  dll_files = []
  dll_names = ["libwinpthread-1.dll"]
  dll_names.each do |dll_name|
    dll_files << absolete_gcc_dll_path(dll_name)
  end
  dll_files
end

def absolete_gcc_dll_path(dll_name)
  `#{build_host}-gcc -print-file-name=#{dll_name}`.strip
end

def build_host
  if groonga_win32_i386_p
    "i686-w64-mingw32"
  else
    "x86_64-w64-mingw32"
  end
end

relative_vendor_dir = "vendor"
relative_binary_dir = File.join("vendor", "local")
vendor_dir = File.join(base_dir, relative_vendor_dir)
binary_dir = File.join(base_dir, relative_binary_dir)

def groonga_win32_i386_p
  ENV["RROONGA_USE_GROONGA_X64"].nil?
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
        binary_path = "#{relative_binary_dir}/bin"
        directory binary_path
        dll_files = collect_gcc_related_dll_files
        dll_files.each do |dll_file|
          cp dll_file, binary_path
        end
        binary_files = collect_binary_files(relative_binary_dir)
        _spec.files += binary_files
        stage_path = "#{ext.tmp_dir}/#{_spec.platform}/stage"
        binary_files.each do |binary_file|
          stage_binary_file = "#{stage_path}/#{binary_file}"
          stage_binary_dir = File.dirname(stage_binary_file)
          directory stage_binary_dir
          file stage_binary_file => [stage_binary_dir, binary_file] do
            cp binary_file, stage_binary_file
          end
        end
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

task :default => :test
