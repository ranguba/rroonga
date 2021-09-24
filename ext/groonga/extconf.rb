#!/usr/bin/env ruby
#
# Copyright (C) 2009-2021  Sutou Kouhei <kou@clear-code.com>
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

require "English"
require "pathname"
require "mkmf"
require "fileutils"
require "shellwords"
require "open-uri"
require "uri"

require "native-package-installer"
require "pkg-config"

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

def install_groonga_locally
  FileUtils.mkdir_p(local_groonga_base_dir)

  Dir.chdir(local_groonga_base_dir) do
    build_groonga
  end

  prepend_pkg_config_path_for_local_groonga
end

def download(url)
  message("downloading %s...", url)
  base_name = File.basename(url)
  if File.exist?(base_name)
    message(" skip (use downloaded file)\n")
  else
    options = {}
    proxy_env = ENV["http_proxy"]
    if proxy_env
      proxy_url = URI.parse(proxy_env)
      if proxy_url.user
        options[:proxy_http_basic_authentication] = [
          proxy_url,
          proxy_url.user,
          proxy_url.password,
        ]
      end
    end
    URI.open(url, "rb", *options) do |input|
      File.open(base_name, "wb") do |output|
        while (buffer = input.read(1024))
          output.print(buffer)
        end
      end
    end
    message(" done\n")
  end
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
  command_line.concat("--enable-debug") if debug_build_p
  command_line << "--prefix=#{prefix}"
  command_line << "--disable-static"
  command_line << "--disable-document"
  command_line << "--disable-benchmark"
  command_line << "--disable-groonga-httpd"
  command_line << "--without-cutter"
  escaped_command_line = command_line.collect do |command|
    Shellwords.escape(command)
  end
  custom_command_line_options = with_config("groonga-configure-options")
  if custom_command_line_options.is_a?(String)
    escaped_command_line << custom_command_line_options
  end
  escaped_command_line.join(" ")
end

def guess_make
  env_make = ENV["MAKE"]
  return env_make if env_make

  candidates = ["gmake", "make"]
  candidates.each do |candidate|
    (ENV["PATH"] || "").split(File::PATH_SEPARATOR).each do |path|
      return candidate if File.executable?(File.join(path, candidate))
    end
  end

  "make"
end

def n_processors
  proc_file = "/proc/cpuinfo"
  if File.exist?(proc_file)
    File.readlines(proc_file).grep(/^processor/).size
  elsif /darwin/ =~ RUBY_PLATFORM
    `sysctl -n hw.ncpu`.to_i
  else
    1
  end
end

def install_for_gnu_build_system(install_dir)
  make = guess_make
  run_command("configuring...",
              configure_command_line(install_dir))
  run_command("building (maybe long time)...",
              "#{make} -j#{n_processors}")
  run_command("installing...",
              "#{make} install")
end

def build_groonga_from_git
  message("removing old cloned repository...")
  FileUtils.rm_rf("groonga")
  message(" done\n")

  repository_url = "https://github.com/groonga/groonga.git"
  run_command("cloning...",
              "git clone --recursive --depth 1 #{repository_url}")

  Dir.chdir("groonga") do
    run_command("running autogen.sh...",
                "./autogen.sh")
    install_for_gnu_build_system(local_groonga_install_dir)
  end

  message("removing cloned repository...")
  FileUtils.rm_rf("groonga")
  message(" done\n")
end

def build_groonga_from_tar_gz
  tar_gz = "groonga-latest.tar.gz"
  url = "https://packages.groonga.org/source/groonga/#{tar_gz}"
  groonga_source_dir = "groonga-latest"

  download(url)

  FileUtils.rm_rf(groonga_source_dir)
  FileUtils.mkdir_p(groonga_source_dir)

  message("extracting...")
  # TODO: Use Zlip::GzipReader and Gem::Package::TarReader
  if xsystem("tar xfz #{tar_gz} -C #{groonga_source_dir} --strip-components=1")
    message(" done\n")
  else
    message(" failed\n")
    exit(false)
  end

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

def build_groonga
  if RequiredGroongaVersion::RELEASED_DATE > Time.now
    build_groonga_from_git
  else
    build_groonga_from_tar_gz
  end
end

def install_local_groonga(package_name, major, minor, micro)
  unless have_local_groonga?(package_name, major, minor, micro)
    install_groonga_locally
  end
  unless PKGConfig.have_package(package_name, major, minor, micro)
    exit(false)
  end
  add_rpath_for_local_groonga
end

need_auto_groonga_install = false
unless PKGConfig.have_package(package_name, major, minor, micro)
  if NativePackageInstaller.install(debian: "libgroonga-dev",
                                    homebrew: "groonga",
                                    msys2: "groonga")
    need_auto_groonga_install =
      !PKGConfig.have_package(package_name, major, minor, micro)
  else
    need_auto_groonga_install = true
  end
end
if need_auto_groonga_install
  install_local_groonga(package_name, major, minor, micro)
end

real_version = PKGConfig.modversion(package_name)
real_major, real_minor, real_micro = real_version.split(/\./)

$defs << "-DRB_GRN_COMPILATION"

$defs << "-DGRN_MAJOR_VERSION=#{real_major}"
$defs << "-DGRN_MINOR_VERSION=#{real_minor}"
$defs << "-DGRN_MICRO_VERSION=#{real_micro}"

have_header("ruby/st.h") unless have_macro("HAVE_RUBY_ST_H", "ruby.h")
have_func("rb_errinfo", "ruby.h")
have_func("rb_sym2str", "ruby.h")
have_func("rb_to_symbol", "ruby.h")
have_func("rb_ary_new_from_args", "ruby.h")
have_func("rb_ary_new_from_values", "ruby.h")
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
    flags.gsub(/(^|\s)?-g\d?(\s|$)/, '\\1-ggdb3\\2')
  else
    flags
  end
end

checking_for(checking_message("--enable-debug-build option")) do
  enable_debug_build = enable_config("debug-build", false)
  if enable_debug_build
    $CFLAGS = disable_optimization_build_flag($CFLAGS)
    $CFLAGS = enable_debug_build_flag($CFLAGS)

    CONFIG["optflags"] = disable_optimization_build_flag(CONFIG["optflags"])
    CONFIG["debugflags"] = enable_debug_build_flag(CONFIG["debugflags"])

    CONFIG["CXXFLAGS"] = disable_optimization_build_flag(CONFIG["CXXFLAGS"])
    CONFIG["CXXFLAGS"] = enable_debug_build_flag(CONFIG["CXXFLAGS"])
  end
  enable_debug_build
end

checking_for(checking_message("--enable-untyped-data-warning option")) do
  enable_check_untyped_data = enable_config("untyped-data-warning", false)
  if enable_check_untyped_data
    $defs << "-DRUBY_UNTYPED_DATA_WARNING"
  end
  enable_check_untyped_data
end

if ENV["INSTALL_RB"] == "yes"
  $INSTALLFILES ||= []
  $INSTALLFILES << ["../../lib/**/*.rb", "$(RUBYLIBDIR)", "../../lib"]
end

create_makefile(module_name)
