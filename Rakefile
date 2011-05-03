# -*- coding: utf-8; mode: ruby -*-
#
# Copyright (C) 2009-2011  Kouhei Sutou <kou@clear-code.com>
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
require 'yard'
require 'jeweler'
require 'rake/extensiontask'

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
spec = nil
Jeweler::Tasks.new do |_spec|
  spec = _spec
  spec.name = "rroonga"
  spec.version = version.dup
  spec.rubyforge_project = "groonga"
  spec.homepage = "http://groonga.rubyforge.org/"
  authors = File.join(base_dir, "AUTHORS")
  spec.authors = File.readlines(authors).collect do |line|
    if /\s*<[^<>]*>$/ =~ line
      $PREMATCH
    else
      nil
    end
  end.compact
  spec.email = [
    'groonga-users-en@rubyforge.org',
    'groonga-dev@lists.sourceforge.jp',
  ]
  entries = File.read("README.textile").split(/^h2\.\s(.*)$/)
  description = cleanup_white_space(entries[entries.index("Description") + 1])
  spec.summary, spec.description, = description.split(/\n\n+/, 3)
  spec.license = "LGPLv2"
end

Jeweler::RubygemsDotOrgTasks.new do
end

YARD::Rake::YardocTask.new do |task|
  task.options += ["--title", "#{spec.name} - #{version}"]
  # task.options += ["--charset", "UTF-8"]
  task.options += ["--readme", "README.textile"]
  task.options += ["--files", "text/expression.rdoc"]
  task.files += FileList["ext/groonga/**/rb-groonga.c"]
  task.files += FileList["ext/groonga/**/*.c"]
  task.files += FileList["lib/**/*.rb"]
  task.files += FileList["**/*.rdoc"]
end

def windows?(platform=nil)
  platform ||= RUBY_PLATFORM
  platform =~ /mswin(?!ce)|mingw|cygwin|bccwin/
end

def collect_binary_files(binary_dir)
  binary_files = []
  Find.find(binary_dir) do |name|
    next unless File.file?(name)
    next if /\.zip\z/i =~ name
    binary_files << name
  end
  binary_files
end

relative_vendor_dir = "vendor"
relative_binary_dir = File.join("vendor", "local")
vendor_dir = File.join(base_dir, relative_vendor_dir)
binary_dir = File.join(base_dir, relative_binary_dir)

Rake::ExtensionTask.new("groonga", spec) do |ext|
  if windows?
    ext.gem_spec.files += collect_binary_files(relative_binary_dir)
  else
    ext.cross_compile = true
    ext.cross_compiling do |spec|
      if windows?(spec.platform.to_s)
        spec.files += collect_binary_files(relative_binary_dir)
      end
    end
  end
end

include ERB::Util

def apply_template(file, head, header, footer, language)
  content = File.read(file)
  content = content.sub(/lang="en"/, 'lang="#{language}"')

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

def rsync_to_rubyforge(spec, source, destination)
  config = YAML.load(File.read(File.expand_path("~/.rubyforge/user-config.yml")))
  host = "#{config["username"]}@rubyforge.org"

  rsync_args = "-av --exclude '*.erb' --delete --dry-run"
  remote_dir = "/var/www/gforge-projects/#{spec.rubyforge_name}/"
  sh("rsync #{rsync_args} #{source} #{host}:#{remote_dir}#{destination}")
end

namespace :reference do
  translate_languages = [:ja]
  supported_languages = [:en, *translate_languages]
  reference_base_dir = "references"
  html_files = FileList["doc/**/*.html"].to_a

  directory reference_base_dir
  CLOBBER.include(reference_base_dir)

  po_dir = "po"
  namespace :pot do
    pot_file = "#{po_dir}/#{spec.name}.pot"

    directory po_dir
    file pot_file => ["po", *html_files] do |t|
      sh("xml2po", "--keep-entities", "--output", t.name, *html_files)
    end

    desc "Generates pot file."
    task :generate => pot_file
  end

  namespace :po do
    translate_languages.each do |language|
      namespace language do
        po_file = "#{po_dir}/#{language}.po"

        file po_file => html_files do |t|
          sh("xml2po", "--keep-entities", "--update", t.name, *html_files)
        end

        desc "Updates po file for #{language}."
        task :update => po_file
      end
    end
  end

  namespace :translate do
    directory reference_base_dir

    translate_languages.each do |language|
      po_file = "#{po_dir}/#{language}.po"
      translate_doc_dir = "#{reference_base_dir}/#{language}"

      doc_dir = Pathname("doc")
      desc "Translates documents to #{language}."
      task language => [po_file, reference_base_dir] do
        doc_dir.find do |path|
          base_path = path.relative_path_from(doc_dir)
          translated_path = "#{translate_doc_dir}/#{base_path}"
          if path.directory?
            mkdir_p(translated_path)
            next
          end
          case path.extname
          when ".html"
            sh("xml2po --keep-entities " +
               "--po-file #{po_file} --language #{language} " +
               "#{path} > #{translated_path}")
          else
            cp(path.to_s, translated_path, :preserve => true)
          end
        end
      end
    end
  end

  translate_task_names = translate_languages.collect do |language|
    "reference:translate:#{language}"
  end
  desc "Translates references."
  task :translate => translate_task_names

  desc "Generates references."
  task :generate => [:yard, :translate] do
    cp_r("doc", "#{reference_base_dir}/en", :preserve => true)
  end

  namespace :publication do
    task :prepare do
      head = erb_template("head")
      header = erb_template("header")
      footer = erb_template("footer")
      supported_languages.each do |language|
        doc_dir = "#{reference_base_dir}/#{language}"
        Find.find(doc_dir) do |file|
          if /\.html\z/ =~ file and /_(?:c|rb)\.html\z/ !~ file
            apply_template(file, head, header, footer, language)
          end
        end
      end
      File.open("#{reference_base_dir}/.htaccess", "w") do |file|
        file.puts("Redirect permanent /rroonga/text/TUTORIAL_ja_rdoc.html " +
                  "http://groonga.rubyforge.org/rroonga/ja/file.tutorial.html")
        file.puts("Redirect permanent /rroonga/ " +
                  "http://groonga.rubyforge.org/rroonga/en/")
      end
    end
  end

  task :publish => [:generate, "reference:publication:prepare"] do
    rsync_to_rubyforge(spec, "#{reference_base_dir}/", "/#{spec.name}")
  end
end

namespace :html do
  desc "Publish HTML to Web site."
  task :publish do
    rsync_to_rubyforge(spec, "html/", "")
  end
end
task :publish => ["reference:publish", "html:publish"]

desc "Tag the current revision."
task :tag do
  sh("git tag -a #{version} -m 'release #{version}!!!'")
end

namespace :win32 do
  patches_dir = (Pathname.new(base_dir) + "patches").expand_path
  if ENV["GROONGA64"] == "yes"
    host = "x86_64-w64-mingw32"
    mecab_patches = [
      "mecab-0.98-mingw-w64.diff",
      "mecab-0.98-not-use-locale-on-mingw.diff",
    ]
  else
    host = "i586-mingw32msvc"
    mecab_patches = []
  end

  desc "Build MeCab and groonga and install them into vendor/local/."
  task(:build => [:build_mecab, :build_mecab_dict, :build_groonga])

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
      mecab_patches.each do |patch|
        sh("patch -p1 < #{patches_dir + patch}")
      end
      sh("./configure",
         "--prefix=#{binary_dir}",
         "--host=#{host}") or exit(false)
      sh("env", "GREP_OPTIONS=--text", "nice", "make", "-j8") or exit(false)
      sh("env", "GREP_OPTIONS=--text", "make", "install") or exit(false)

      mecab_files_dir = File.join(vendor_dir, "mecab")
      mkdir_p(mecab_files_dir)
      files = ["AUTHORS", "BSD", "COPYING", "GPL", "LGPL"]
      cp(files, mecab_files_dir)
    end
  end

  task(:build_mecab_dict) do
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
  task(:build_groonga) do
    tmp_dir = "tmp/groonga"
    rm_rf(tmp_dir)
    mkdir_p(tmp_dir)
    Dir.chdir(tmp_dir) do
      sh("git", "clone", "git://github.com/groonga/groonga.git") or exit(false)
    end
    Dir.chdir(File.join(tmp_dir, "groonga")) do
      sh("./autogen.sh") or exit(false)
      mecab_config = File.join(binary_dir, "bin", "mecab-config")
      args = ["./configure",
              "--prefix=#{binary_dir}",
              "--host=#{host}",
              "--without-cutter",
              "--disable-benchmark"]
      if File.exist?(mecab_config)
        args << "--with-mecab-config=#{mecab_config}"
      else
        args << "--without-mecab"
      end
      sh(*args) or exit(false)
      sh("env", "GREP_OPTIONS=--text", "nice", "make", "-j8") or exit(false)
      sh("env", "GREP_OPTIONS=--text", "make", "install") or exit(false)

      groonga_files_dir = File.join(vendor_dir, "groonga")
      mkdir_p(groonga_files_dir)
      files = ["AUTHORS", "COPYING"]
      cp(files, groonga_files_dir)
    end
  end
end
