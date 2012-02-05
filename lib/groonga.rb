# -*- coding: utf-8 -*-
#
# Copyright (C) 2009-2012  Kouhei Sutou <kou@clear-code.com>
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

base_dir = Pathname.new(__FILE__).dirname.dirname.expand_path
local_groonga_dir = base_dir + "vendor" + "local"
local_groonga_bin_dir = local_groonga_dir + "bin"
if local_groonga_bin_dir.exist?
  prepend_path = Proc.new do |environment_name, separator|
    paths = (ENV[environment_name] || '').split(/#{separator}/)
    dir = local_groonga_bin_dir.to_s
    dir = dir.gsub(/\//, File::ALT_SEPARATOR) if File::ALT_SEPARATOR
    unless paths.include?(dir)
      paths = [dir] + paths
      ENV[environment_name] = paths.join(separator)
    end
  end

  prepend_path.call("PATH", File::PATH_SEPARATOR)
end

require 'groonga/view-record'
require 'groonga/record'
require 'groonga/expression-builder'
require 'groonga/posting'
begin
  major, minor, _ = RUBY_VERSION.split(/\./)
  require "#{major}.#{minor}/groonga.so"
rescue LoadError
  require 'groonga.so'
end

##
# rroonga用のネームスペース。rroongaのクラスやメソッ
# ドなどはこのモジュール以下に定義される。
module Groonga
  class << self
    ##
    # BUILD_VERSIONを"."で結合して<tt>"メジャーバージョン.マ
    # イナーバージョン.マイクロバージョン"</tt>という形式の
    # 文字列にしたもの。
    def build_version
      BUILD_VERSION.join(".")
    end

    # Format version.
    #
    # @return [String] If Groonga::VERSION has tag,
    #   @MAJOR.MINOR.MICRO-TAG@. Otherwise, @MAJOR.MINOR.MICRO@.
    def version
      major, minor, micro, tag = VERSION
      version_string = [major, minor, micro].join(".")
      version_string << "-#{tag}" if tag
      version_string
    end

    ##
    # BINDINGS_VERSIONを"."で結合して<tt>"メジャーバージョン.マ
    # イナーバージョン.マイクロバージョン"</tt>という形式の文
    # 字列にしたもの。
    def bindings_version
      BINDINGS_VERSION.join(".")
    end

    ##
    # call-seq:
    #   Groonga[name] -> Groonga::Object or nil
    #   Groonga[id]   -> Groonga::Object or nil
    #
    # 便利メソッド。Groonga::Context.default[name]と同じ。
    def [](name)
      Context.default[name]
    end
  end
end

require 'groonga/context'
require 'groonga/patricia-trie'
require 'groonga/dumper'
require 'groonga/schema'
require 'groonga/pagination'
require 'groonga/query-log'
require 'groonga/grntest-log'
