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

module SchemaCreateTableTests
  def test_normal
    assert_nil(context["Posts"])
    Groonga::Schema.create_table("Posts", options) do |table|
    end
    assert_not_nil(context["Posts"])
  end

  def test_force
    Groonga::Schema.create_table("Posts", options) do |table|
      table.string("name")
    end
    assert_not_nil(context["Posts.name"])

    Groonga::Schema.create_table("Posts",
                                 options(:type => differnt_type,
                                         :force => true)) do |table|
    end
    assert_nil(context["Posts.name"])
  end

  def test_different_type
    Groonga::Schema.create_table("Posts", options) do |table|
    end

    assert_raise(Groonga::Schema::TableCreationWithDifferentOptions) do
      Groonga::Schema.create_table("Posts",
                                   options(:type => differnt_type)) do |table|
      end
    end
  end

  def test_same_options
    Groonga::Schema.create_table("Posts", options) do |table|
    end

    Groonga::Schema.create_table("Posts", options) do |table|
    end
    assert_not_nil(context["Posts"])
  end

  def test_same_value_type
    Groonga::Schema.create_table("Posts",
                                 options(:value_type => "UInt32")) do |table|
    end

    Groonga::Schema.create_table("Posts",
                                 options(:value_type => "UInt32")) do |table|
    end
    assert_equal(context["UInt32"], context["Posts"].range)
  end

  def test_differnt_value_type
    Groonga::Schema.create_table("Posts", options) do |table|
    end

    assert_raise(Groonga::Schema::TableCreationWithDifferentOptions) do
      Groonga::Schema.create_table("Posts",
                                   options(:value_type => "Int32")) do |table|
      end
    end
  end

  def test_same_sub_records
    Groonga::Schema.create_table("Posts",
                                 options(:sub_records => true)) do |table|
    end

    Groonga::Schema.create_table("Posts",
                                 options(:sub_records => true)) do |table|
    end
    assert_true(context["Posts"].support_sub_records?)
  end

  def test_different_sub_records
    Groonga::Schema.create_table("Posts",
                                 options(:sub_records => true)) do |table|
    end

    assert_raise(Groonga::Schema::TableCreationWithDifferentOptions) do
      Groonga::Schema.create_table("Posts",
                                   options(:sub_records => false)) do |table|
      end
    end
  end

  def test_default_path
    Groonga::Schema.create_table("Posts", options) do |table|
    end
    assert_equal("#{@database_path}.0000100",
                 context["Posts"].path)
  end

  def test_default_named_path
    options_with_named_path = options.merge(:named_path => true)
    Groonga::Schema.create_table("Posts", options_with_named_path) do |table|
    end
    assert_equal("#{@database_path}.tables/Posts",
                 context["Posts"].path)
  end

  private
  def options(addional_options={})
    default_options.merge(addional_options)
  end
end

module SchemaCreateTableWithKeyTests
  def test_same_key_type
    Groonga::Schema.create_table("Posts",
                                 options(:key_type => "ShortText")) do |table|
    end

    Groonga::Schema.create_table("Posts",
                                 options(:key_type => "ShortText")) do |table|
    end
    assert_equal(context["ShortText"], context["Posts"].domain)
  end

  def test_differnt_key_type
    Groonga::Schema.create_table("Posts",
                                 options(:key_type => "Int32")) do |table|
    end

    assert_raise(Groonga::Schema::TableCreationWithDifferentOptions) do
      Groonga::Schema.create_table("Posts",
                                   options(:key_type => "ShortText")) do |table|
      end
    end
  end

  def test_same_default_tokenizer
    _options = options(:key_type => "ShortText",
                       :default_tokenizer => "TokenBigram")
    Groonga::Schema.create_table("Posts", _options) do |table|
    end

    Groonga::Schema.create_table("Posts", _options) do |table|
    end
    assert_equal(context["TokenBigram"], context["Posts"].default_tokenizer)
  end

  def test_differnt_default_tokenizer
    _options = options(:key_type => "ShortText")
    Groonga::Schema.create_table("Posts", _options) do |table|
    end

    _options = _options.merge(:default_tokenizer => "TokenBigram")
    assert_raise(Groonga::Schema::TableCreationWithDifferentOptions) do
      Groonga::Schema.create_table("Posts", _options) do |table|
      end
    end
  end

  def test_same_key_normalize
    _options = options(:key_type => "ShortText",
                       :key_normalize => true)
    Groonga::Schema.create_table("Posts", _options) do |table|
    end

    Groonga::Schema.create_table("Posts", _options) do |table|
    end
    assert_true(context["Posts"].normalize_key?)
  end

  def test_differnt_key_normalize
    _options = options(:key_type => "ShortText",
                       :key_normalize => true)
    Groonga::Schema.create_table("Posts", _options) do |table|
    end

    _options = _options.merge(:key_normalize => false)
    assert_raise(Groonga::Schema::TableCreationWithDifferentOptions) do
      Groonga::Schema.create_table("Posts", _options) do |table|
      end
    end
  end
end

class SchemaCreateTableArrayTest < Test::Unit::TestCase
  include GroongaTestUtils
  include SchemaCreateTableTests

  setup :setup_database

  private
  def default_options
    {}
  end

  def differnt_type
    :hash
  end
end

class SchemaCreateTableHashTest < Test::Unit::TestCase
  include GroongaTestUtils
  include SchemaCreateTableTests
  include SchemaCreateTableWithKeyTests

  setup :setup_database

  private
  def default_options
    {:type => :hash}
  end

  def differnt_type
    :patricia_trie
  end
end

class SchemaCreateTablePatriciaTrieTest < Test::Unit::TestCase
  include GroongaTestUtils
  include SchemaCreateTableTests
  include SchemaCreateTableWithKeyTests

  setup :setup_database

  def test_same_key_with_sis
    _options = options(:key_type => "ShortText",
                       :key_with_sis => true)
    Groonga::Schema.create_table("Posts", _options) do |table|
    end

    Groonga::Schema.create_table("Posts", _options) do |table|
    end
    assert_true(context["Posts"].register_key_with_sis?)
  end

  def test_differnt_key_with_sis
    _options = options(:key_type => "ShortText",
                       :key_with_sis => true)
    Groonga::Schema.create_table("Posts", _options) do |table|
    end

    _options = _options.merge(:key_with_sis => false)
    assert_raise(Groonga::Schema::TableCreationWithDifferentOptions) do
      Groonga::Schema.create_table("Posts", _options) do |table|
      end
    end
  end

  private
  def default_options
    {:type => :patricia_trie}
  end

  def differnt_type
    :array
  end
end
