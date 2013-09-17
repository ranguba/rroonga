# Copyright (C) 2013  Kouhei Sutou <kou@clear-code.com>
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

class TableKeySupportTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  class TypeTest < self
    def test_int8
      key = -1
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "Int8")
      id = ids.add(key)
      assert_equal(key, id.key)
    end

    def test_uint8
      key = 1
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "UInt8")
      id = ids.add(key)
      assert_equal(key, id.key)
    end

    def test_int16
      key = -(2 ** 8)
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "Int16")
      id = ids.add(key)
      assert_equal(key, id.key)
    end

    def test_uint16
      key = 2 ** 8
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "UInt16")
      id = ids.add(key)
      assert_equal(key, id.key)
    end

    def test_int32
      key = -(2 ** 16)
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "Int32")
      id = ids.add(key)
      assert_equal(key, id.key)
    end

    def test_uint32
      key = 2 ** 16
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "UInt32")
      id = ids.add(key)
      assert_equal(key, id.key)
    end

    def test_int64
      key = -(2 ** 32)
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "Int64")
      id = ids.add(key)
      assert_equal(key, id.key)
    end

    def test_uint64
      key = 2 ** 32
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "UInt64")
      id = ids.add(key)
      assert_equal(key, id.key)
    end
  end

  class TokenizeTest < self
    class TableTypeTest < self
      def test_hash
        Groonga::Schema.create_table("Terms",
                                     :type => :hash,
                                     :key_type => "ShortText",
                                     :default_tokenizer => "TokenBigram",
                                     :normalizer => "NormalizerAuto")
        terms = Groonga["Terms"]
        tokens = terms.tokenize("Hello World!")
        assert_equal(["!", "hello", "world"],
                     tokens.collect(&:key).sort)
      end

      def test_patricia_trie
        Groonga::Schema.create_table("Terms",
                                     :type => :patricia_trie,
                                     :key_type => "ShortText",
                                     :default_tokenizer => "TokenBigram",
                                     :normalizer => "NormalizerAuto")
        terms = Groonga["Terms"]
        tokens = terms.tokenize("Hello World!")
        assert_equal(["!", "hello", "world"],
                     tokens.collect(&:key).sort)
      end
    end
  end
end
