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

require 'English'
require 'pathname'
require 'mkmf'

begin
  require 'pkg-config'
rescue LoadError
  require 'rubygems'
  require 'pkg-config'
end

base_dir = Pathname(__FILE__).dirname.parent.parent
$LOAD_PATH.unshift(base_dir.to_s)

require 'rroonga-build'

include RroongaBuild

package_name = "groonga"
module_name = "groonga"
major, minor, micro = RequiredGroongaVersion::VERSION

checking_for(checking_message("GCC")) do
  if macro_defined?("__GNUC__", "")
    $CFLAGS += ' -Wall'
    true
  else
    false
  end
end

checking_for(checking_message("Win32 OS")) do
  win32 = /cygwin|mingw|mswin/ =~ RUBY_PLATFORM
  if win32
    $defs << "-DRB_GRN_PLATFORM_WIN32"
    import_library_name = "libruby-#{module_name}.a"
    $DLDFLAGS << " -Wl,--out-implib=#{import_library_name}"
    $cleanfiles << import_library_name
    binary_base_dir = base_dir + "vendor" + "local"
    pkg_config_dir = binary_base_dir + "lib" + "pkgconfig"
    PKGConfig.add_path(pkg_config_dir.to_s)
    PKGConfig.set_override_variable("prefix", binary_base_dir.to_s)
  end
  win32
end

def install_groonga_locally(major, minor, micro)
  require 'open-uri'
  require 'shellwords'

  tar_gz = "groonga-#{major}.#{minor}.#{micro}.tar.gz"
  FileUtils.mkdir_p(local_groonga_base_dir)

  install_dir = local_groonga_install_dir
  Dir.chdir(local_groonga_base_dir) do
    url = "http://packages.groonga.org/source/groonga/#{tar_gz}"
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
  end
  unless PKGConfig.have_package(package_name, major, minor, micro)
    exit(false)
  end
  add_rpath_for_local_groonga
end

real_version = PKGConfig.modversion(package_name)
real_major, real_minor, real_micro = real_version.split(/\./)

$defs << "-DRB_GRN_COMPILATION"

$defs << "-DGRN_MAJOR_VERSION=#{real_major}"
$defs << "-DGRN_MINOR_VERSION=#{real_minor}"
$defs << "-DGRN_MICRO_VERSION=#{real_micro}"

have_header("ruby/st.h") unless have_macro("HAVE_RUBY_ST_H", "ruby.h")
have_func("rb_errinfo", "ruby.h")
have_type("enum ruby_value_type", "ruby.h")

checking_for(checking_message("debug flag")) do
  debug = with_config("debug")
  if debug
    debug_flag = "-DRB_GRN_DEBUG"
    $defs << debug_flag unless $defs.include?(debug_flag)
  end
  debug
end

if ENV["INSTALL_RB"] == "yes"
  $INSTALLFILES ||= []
  $INSTALLFILES << ["../../lib/**/*.rb", "$(RUBYLIBDIR)", "../../lib"]
end

create_makefile(module_name)
