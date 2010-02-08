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

$LOAD_PATH.unshift(File.dirname(__FILE__))

require 'English'
require 'mkmf'
require 'pkg-config'
require 'fileutils'
require 'shellwords'

checking_for(checking_message("GCC")) do
  if macro_defined?("__GNUC__", "")
    $CFLAGS += ' -Wall'
    true
  else
    false
  end
end

package_name = "groonga"
module_name = "groonga"
ext_dir_name = "ext"
src_dir = File.join(File.expand_path(File.dirname(__FILE__)), ext_dir_name)
major, minor, micro = 0, 1, 5

def local_groonga_base_dir
  File.join(File.dirname(__FILE__), "vendor")
end

def local_groonga_install_dir
  File.expand_path(File.join(local_groonga_base_dir, "local"))
end

def have_local_groonga?(package_name, major, minor, micro)
  return false unless File.exist?(File.join(local_groonga_install_dir, "lib"))

  prepend_pkg_config_path_for_local_groonga
  PKGConfig.have_package(package_name, major, minor, micro)
end

def prepend_pkg_config_path_for_local_groonga
  pkg_config_dir = File.join(local_groonga_install_dir, "lib", "pkgconfig")
  PackageConfig.prepend_default_path(pkg_config_dir)

  lib_dir = File.join(local_groonga_install_dir, "lib")
  original_LDFLAGS = $LDFLAGS
  checking_for(checking_message("-Wl,-rpath is available")) do
    $LDFLAGS += " -Wl,-rpath,#{Shellwords.escape(lib_dir)}"
    available = try_compile("int main() {return 0;}")
    $LDFLAGS = original_LDFLAGS unless available
    available
  end
end

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
      if [major, minor, micro] == [0, 1, 5]
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

real_version = PKGConfig.modversion(package_name)
real_major, real_minor, real_micro = real_version.split(/\./)
$defs << "-DGRN_MAJOR_VERSION=#{real_major}"
$defs << "-DGRN_MINOR_VERSION=#{real_minor}"
$defs << "-DGRN_MICRO_VERSION=#{real_micro}"

checking_for(checking_message("debug flag")) do
  debug = with_config("debug")
  if debug
    debug_flag = "-DRB_GRN_DEBUG"
    $defs << debug_flag unless $defs.include?(debug_flag)
  end
  debug
end

have_header("ruby/st.h") unless have_macro("HAVE_RUBY_ST_H", "ruby.h")
have_func("rb_errinfo", "ruby.h")
have_type("enum ruby_value_type", "ruby.h")

$INSTALLFILES ||= []
$INSTALLFILES << ["../lib/**/*.rb", "$(RUBYLIBDIR)", "../lib"]

create_makefile(module_name, src_dir)

makefile = File.read("Makefile")
File.open("Makefile", "w") do |f|
  objs = []
  co = nil
  makefile.each_line do |line|
    case line
    when /^DLLIB\s*=\s*/
      dllib = $POSTMATCH
      f.puts("DLLIB = #{ext_dir_name}/#{dllib}")
      f.puts("IMPLIB = #{ext_dir_name}/libruby-#{dllib.gsub(/\..+?$/, '.lib')}")
    when /^(SRCS)\s*=\s*/
      name = $1
      vars = $POSTMATCH.split.collect {|var| "$(srcdir)/#{var}"}.join(" ")
      f.puts("#{name} = #{vars}")
    when /^(OBJS|CLEANLIBS|CLEANOBJS)\s*=\s*/
      name = $1
      vars = $POSTMATCH.split.collect {|var| "#{ext_dir_name}/#{var}"}
      objs = vars if name == "OBJS"
      vars = vars.join(" ")
      f.puts("#{name} = #{vars}")
    when /^\t\$\(CC\)/
      if PKGConfig.msvc?
        output_option = "-Fo"
      else
        output_option = "-o"
      end
      unless /#{Regexp.escape(output_option)}/ =~ line
        line = "#{line.chomp} #{output_option}$@"
      end
      co = line
      f.puts(line)
    else
      f.puts(line)
    end
  end

  if co and !objs.empty?
    f.puts
    if PKGConfig.msvc?
      f.puts "{$(srcdir)}.c{#{ext_dir_name}}.obj:"
      f.puts co
    else
      objs.each do |obj|
        f.puts "#{obj}: $(srcdir)/#{File.basename(obj).sub(/\..+?$/, '.c')}"
        f.puts co
      end
    end
  end
end

FileUtils.mkdir_p(ext_dir_name)
