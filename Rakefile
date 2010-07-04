# -*- coding: utf-8; mode: ruby -*-
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

require 'find'
require 'fileutils'
require 'pathname'
require 'erb'
require 'rubygems'
gem 'rdoc'
require 'hoe'
require 'rake/extensiontask'

ENV["NODOT"] = "yes"

base_dir = File.join(File.dirname(__FILE__))
truncate_base_dir = Proc.new do |x|
  x.gsub(/^#{Regexp.escape(base_dir + File::SEPARATOR)}/, '')
end

groonga_ext_dir = File.join(base_dir, "ext", "groonga")
groonga_lib_dir = File.join(base_dir, 'lib')
$LOAD_PATH.unshift(groonga_ext_dir)
$LOAD_PATH.unshift(groonga_lib_dir)
ENV["RUBYLIB"] = "#{groonga_lib_dir}:#{groonga_ext_dir}:#{ENV['RUBYLIB']}"

def guess_version
  require 'groonga'
  Groonga.bindings_version
end

manifest = File.join(base_dir, "Manifest.txt")
manifest_contents = []
base_dir_included_components = %w(AUTHORS Rakefile
                                  README.rdoc README.ja.rdoc
                                  NEWS.rdoc NEWS.ja.rdoc
                                  rroonga-build.rb extconf.rb pkg-config.rb)
excluded_components = %w(.cvsignore .gdb_history CVS depend Makefile doc pkg
                         .svn .git doc data .test-result tmp)
excluded_suffixes = %w(.png .ps .pdf .o .so .a .txt .~)
unless ENV["RUBY_CC_VERSION"]
  excluded_components << "vendor"
end
Find.find(base_dir) do |target|
  target = truncate_base_dir[target]
  components = target.split(File::SEPARATOR)
  if components.size == 1 and !File.directory?(target)
    next unless base_dir_included_components.include?(components[0])
  end
  Find.prune if (excluded_components - components) != excluded_components
  next if excluded_suffixes.include?(File.extname(target))
  manifest_contents << target if File.file?(target)
end

File.open(manifest, "w") do |f|
  f.puts manifest_contents.sort.join("\n")
end

# For Hoe's no user friendly default behavior. :<
File.open("README.txt", "w") {|file| file << "= Dummy README\n== XXX\n"}
FileUtils.cp("NEWS.rdoc", "History.txt")
at_exit do
  FileUtils.rm_f("README.txt")
  FileUtils.rm_f("History.txt")
  FileUtils.rm_f(manifest)
end

def cleanup_white_space(entry)
  entry.gsub(/(\A\n+|\n+\z)/, '') + "\n"
end

ENV["VERSION"] ||= guess_version
version = ENV["VERSION"]
project = nil
Hoe.spec('rroonga') do
  Hoe::Test::SUPPORTED_TEST_FRAMEWORKS[:testunit2] = "test/run-test.rb"
  project = self
  project.version = version
  project.rubyforge_name = 'groonga'
  authors = File.join(base_dir, "AUTHORS")
  project.author = File.readlines(authors).collect do |line|
    if /\s*<[^<>]*>$/ =~ line
      $PREMATCH
    else
      nil
    end
  end.compact
  project.email = ['groonga-users-en@rubyforge.org',
                   'groonga-dev@lists.sourceforge.jp']
  project.url = 'http://groonga.rubyforge.org/'
  project.testlib = :testunit2
  project.test_globs = ["test/run-test.rb"]
  project.spec_extras = {
    :extensions => ['extconf.rb'],
    :require_paths => ["lib", "ext/groonga"],
    :extra_rdoc_files => Dir.glob("**/*.rdoc"),
  }
  project.readme_file = "README.ja.rdoc"

  news_of_current_release = File.read("NEWS.rdoc").split(/^==\s.*$/)[1]
  project.changes = cleanup_white_space(news_of_current_release)

  entries = File.read("README.rdoc").split(/^==\s(.*)$/)
  description = cleanup_white_space(entries[entries.index("Description") + 1])
  project.summary, project.description, = description.split(/\n\n+/, 3)

  project.remote_rdoc_dir = "rroonga"
end

project.spec.dependencies.delete_if {|dependency| dependency.name == "hoe"}

ObjectSpace.each_object(Rake::RDocTask) do |rdoc_task|
  options = rdoc_task.options
  t_option_index = options.index("--title") || options.index("-t")
  rdoc_task.options[t_option_index, 2] = nil
  rdoc_task.title = "rroonga - #{version}"

  rdoc_task.rdoc_files = ["ext/rb-groonga.c"] + Dir.glob("ext/rb-grn-*.c")
  rdoc_task.rdoc_files += Dir.glob("lib/**/*.rb")
  rdoc_task.rdoc_files += Dir.glob("**/*.rdoc")
end

Rake::ExtensionTask.new("groonga", project.spec) do |ext|
  ext.cross_compile = true
  ext.cross_platform = 'x86-mingw32'
end

task :publish_docs => [:prepare_docs_for_publishing]


include ERB::Util

def apply_template(file, head, header, footer)
  content = File.read(file)
  content = content.sub(/lang="en"/, 'lang="ja"')

  title = nil
  content = content.sub(/<title>(.+?)<\/title>/) do
    title = $1
    head.result(binding)
  end

  content = content.sub(/<body(?:.*?)>/) do |body_start|
    "#{body_start}\n#{header.result(binding)}\n"
  end

  content = content.sub(/<\/body/) do |body_end|
    "\n#{footer.result(binding)}\n#{body_end}"
  end

  File.open(file, "w") do |file|
    file.print(content)
  end
end

def erb_template(name)
  file = File.join("html", "#{name}.html.erb")
  template = File.read(file)
  erb = ERB.new(template, nil, "-")
  erb.filename = file
  erb
end

task :prepare_docs_for_publishing do
  head = erb_template("head")
  header = erb_template("header")
  footer = erb_template("footer")
  Find.find("doc") do |file|
    if /\.html\z/ =~ file and /_(?:c|rb)\.html\z/ !~ file
      apply_template(file, head, header, footer)
    end
  end
end

task :publish_html do
  config = YAML.load(File.read(File.expand_path("~/.rubyforge/user-config.yml")))
  host = "#{config["username"]}@rubyforge.org"

  rsync_args = "-av --exclude '*.erb' --exclude '*.svg' --exclude .svn"
  remote_dir = "/var/www/gforge-projects/#{project.rubyforge_name}/"
  sh "rsync #{rsync_args} html/ #{host}:#{remote_dir}"
end

task :tag do
  sh("git tag -a #{version} -m 'release #{version}!!!'")
end

# fix Hoe's incorrect guess.
project.spec.executables.clear
# project.lib_files = project.spec.files.grep(%r|^src/lib/|)

task(:release).prerequisites.reject! {|name| name == "clean"}
