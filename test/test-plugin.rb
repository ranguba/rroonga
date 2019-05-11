# Copyright (C) 2011-2015  Kouhei Sutou <kou@clear-code.com>
# Copyright (C) 2016  Masafumi Yokoyama <yokoyama@clear-code.com>
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

class PluginTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_sandbox
  setup :setup_database
  teardown :teardown_sandbox

  def test_register
    assert_nil(context["TokenFilterStopWord"])
    context.register_plugin("token_filters/stop_word")
    assert_not_nil(context["TokenFilterStopWord"])
  end

  def test_system_plugins_dir
    plugin_path = "#{Groonga::Plugin.system_plugins_dir}/"
    plugin_path << "token_filters/stop_word#{Groonga::Plugin.suffix}"
    assert do
      File.exist?(plugin_path)
    end
  end

  def test_ruby_suffix
    assert_equal(".rb", Groonga::Plugin.ruby_suffix)
  end

  class UnregisterTest < self
    def test_by_name
      context.register_plugin("token_filters/stop_word")
      assert_not_nil(context["TokenFilterStopWord"])
      context.unregister_plugin("token_filters/stop_word")
      assert_nil(context["TokenFilterStopWord"])
    end

    def test_by_path
      only_not_windows # TODO: We can remove this with Groonga 9.0.3
      context.register_plugin("token_filters/stop_word")
      assert_not_nil(context["TokenFilterStopWord"])
      plugin_path = "#{Groonga::Plugin.system_plugins_dir}/"
      plugin_path << "token_filters/stop_word#{Groonga::Plugin.suffix}"
      context.unregister_plugin(plugin_path)
      assert_nil(context["TokenFilterStopWord"])
    end
  end

  class NamesTest < self
    def test_nothing
      assert_equal([], Groonga::Plugin.names)
    end

    def test_one_plugin
      context.register_plugin("token_filters/stop_word")
      assert_equal(["token_filters/stop_word"],
                   Groonga::Plugin.names)
    end

    def test_multiple_plugins
      context.register_plugin("token_filters/stop_word")
      context.register_plugin("query_expanders/tsv")
      assert_equal(["token_filters/stop_word", "query_expanders/tsv"],
                   Groonga::Plugin.names)
    end

    def test_context_option
      context.register_plugin("token_filters/stop_word")
      assert_equal([], Groonga::Plugin.names(context: Groonga::Context.new))
    end
  end
end
