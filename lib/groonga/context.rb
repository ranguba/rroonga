# -*- coding: utf-8 -*-
#
# Copyright (C) 2010-2012  Kouhei Sutou <kou@clear-code.com>
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

require "groonga/command"

module Groonga
  class Context
    # _path_ にある既存のデータベースを開く。ブロックを指定した場
    # 合はブロックに開いたデータベースを渡し、ブロックを抜けると
    # きに閉じる。
    def open_database(path, &block)
      options = {:context => self}

      Database.open(path, options, &block)
    end

    # _path_ に新しくデータベースを作成する。 _path_ を省略すると
    # 一時データベースとなる。
    #
    # @example
    #   #一時データベースを作成:
    #   context.create_database
    #
    #   #永続データベースを作成:
    #   context.create_database("/tmp/db.groonga")
    def create_database(path=nil)
      options = {:context => self}
      if path
        options[:path] = path
      end

      Database.create(options)
    end

    # groongaのプラグインディレクトリにあるプラグイン _name_
    # を登録する。 _path_ を指定するとプラグインディレクトリ以
    # 外にあるプラグインを登録することができる。
    def register_plugin(name_or_options)
      options = {:context => self}
      if name_or_options.is_a?(String)
        name = name_or_options
        Plugin.register(name, options)
      else
        Plugin.register(name_or_options.merge(options))
      end
    end

    # _table_ から指定した条件にマッチするレコードの値を取得
    # する。 _table_ はテーブル名かテーブルオブジェクトを指定
    # する。
    #
    # _options_ に指定できるキーは以下の通り。
    # @param [::Hash] options The name and value
    #   pairs. Omitted names are initialized as the default value.
    # @option options [Array] output_columns The output_columns
    #
    #   値を取得するカラムを指定する。
    # @option options [Array] XXX TODO
    #   TODO
    def select(table, options={})
      select = Command::Select.new(self, table, options)
      select.exec
    end
  end
end
