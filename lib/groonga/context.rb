# -*- coding: utf-8 -*-
#
# Copyright (C) 2010-2016  Kouhei Sutou <kou@clear-code.com>
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

require "groonga/memory-pool"
require "groonga/context/command-executor"

module Groonga
  class Context
    class << self
      # Opens a new context. If block is given, the opened context is
      # closed automatically after the given block is evaluated.
      #
      # @overload open(*args, &block)
      #   @param args [::Array<Object>] Passed through to
      #     {Groonga::Context#initialize}.
      #   @yieldparam context [Groonga::Context] The newly created context.
      #   @return [Object] The return value of the given block is the
      #     return value of the call.
      #
      # @overload open(*args)
      #   @param args [::Array<Object>] Passed through to
      #     {Groonga::Context#initialize}.
      #   @yieldparam context [Groonga::Context] The newly created context.
      #   @return [Groonga::Context] The newly created context.
      #
      # @see Groonga::Context#initialize
      def open(*args, **kwargs)
        context = new(*args, **kwargs)
        if block_given?
          begin
            yield(context)
          ensure
            context.close
          end
        else
          context
        end
      end
    end

    # _path_ にある既存のデータベースを開く。ブロックを指定した場
    # 合はブロックに開いたデータベースを渡し、ブロックを抜けると
    # きに閉じる。
    def open_database(path, &block)
      options = {:context => self}

      Database.open(path, options, &block)
    end

    # This is convenience method. It wraps {Groonga::Database.create}
    # for the context.
    #
    # @overload create_database
    #   Creates a new temproary database for the context.
    #
    #   @return [Groonga::Database] A new temporary database.
    #
    #   @example Creating a new temporary database
    #     temporary_database = context.create_database
    #
    # @overload create_database(&block)
    #   Creates a new temproary database for the context. The database
    #   is closed after the passed block is finished.
    #
    #   @yield [database] Yields a newly created temporary database
    #     for the context. The database is available only in the
    #     block.
    #
    #   @yieldparam [Groonga::Database] database A newly created
    #     temporary database for the context.
    #
    #   @yieldreturn [Object] The returned value from the block is the
    #     returned value from this method.
    #
    #   @return Returned value from the block.
    #
    #   @example Creating a new temporary database with block
    #     context.create_database do |temporary_database|
    #       # ...
    #     end
    #
    # @overload create_database(path)
    #   Creates a new persistent database for the context to the _path_.
    #
    #   @param [String] path Database path for a new persistent
    #     database.
    #
    #   @return [Groonga::Database] A new persistent database for the
    #     context.
    #
    #   @example Creating a new persistent database to _"/tmp/db.groonga"_
    #     database = context.create_database("/tmp/db.groonga")
    #
    # @overload create_database(path, &block)
    #   Creates a new persistent database for the context to the
    #   _path_. The database is closed after the passed block is
    #   finished.
    #
    #   @param [String] path Database path for a new persistent
    #     database.
    #
    #   @yield [database] Yields a newly created persistent database
    #     for the context. The database is available only in the
    #     block.
    #
    #   @yieldparam [Groonga::Database] database A newly created
    #     persistent database for the context.
    #
    #   @yieldreturn [Object] The returned value from the block is the
    #     returned value from this method.
    #
    #   @return Returned value from the block.
    #
    #   @example Creating a new persistent database to _"/tmp/db.groonga"_ database with block
    #     context.create_database("/tmp/db.groonga") do |persistent_database|
    #       # ...
    #     end
    def create_database(path=nil, &block)
      options = {:context => self}
      if path
        options[:path] = path
      end

      Database.create(options, &block)
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

    # Unregister a registered `name` plugin.
    #
    # You can unregister `name` plugin by name if
    # `name` plugin is installed to plugin directory.
    #
    # @example Unregister a registerd plugin by name.
    #   context.register_plugin("token_filters/stop_word")
    #   context.unregister_plugin("token_filters/stop_word")
    #
    # You can also specify the path of `name` plugin explicitly.
    #
    # @example Unregister a registerd plugin by path.
    #   context.register_plugin("token_filters/stop_word")
    #   context.unregister_plugin("/usr/local/lib/groonga/plugins/token_filters/stop_word.so")
    #
    # @overload unregister_plugin(name)
    #   Unregister a registerd plugin by name.
    #
    #   @param [String] name The plugin name.
    #
    # @overload unregister_plugin(path)
    #   Unregister a registerd plugin by path.
    #
    #   @param [String] path The path to plugin.
    #
    # @since 5.0.1
    def unregister_plugin(name_or_path)
      options = {:context => self}
      Plugin.unregister(name_or_path, options)
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
      execute_command("select", {:table => table}.merge(options))
    end

    def execute_command(name, parameters={})
      executor = CommandExecutor.new(self)
      executor.execute(name, parameters)
    end

    # Restore commands dumped by "grndump" command.
    #
    # @example Restore dumped commands as a String object.
    #   dumped_commands = File.read("dump.grn")
    #   context.restore(dumped_commands)
    #
    # @example Restore dumped commands from a File object.
    #   File.open("dump.grn") do |file|
    #     context.restore(file)
    #   end
    #
    # If block is given, a response is yielded.
    #
    # @example Restore dumped commands and reports result.
    #   dumped_commands = File.read("dump.grn")
    #   context.restore(dumped_commands) do |command, response|
    #     puts("#{command} -> #{response}")
    #   end
    #
    # @param [#each_line] dumped_commands commands dumped by grndump.
    #   It can be a String object or any objects like an IO object such
    #   as a File object. It should have #each_line that iterates a
    #   line.
    # @yield [command, response]
    #   Yields a sent command and its response if block is given.
    # @yieldparam command [String] A sent command.
    # @yieldparam response [String] A response for a command.
    # @return [void]
    def restore(dumped_commands)
      buffer = ""
      continued = false
      dumped_commands.each_line do |line|
        line = line.chomp
        case line
        when /\\\z/
          continued = true
          buffer << $PREMATCH
        else
          continued = false
          buffer << line
          send(buffer)
          _, response = receive
          if block_given?
            not_shared_command = continued ? buffer.dup : line
            yield(not_shared_command, response)
          end
          buffer.clear
        end
      end
      unless buffer.empty?
        send(buffer)
        _, response = receive
        yield(buffer.dup, response) if block_given?
      end
    end

    # Pushes a new memory pool to the context. Temporary objects that
    # are created between pushing a new memory pool and popping the
    # new memory pool are closed automatically when popping the new
    # memory pool.
    #
    # It is useful for request and response style applications. These
    # style applications can close temporary objects between a request
    # and resopnse pair. There are some merits for closing temporary
    # objects explicilty rather than closing implicitly by GC:
    #
    #   * Less memory consumption
    #   * Faster
    #
    # The "less memory consumption" merit is caused by temporary
    # objects are closed each request and response pair. The max
    # memory consumption in these applications is the same as the max
    # memory consumption in a request and response pair. If temporary
    # objects are closed by GC, the max memory consumption in these
    # applications is the same as the max memory consumption between
    # the current GC and the next GC. These applications process many
    # request and response pairs during two GCs.
    #
    # The "faster" merit is caused by reducing GC. You can reduce GC,
    # your application run faster because GC is a heavy process. You
    # can reduce GC because memory consumption is reduced.
    #
    # You can nest {#push_memory_pool} and {#pop_memory_pool} pair.
    #
    # @example Pushes a new memory pool with block
    #   adults = nil
    #   context.push_memory_pool do
    #     users = context["Users"]
    #     adults = users.select do |user|
    #       user.age >= 20
    #     end
    #     p adults.temporary? # => true
    #     p adults.closed?    # => false
    #   end
    #   p adults.closed?      # => true
    #
    # @example Pushes a new memory pool without block
    #   adults = nil
    #   context.push_memory_pool
    #   users = context["Users"]
    #   adults = users.select do |user|
    #     user.age >= 20
    #   end
    #   p adults.temporary? # => true
    #   p adults.closed?    # => false
    #   context.pop_memory_pool
    #   p adults.closed?    # => true
    #
    # @example Nesting push and pop pair
    #   adults = nil
    #   context.push_memory_pool do
    #     users = context["Users"]
    #     adults = users.select do |user|
    #       user.age >= 20
    #     end
    #     grouped_adults = nil
    #     context.push_memory_pool do
    #       grouped_adults = adults.group(["hobby"])
    #       p grouped_adults.temporary? # => true
    #       p grouped_adults.closed?    # => false
    #     end
    #     p grouped_adults.closed?      # => true
    #     p adults.temporary?           # => true
    #     p adults.closed?              # => false
    #   end
    #   p adults.closed?                # => true
    #
    # @overload push_memory_pool
    #   Pushes a new memory pool to the context. You need to pop the
    #   memory pool explicitly by yourself.
    #
    #   @return [void]
    #
    # @overload push_memory_pool {}
    #   Closes temporary objects created in the given block
    #   automatically.
    #
    #   @yield []
    #     Yields the block. Temporary objects created in the block
    #     are closed automatically when the block is exited.
    #   @yieldreturn [Object] It is the return value of this
    #     method call.
    #   @return [Object] The value returned by the block.
    #
    # @since 3.0.5
    def push_memory_pool
      memory_pool = MemoryPool.new
      @memory_pools.push(memory_pool)
      return unless block_given?

      begin
        yield
      ensure
        pop_memory_pool
      end
    end

    # Pops the pushed memory pool.
    #
    # @return [void]
    #
    # @see push_memory_pool
    #
    # @since 3.0.5
    def pop_memory_pool
      memory_pool = @memory_pools.pop
      memory_pool.close
    end

    # @api private
    def object_created(object)
      return if @memory_pools.empty?
      memory_pool = @memory_pools.last
      memory_pool.register(object)
    end

    # @return [Groonga::Config] The database level configuration sets of
    #   this context.
    #
    # @since 5.0.9
    # @deprecated since 5.1.1. Use {Groonga::Context#config} instead.
    def conf
      config
    end

    # @return [Groonga::Config] The database level configuration sets of
    #   this context.
    #
    # @since 5.1.1
    def config
      @config ||= Config.new(self)
    end
  end
end
