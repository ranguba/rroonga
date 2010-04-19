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

base_dir = Pathname(__FILE__).dirname.parent.parent
$LOAD_PATH.unshift(base_dir.to_s)

require 'English'
require 'mkmf'
require 'pkg-config'
require 'rroonga-build'

include RroongaBuild

package_name = "groonga"
module_name = "groonga"
major, minor, micro = RequiredGroongaVersion::VERSION

unless PKGConfig.have_package(package_name, major, minor, micro)
  have_local_groonga?(package_name, major, minor, micro) or exit 1
  add_rpath_for_local_groonga
end

real_version = PKGConfig.modversion(package_name)
real_major, real_minor, real_micro = real_version.split(/\./)

$defs << "-DRB_GRN_COMPILATION"

$defs << "-DGRN_MAJOR_VERSION=#{real_major}"
$defs << "-DGRN_MINOR_VERSION=#{real_minor}"
$defs << "-DGRN_MICRO_VERSION=#{real_micro}"

gcc = false
checking_for(checking_message("GCC")) do
  if macro_defined?("__GNUC__", "")
    $CFLAGS += ' -Wall'
    gcc = true
    true
  else
    false
  end
end

checking_for(checking_message("Win32 OS")) do
  win32 = /cygwin|mingw|mswin32/ =~ RUBY_PLATFORM
  if win32
    $defs << "-DRB_GRN_PLATFORM_WIN32"
    import_library_name = "libruby-#{module_name}.a"
    $DLDFLAGS << " -Wl,--out-implib=#{import_library_name}"
    $cleanfiles << import_library_name
    local_groonga_install_dir = base_dir + "vendor" + "local"
    $CFLAGS += " -I#{local_groonga_install_dir}/include"
    if gcc
      $DLDFLAGS << " -L#{local_groonga_install_dir}/lib"
    else
      $libs += " #{local_groonga_install_dir}/lib/libgroonga.lib"
    end
  end
  win32
end

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

$INSTALLFILES ||= []
$INSTALLFILES << ["../../lib/**/*.rb", "$(RUBYLIBDIR)", "../../lib"]

create_makefile(module_name)
