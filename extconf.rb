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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

require 'English'
require 'pathname'
require 'fileutils'
require 'mkmf'

base_dir = Pathname(__FILE__).dirname.realpath

source_ext_dir = Pathname("ext") + "groonga"
FileUtils.mkdir_p(source_ext_dir.to_s)

require 'rbconfig'
ext_dir = base_dir + "ext" + "groonga"
definitions = ""
Dir.chdir(source_ext_dir.to_s) do
  config = Proc.new do |key|
    RbConfig::CONFIG[key]
  end
  ruby = "#{config['bindir']}/#{config['ruby_install_name']}#{config['EXEEXT']}"
  message("checking in #{ext_dir}...\n")
  ENV["INSTALL_RB"] = "yes"
  system(ruby, (ext_dir + 'extconf.rb').to_s, *ARGV) or exit 1
  message("checking in #{ext_dir}: done.\n")
  File.open("Makefile") do |file|
    file.each_line do |line|
      case line
      when /\A\w+\s*=/
        definitions << line
      end
    end
  end
end

message("creating top-level Makefile\n")
File.open("Makefile", "w") do |makefile|
  makefile.puts(definitions)
  makefile.puts
  targets = ["all", "clean", "install"]
  targets.each do |target|
    # overriding RUBYARCHDIR and RUBYLIBDIR for RubyGems.
    makefile.puts <<-EOM
#{target}:
	cd #{source_ext_dir}; \\
	  $(MAKE) $(MAKE_ARGS) \\
	    RUBYARCHDIR=$(RUBYARCHDIR) \\
	    RUBYLIBDIR=$(RUBYLIBDIR) \\
	    #{target}
EOM
  end
end
