#!/usr/bin/env ruby
#
# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
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
ext_dir_name = "src"
src_dir = File.join(File.expand_path(File.dirname(__FILE__)), ext_dir_name)
major, minor, micro = 0, 0, 3

PKGConfig.have_package(package_name, major, minor, micro) or exit 1
$defs << "-DGRN_VERSION=\\\"#{PKGConfig.modversion(package_name)}\\\""

have_header("ruby/st.h") unless have_macro("HAVE_RUBY_ST_H", "ruby.h")
have_func("rb_errinfo", "ruby.h")
have_type("enum ruby_value_type", "ruby.h")

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
