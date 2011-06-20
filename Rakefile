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

if YAML.const_defined?(:ENGINE)
  begin
    YAML::ENGINE.yamler = "psych"
  rescue LoadError
  end
end

base_dir = File.join(File.dirname(__FILE__))

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
  spec.files = FileList["{lib,benchmark,misc}/**/*.rb",
                        "bin/*",
                        "*.rb",
                        "example/*.rb",
                        "Rakefile",
                        "ext/**/*"]
  spec.test_files = FileList["test/**/*.rb"]
end

Jeweler::RubygemsDotOrgTasks.new do
end

module YARD
  module CodeObjects
    class Proxy
      alias_method :initialize_original, :initialize
      def initialize(namespace, name)
        name = name.gsub(/\AGrn(.*)\z/) do
          suffix = $1
          case suffix
          when ""
            "Groonga"
          when "TableKeySupport"
            "Groonga::Table::KeySupport"
          else
            "Groonga::#{suffix}"
          end
        end
        initialize_original(namespace, name)
      end
    end
  end
end

YARD::Rake::YardocTask.new do |task|
  task.options += ["--title", "#{spec.name} - #{version}"]
  # task.options += ["--charset", "UTF-8"]
  task.options += ["--readme", "README.textile"]
  task.options += ["--files", "doc/text/**/*"]
  task.options += ["--output-dir", "doc/html/en/"]
  task.files += FileList["ext/groonga/**/*.c"]
  task.files += FileList["lib/**/*.rb"]
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

groonga_win32_i386_p = ENV["GROONGA32"] == "yes"

Rake::ExtensionTask.new("groonga", spec) do |ext|
  if groonga_win32_i386_p
    ext.cross_platform = ["x86-mingw32", "i386-mswin32"]
  else
    ext.cross_platform = ["x64-mingw32"]
    # ext.cross_platform << "x64-mswin64" # We need to build with VC++ 2010. :<
  end
  if windows?
    ext.gem_spec.files += collect_binary_files(relative_binary_dir)
  else
    ext.cross_compile = true
    ext.cross_compiling do |_spec|
      if windows?(_spec.platform.to_s)
        _spec.files += collect_binary_files(relative_binary_dir)
      end
    end
  end
end

include ERB::Util

def apply_template(file, top_path, head, header, footer, language)
  content = File.read(file)
  content = content.sub(/lang="en"/, 'lang="#{language}"')

  title = nil
  content = content.sub(/<title>(.+?)<\/title>/m) do
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

def rsync_to_rubyforge(spec, source, destination, options={})
  config = YAML.load(File.read(File.expand_path("~/.rubyforge/user-config.yml")))
  host = "#{config["username"]}@rubyforge.org"

  rsync_args = "-av --exclude '*.erb' --chmod=ug+w"
  rsync_args << " --delete" if options[:delete]
  remote_dir = "/var/www/gforge-projects/#{spec.rubyforge_project}/"
  sh("rsync #{rsync_args} #{source} #{host}:#{remote_dir}#{destination}")
end

namespace :reference do
  translate_languages = [:ja]
  supported_languages = [:en, *translate_languages]
  reference_base_dir = "doc/html"
  html_files = FileList["doc/html/en/**/*.html"].to_a

  directory reference_base_dir
  CLOBBER.include(reference_base_dir)

  po_dir = "doc/po"
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

      doc_dir = Pathname("#{reference_base_dir}/en")
      desc "Translates documents to #{language}."
      task language => [po_file, reference_base_dir, *html_files] do
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
      supported_languages.each do |language|
        doc_dir = Pathname.new("#{reference_base_dir}/#{language}")
        head = erb_template("head.#{language}")
        header = erb_template("header.#{language}")
        footer = erb_template("footer.#{language}")
        doc_dir.find do |file|
          case file.basename.to_s
          when "_index.html", /\A(?:class|method|file)_list.html\z/
            next
          when /\.html\z/
            relative_top_path = file.relative_path_from(doc_dir).dirname
            apply_template(file, "../../#{relative_top_path}",
                           head, header, footer, language)
          end
        end
      end
      File.open("#{reference_base_dir}/.htaccess", "w") do |file|
        file.puts("Redirect permanent /rroonga/text/TUTORIAL_ja_rdoc.html " +
                  "http://groonga.rubyforge.org/rroonga/ja/file.tutorial.html")
        file.puts("RedirectMatch permanent ^/rroonga/$ " +
                  "http://groonga.rubyforge.org/rroonga/en/")
      end
    end
  end

  desc "Upload document to rubyforge."
  task :publish => [:generate, "reference:publication:prepare"] do
    rsync_to_rubyforge(spec, "#{reference_base_dir}/", spec.name,
                       :delete => true)
  end
end

namespace :html do
  desc "Publish HTML to Web site."
  task :publish do
    rsync_to_rubyforge(spec, "html/", "")
  end
end

desc "Upload document and HTML to rubyforge."
task :publish => ["reference:publish", "html:publish"]

desc "Tag the current revision."
task :tag do
  sh("git tag -a #{version} -m 'release #{version}!!!'")
end

namespace :win32 do
  patches_dir = (Pathname.new(base_dir) + "patches").expand_path
  if groonga_win32_i386_p
    host = "i586-mingw32msvc"
    mecab_patches = []
  else
    host = "x86_64-w64-mingw32"
    mecab_patches = [
      "mecab-0.98-mingw-w64.diff",
      "mecab-0.98-not-use-locale-on-mingw.diff",
    ]
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

      mecab_rc_path = File.join(binary_dir, "etc", "mecabrc")
      win32_mecab_rc_path = File.join(binary_dir, "bin", "mecabrc")
      mv(mecab_rc_path, win32_mecab_rc_path)

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
    dictionary_dir = '$(rcpath)\..\share\mecab\dic\naist-jdic'
    mecab_rc_path = File.join(binary_dir, "bin", "mecabrc")
    mecab_rc_content = File.read(mecab_rc_path)
    File.open(mecab_rc_path, "w") do |mecab_rc|
      mecab_rc.print(mecab_rc_content.gsub(/^dicdir\s*=.+$/,
                                           "dicdir = #{dictionary_dir}"))
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
