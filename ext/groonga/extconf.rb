#!/usr/bin/env ruby
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

require "English"
require "pathname"
require "mkmf"
require "fileutils"
require "shellwords"
require "open-uri"

begin
  require "pkg-config"
rescue LoadError
  require "rubygems"
  require "pkg-config"
end

base_dir = Pathname(__FILE__).dirname.parent.parent.expand_path
$LOAD_PATH.unshift(base_dir.to_s)

require "rroonga-build"

include RroongaBuild

package_name = "groonga"
module_name = "groonga"
major, minor, micro = RequiredGroongaVersion::VERSION

checking_for(checking_message("GCC")) do
  if macro_defined?("__GNUC__", "")
    $CFLAGS += " -Wall"
    true
  else
    false
  end
end

def win32?
  /cygwin|mingw|mswin/ =~ RUBY_PLATFORM
end

checking_for(checking_message("Win32 OS")) do
  win32 = win32?
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
  FileUtils.mkdir_p(local_groonga_base_dir)

  Dir.chdir(local_groonga_base_dir) do
    if win32?
      extract_groonga_win32_binary(major, minor, micro)
    else
      build_groonga(major, minor, micro)
    end
  end

  prepend_pkg_config_path_for_local_groonga
end

def download(url)
  message("downloading %s...", url)
  base_name = File.basename(url)
  if File.exist?(base_name)
    message(" skip (use downloaded file)\n")
  else
    open(url, "rb") do |input|
      File.open(base_name, "wb") do |output|
        while (buffer = input.read(1024))
          output.print(buffer)
        end
      end
    end
    message(" done\n")
  end
end

def extract_zip(filename, destrination_dir)
  begin
    require "archive/zip"
  rescue LoadError
    require "rubygems"
    require "archive/zip"
  end

  Archive::Zip.extract(filename, destrination_dir)
end

def extract_groonga_win32_binary(major, minor, micro)
  if ENV["RROONGA_USE_GROONGA_X64"]
    architecture = "x64"
  else
    architecture = "x86"
  end
  zip = "groonga-#{major}.#{minor}.#{micro}-#{architecture}.zip"
  url = "http://packages.groonga.org/windows/groonga/#{zip}"
  install_dir = local_groonga_install_dir

  download(url)

  message("extracting...")
  extract_zip(zip, ".")
  message(" done\n")

  if File.exist?(install_dir)
    message("removing old install... #{install_dir}")
    FileUtils.rm_rf(install_dir)
    message(" done\n")
  end

  message("installing...")
  FileUtils.mv(File.basename(zip, ".zip"), install_dir)
  message(" done\n")

  message("removing binary archive...")
  FileUtils.rm_rf(zip)
  message(" done\n")
end

def run_command(start_message, command)
  message(start_message)
  if xsystem(command)
    message(" done\n")
  else
    message(" failed\n")
    exit(false)
  end
end

def configure_command_line(prefix)
  command_line = ["./configure"]
  debug_build_p = ENV["RROONGA_DEBUG_BUILD"] == "yes"
  debug_flags = ["CFLAGS=-g3 -O0", "CXXFLAGS=-g3 -O0"]
  command_line.concat(debug_flags) if debug_build_p
  command_line << "--prefix=#{prefix}"
  command_line << "--disable-static"
  command_line << "--disable-document"
  escaped_command_line = command_line.collect do |command|
    Shellwords.escape(command)
  end
  escaped_command_line.join(" ")
end

def install_for_gnu_build_system(install_dir)
  run_command("configuring...",
              configure_command_line(install_dir))
  run_command("building (maybe long time)...",
              "make")
  run_command("installing...",
              "make install")
end

def build_groonga_from_git(major, minor, micro)
  message("removing old cloned repository...")
  FileUtils.rm_rf("groonga")
  message(" done\n")

  run_command("cloning...",
              "git clone --depth 1 https://github.com/groonga/groonga")

  Dir.chdir("groonga") do
    run_command("running autogen.sh...",
                "./autogen.sh")
    install_for_gnu_build_system(local_groonga_install_dir)
  end

  message("removing cloned repository...")
  FileUtils.rm_rf("groonga")
  message(" done\n")
end

def build_groonga_from_tar_gz(major, minor, micro)
  tar_gz = "groonga-#{major}.#{minor}.#{micro}.tar.gz"
  url = "http://packages.groonga.org/source/groonga/#{tar_gz}"

  download(url)

  message("extracting...")
  if xsystem("tar xfz #{tar_gz}")
    message(" done\n")
  else
    message(" failed\n")
    exit(false)
  end

  groonga_source_dir = "groonga-#{major}.#{minor}.#{micro}"
  Dir.chdir(groonga_source_dir) do
    install_for_gnu_build_system(local_groonga_install_dir)
  end

  message("removing source...")
  FileUtils.rm_rf(groonga_source_dir)
  message(" done\n")

  message("removing source archive...")
  FileUtils.rm_rf(tar_gz)
  message(" done\n")
end

def build_groonga(major, minor, micro)
  if RequiredGroongaVersion::RELEASED_DATE > Time.now
    build_groonga_from_git(major, minor, micro)
  else
    build_groonga_from_tar_gz(major, minor, micro)
  end
end

unless PKGConfig.have_package(package_name, major, minor, micro)
  unless have_local_groonga?(package_name, major, minor, micro)
    required_version = [major, minor, micro]
    if (required_version <=> LatestGroongaVersion::VERSION) > 0
      target_version = LatestGroongaVersion::VERSION
    else
      target_version = required_version
    end
    install_groonga_locally(*target_version)
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

checking_for(checking_message("--enable-debug-log option")) do
  enable_debug_log = enable_config("debug-log", false)
  if enable_debug_log
    debug_flag = "-DRB_GRN_DEBUG"
    $defs << debug_flag unless $defs.include?(debug_flag)
  end
  enable_debug_log
end

def gcc?
  CONFIG["GCC"] == "yes"
end

def disable_optimization_build_flag(flags)
  if gcc?
    flags.gsub(/(^|\s)?-O\d(\s|$)?/, '\\1-O0\\2')
  else
    flags
  end
end

def enable_debug_build_flag(flags)
  if gcc?
    flags.gsub(/(^|\s)?-g\d?(\s|$)?/, '\\1-ggdb3\\2')
  else
    flags
  end
end

checking_for(checking_message("--enable-debug-build option")) do
  enable_debug_build = enable_config("debug-build", false)
  if enable_debug_build
    $CFLAGS = disable_optimization_build_flag($CFLAGS)
    $CFLAGS = enable_debug_build_flag($CFLAGS)

    CONFIG["CXXFLAGS"] = disable_optimization_build_flag(CONFIG["CXXFLAGS"])
    CONFIG["CXXFLAGS"] = enable_debug_build_flag(CONFIG["CXXFLAGS"])
  end
  enable_debug_build
end

if ENV["INSTALL_RB"] == "yes"
  $INSTALLFILES ||= []
  $INSTALLFILES << ["../../lib/**/*.rb", "$(RUBYLIBDIR)", "../../lib"]
end

create_makefile(module_name)
