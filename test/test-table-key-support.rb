# Copyright (C) 2013-2014  Kouhei Sutou <kou@clear-code.com>
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

  class CastTest < self
    def test_int8
      key = "-1"
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "Int8")
      id = ids.add(key)
      assert_equal(key.to_i, id.key)
    end

    def test_uint8
      key = "1"
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "UInt8")
      id = ids.add(key)
      assert_equal(key.to_i, id.key)
    end

    def test_int16
      key = (-(2 ** 8)).to_s
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "Int16")
      id = ids.add(key)
      assert_equal(key.to_i, id.key)
    end

    def test_uint16
      key = (2 ** 8).to_s
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "UInt16")
      id = ids.add(key)
      assert_equal(key.to_i, id.key)
    end

    def test_int32
      key = (-(2 ** 16)).to_s
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "Int32")
      id = ids.add(key)
      assert_equal(key.to_i, id.key)
    end

    def test_uint32
      key = (2 ** 16).to_s
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "UInt32")
      id = ids.add(key)
      assert_equal(key.to_i, id.key)
    end

    def test_int64
      key = (-(2 ** 32)).to_s
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "Int64")
      id = ids.add(key)
      assert_equal(key.to_i, id.key)
    end

    def test_uint64
      key = (2 ** 32).to_s
      ids = Groonga::Hash.create(:name => "IDs", :key_type => "UInt64")
      id = ids.add(key)
      assert_equal(key.to_i, id.key)
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

    class AddOptionTest < self
      setup
      def setup_lexicon
        Groonga::Schema.create_table("Terms",
                                     :type => :patricia_trie,
                                     :key_type => "ShortText",
                                     :default_tokenizer => "TokenBigram",
                                     :normalizer => "NormalizerAuto")
        @lexicon = Groonga["Terms"]
      end

      def test_default
        tokens = @lexicon.tokenize("Hello World!")
        assert_equal(["!", "hello", "world"],
                     tokens.collect(&:key).sort)
      end

      def test_true
        tokens = @lexicon.tokenize("Hello World!", :add => true)
        assert_equal(["!", "hello", "world"],
                     tokens.collect(&:key).sort)
      end

      def test_false
        tokens = @lexicon.tokenize("Hello groonga!", :add => true)
        assert_equal(["!", "groonga", "hello"],
                     tokens.collect(&:key).sort)

        tokens = @lexicon.tokenize("Hello World!", :add => false)
        assert_equal(["!", "hello"],
                     tokens.collect(&:key).sort)
      end
    end
  end

  class ReindexTest < self
    def test_patricia_trie
      Groonga::Schema.define do |schema|
        schema.create_table("Memos",
                            :type => :array) do |table|
          table.text("content")
        end
        schema.create_table("Terms",
                            :type => :patricia_trie,
                            :key_type => :short_text,
                            :default_tokenizer => "TokenBigram",
                            :normalizer => "NormalizerAuto") do |table|
          table.index("Memos.content")
        end
      end

      memos = context["Memos"]
      memos.add(:content => "This is a memo")

      terms = context["Terms"]
      terms.delete("this")

      assert_equal([
                     "a",
                     "is",
                     "memo",
                   ],
                   terms.collect(&:_key).sort)

      terms.reindex

      assert_equal([
                     "a",
                     "is",
                     "memo",
                     "this",
                   ],
                   terms.collect(&:_key).sort)
    end
  end
end
