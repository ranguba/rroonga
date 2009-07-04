# -*- coding: utf-8 -*-
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

require 'groonga/record'
require 'groonga.so'

##
# Ruby/groonga用のネームスペース。Ruby/groongaのクラスやメソッ
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

    ##
    # VERSIONを"."で結合して<tt>"メジャーバージョン.マイナー
    # バージョン.マイクロバージョン"</tt>という形式の文字列に
    # したもの。
    def version
      VERSION.join(".")
    end

    ##
    # BINDINGS_VERSIONを"."で結合して<tt>"メジャーバージョン.マ
    # イナーバージョン.マイクロバージョン"</tt>という形式の文
    # 字列にしたもの。
    def bindings_version
      BINDINGS_VERSION.join(".")
    end
  end
end

require 'groonga/schema'
