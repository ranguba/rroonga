# -*- mode: ruby; coding: utf-8 -*-
#
# Copyright (C) 2012-2013  Kouhei Sutou <kou@clear-code.com>
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

base_dir = File.dirname(__FILE__)
ext_dir = File.join(base_dir, "ext", "groonga")

guess_version = lambda do |ext_dir|
  version = {}
  File.open(File.join(ext_dir, "rb-grn.h")) do |rb_grn_h|
    rb_grn_h.each_line do |line|
      case line
      when /\A#define RB_GRN_([A-Z]+)_VERSION (\d+)/
        version[$1.downcase] = $2
      end
    end
  end
  [version["major"], version["minor"], version["micro"]].join(".")
end

clean_white_space = lambda do |entry|
  entry.gsub(/(\A\n+|\n+\z)/, '') + "\n"
end

Gem::Specification.new do |s|
  s.name = "rroonga"
  s.version = guess_version.call(ext_dir)

  authors_path = File.join(base_dir, "AUTHORS")
  authors = []
  emails = []
  File.readlines(authors_path).each do |line|
    if /\s*<([^<>]*)>$/ =~ line
      authors << $PREMATCH
      emails << $1
    end
  end

  s.authors = authors
  s.email = emails

  readme_path = File.join(base_dir, "README.md")
  entries = File.read(readme_path).split(/^##\s(.*)$/)
  description = clean_white_space.call(entries[entries.index("Description") + 1])
  s.summary, s.description, = description.split(/\n\n+/, 3)

  s.files = ["README.md", "AUTHORS", "Rakefile", "Gemfile", ".yardopts"]
  s.files += Dir.glob("doc/text/*.textile")
  s.files += Dir.glob("doc/images/*")
  s.files += ["#{s.name}.gemspec"]
  s.files += ["rroonga-build.rb", "extconf.rb"]
  Dir.chdir(base_dir) do
    s.files += Dir.glob("{lib,benchmark,misc,example}/**/*.rb")
    s.files += Dir.glob("ext/**/*.{c,h,rb,def}")
    s.extensions = ["ext/groonga/extconf.rb"]
    s.extra_rdoc_files = ["README.md"]
    s.test_files = Dir.glob("test/**/*.rb")
    Dir.chdir("bin") do
      s.executables = Dir.glob("*")
    end
  end

  s.homepage = "http://ranguba.org/#about-rroonga"
  s.licenses = ["LGPLv2"]
  s.require_paths = ["lib", "ext/groonga"]
  s.rubyforge_project = "groonga"

  s.required_ruby_version = ">= 1.9.3"

  s.add_runtime_dependency("pkg-config")
  s.add_runtime_dependency("groonga-client", ">= 0.0.3")
  s.add_runtime_dependency("json")
  s.add_runtime_dependency("archive-zip")
  s.add_development_dependency("test-unit", [">= 3.0.0"])
  s.add_development_dependency("test-unit-notify")
  s.add_development_dependency("rake")
  s.add_development_dependency("rake-compiler", [">= 0.9.5"])
  s.add_development_dependency("bundler")
  s.add_development_dependency("yard")
  s.add_development_dependency("packnga", [">= 1.0.0"])
  s.add_development_dependency("kramdown")
end

