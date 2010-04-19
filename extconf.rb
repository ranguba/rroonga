#!/usr/bin/env ruby
#
# Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>
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

require 'pathname'

base_dir = Pathname(__FILE__).dirname.realpath
$LOAD_PATH.unshift(base_dir.to_s)

require 'English'
require 'pkg-config'
require 'rroonga-build'
require 'fileutils'

include RroongaBuild

package_name = "groonga"
module_name = "groonga"
major, minor, micro = RequiredGroongaVersion::VERSION

def install_groonga_locally(major, minor, micro)
  require 'open-uri'
  require 'shellwords'

  tar_gz = "groonga-#{major}.#{minor}.#{micro}.tar.gz"
  FileUtils.mkdir_p(local_groonga_base_dir)

  install_dir = local_groonga_install_dir
  Dir.chdir(local_groonga_base_dir) do
    url = "http://groonga.org/files/groonga/#{tar_gz}"
    message("downloading %s...", url)
    open(url, "rb") do |input|
      File.open(tar_gz, "wb") do |output|
        while (buffer = input.read(1024))
          output.print(buffer)
        end
      end
    end
    message(" done\n")

    message("extracting...")
    if xsystem("tar xfz #{tar_gz}")
      message(" done\n")
    else
      message(" failed\n")
      exit 1
    end

    groonga_source_dir = "groonga-#{major}.#{minor}.#{micro}"
    Dir.chdir(groonga_source_dir) do
      message("configuring...")
      prefix = Shellwords.escape(install_dir)
      if xsystem("./configure CFLAGS='-g -O0' --prefix=#{prefix}")
        message(" done\n")
      else
        message(" failed\n")
        exit 1
      end

      message("building (maybe long time)...")
      if xsystem("make")
        message(" done\n")
      else
        message(" failed\n")
        exit 1
      end

      message("installing...")
      if [major, minor, micro] == [0, 1, 6]
        make_install_args = " MKDIR_P='mkdir -p --'"
      else
        make_install_args = ""
      end
      if xsystem("make install#{make_install_args}")
        message(" done\n")
      else
        message(" failed\n")
        exit 1
      end
    end
  end

  prepend_pkg_config_path_for_local_groonga
end

unless PKGConfig.have_package(package_name, major, minor, micro)
  unless have_local_groonga?(package_name, major, minor, micro)
    install_groonga_locally(major, minor, micro)
    PKGConfig.have_package(package_name, major, minor, micro) or exit 1
  end
end

source_ext_dir = Pathname("ext") + "groonga"
FileUtils.mkdir_p(source_ext_dir.to_s)

require 'rbconfig'
ext_dir = base_dir + "ext" + "groonga"
Dir.chdir(source_ext_dir.to_s) do
  config = Proc.new do |key|
    RbConfig::CONFIG[key]
  end
  ruby = "#{config['bindir']}/#{config['ruby_install_name']}#{config['EXEEXT']}"
  message("checking in #{ext_dir}...\n")
  system("#{ruby} #{ext_dir + 'extconf.rb'}") or exit 1
  message("checking in #{ext_dir}: done.\n")
end

message("creating top-level Makefile\n")
File.open("Makefile", "w") do |makefile|
  targets = ["all", "clean", "install"]
  targets.each do |target|
    makefile.puts <<-EOM
#{target}:
	cd #{source_ext_dir}; $(MAKE) $(MAKE_ARGS) #{target}
EOM
  end
end
