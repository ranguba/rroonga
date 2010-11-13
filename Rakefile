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

def guess_version(groonga_ext_dir)
  version = {}
  File.open(File.join(groonga_ext_dir, "rb-grn.h")) do |rb_grn_h|
    rb_grn_h.each_line do |line|
      case line
      when /\A#define RB_GRN_([A-Z]+)_VERSION (\d+)/
        version[$1.downcase] = $2
      end
    end
  end
  [version["major"], version["minor"], version["micro"]].join(".")
end

manifest = File.join(base_dir, "Manifest.txt")
manifest_contents = []
base_dir_included_components = %w(AUTHORS Rakefile
                                  README.rdoc README.ja.rdoc
                                  NEWS.rdoc NEWS.ja.rdoc
                                  rroonga-build.rb extconf.rb)
excluded_components = %w(.cvsignore .gdb_history CVS depend Makefile doc pkg
                         .svn .git doc data .test-result tmp vendor)
excluded_suffixes = %w(.png .ps .pdf .o .so .a .txt .~)
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

ENV["VERSION"] ||= guess_version(groonga_ext_dir)
version = ENV["VERSION"]
project = nil
Hoe.spec('rroonga') do
  Hoe::Test::SUPPORTED_TEST_FRAMEWORKS[:testunit2] = "test/run-test.rb"
  project = self
  project.version = version.dup
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
  project.extra_deps << ['pkg-config', '>= 0']
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

relative_vendor_dir = "vendor"
relative_binary_dir = File.join("vendor", "local")
vendor_dir = File.join(base_dir, relative_vendor_dir)
binary_dir = File.join(base_dir, relative_binary_dir)
Rake::ExtensionTask.new("groonga", project.spec) do |ext|
  ext.cross_compile = true
  ext.cross_compiling do |spec|
    if /mingw|mswin/ =~ spec.platform.to_s
      binary_files = []
      Find.find(relative_binary_dir) do |name|
        next unless File.file?(name)
        next if /\.zip\z/i =~ name
        binary_files << name
      end
      spec.files += binary_files
    end
  end
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

desc "Publish HTML to Web site."
task :publish_html do
  config = YAML.load(File.read(File.expand_path("~/.rubyforge/user-config.yml")))
  host = "#{config["username"]}@rubyforge.org"

  rsync_args = "-av --exclude '*.erb' --exclude '*.svg' --exclude .svn"
  remote_dir = "/var/www/gforge-projects/#{project.rubyforge_name}/"
  sh "rsync #{rsync_args} html/ #{host}:#{remote_dir}"
end

desc "Tag the current revision."
task :tag do
  sh("git tag -a #{version} -m 'release #{version}!!!'")
end

# fix Hoe's incorrect guess.
project.spec.executables.clear
# project.lib_files = project.spec.files.grep(%r|^src/lib/|)

task(:release).prerequisites.reject! {|name| name == "clean"}

<<<<<<< HEAD
namespace :win32 do
  desc "Build MeCab and groonga and install them into vendor/local/."
  task(:build => :build_groonga)

  desc "Build MeCab and install it into vendor/local/."
  task(:build_mecab) do
    tmp_dir = "tmp/mecab"
    rm_rf(tmp_dir)
    mkdir_p(tmp_dir)
    require 'open-uri'
    mecab_version = "0.98"
    mecab_base = "mecab-#{mecab_version}"
    mecab_tar_gz = "#{mecab_base}.tar.gz"
    mecab_tar_gz_url = "http://sourceforge.net/projects/mecab/files/mecab/#{mecab_version}/#{mecab_tar_gz}/download"
    Dir.chdir(tmp_dir) do
      open(mecab_tar_gz_url) do |downloaded_tar_gz|
        File.open(mecab_tar_gz, "wb") do |tar_gz|
          tar_gz.print(downloaded_tar_gz.read)
        end
      end
      sh("tar", "xzf", mecab_tar_gz) or exit(false)
    end
    Dir.chdir(File.join(tmp_dir, mecab_base)) do
      sh("./configure",
         "--prefix=#{binary_dir}",
         "--host=i586-mingw32msvc") or exit(false)
      sh("make", "-j8") or exit(false)
      sh("make", "install") or exit(false)

      mecab_files_dir = File.join(vendor_dir, "mecab")
      mkdir_p(mecab_files_dir)
      files = ["AUTHORS", "BSD", "COPYING", "GPL", "LGPL"]
      cp(files, mecab_files_dir)
    end
  end

  task(:build_mecab_dict => :build_mecab) do
    tmp_dir = "tmp/mecab_dict"
    rm_rf(tmp_dir)
    mkdir_p(tmp_dir)
    require 'open-uri'
    naist_jdic_base = "mecab-naist-jdic-0.6.3-20100801"
    naist_jdic_tar_gz = "#{naist_jdic_base}.tar.gz"
    naist_jdic_tar_gz_url = "http://osdn.dl.sourceforge.jp/naist-jdic/48487/#{naist_jdic_tar_gz}"
    Dir.chdir(tmp_dir) do
      open(naist_jdic_tar_gz_url) do |downloaded_tar_gz|
        File.open(naist_jdic_tar_gz, "wb") do |tar_gz|
          tar_gz.print(downloaded_tar_gz.read)
        end
      end
      sh("tar", "xzf", naist_jdic_tar_gz) or exit(false)
    end
    Dir.chdir(File.join(tmp_dir, naist_jdic_base)) do
      sh("./configure",
         "--with-dicdir=#{binary_dir}/share/mecab/dic/naist-jdic",
         "--with-charset=utf-8") or exit(false)
      sh("make", "-j8") or exit(false)
      sh("make", "install-data") or exit(false)

      naist_jdic_files_dir = File.join(vendor_dir, "mecab-naist-jdic")
      mkdir_p(naist_jdic_files_dir)
      files = ["AUTHORS", "COPYING"]
      cp(files, naist_jdic_files_dir)
    end
    dictionary_dir = '$(rcpath)\..\share/mecab\dic\naist-jdic'
    mecab_rc_path = File.join(binary_dir, "etc", "mecabrc")
    mecab_rc_content = File.read(mecab_rc_path)
    File.open(mecab_rc_path, "w") do |mecab_rc|
      mecab_rc.print(mecab_rc_content.gsub(/\Adictdir\s*=.+$/,
                                           "dictdir = #{dictionary_dir}"))
    end
  end

  desc "Build groonga and install it into vendor/local/."
  task(:build_groonga => :build_mecab_dict) do
    tmp_dir = "tmp/groonga"
    rm_rf(tmp_dir)
    mkdir_p(tmp_dir)
    Dir.chdir(tmp_dir) do
      sh("git", "clone", "git://github.com/groonga/groonga.git") or exit(false)
    end
    Dir.chdir(File.join(tmp_dir, "groonga")) do
      sh("./autogen.sh") or exit(false)
      mecab_config = File.join(binary_dir, "bin", "mecab-config")
      sh("./configure",
         "--prefix=#{binary_dir}",
         "--host=i586-mingw32msvc",
         "--with-mecab-config=#{mecab_config}",
         "--without-cutter",
         "--disable-benchmark") or exit(false)
      sh("make", "-j8") or exit(false)
      sh("make", "install") or exit(false)

      groonga_files_dir = File.join(vendor_dir, "groonga")
      mkdir_p(groonga_files_dir)
      files = ["AUTHORS", "COPYING"]
      cp(files, groonga_files_dir)
    end
  end
end

desc "generate rroonga.gemspec"
task :generate_gemspec do
  spec = project.spec
  spec_name = File.join(base_dir, project.spec.spec_name)
  File.open(spec_name, "w") do |spec_file|
    spec_file.puts(spec.to_ruby)
  end
end
