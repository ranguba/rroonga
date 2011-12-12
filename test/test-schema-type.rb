# Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>
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

class SchemaTypeTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_normalize_short_text
    assert_normalize_type("ShortText", "short_text")
    assert_normalize_type("ShortText", "string")
    assert_normalize_type("ShortText", "ShortText")
  end

  def test_normalize_text
    assert_normalize_type("Text", "text")
    assert_normalize_type("Text", "Text")
  end

  def test_normalize_long_text
    assert_normalize_type("LongText", "binary")
    assert_normalize_type("LongText", "long_text")
    assert_normalize_type("LongText", "LongText")
  end

  def test_normalize_integer8
    assert_normalize_type("Int8", "int8")
    assert_normalize_type("Int8", "integer8")
    assert_normalize_type("Int8", "Int8")
  end

  def test_normalize_integer16
    assert_normalize_type("Int16", "int16")
    assert_normalize_type("Int16", "integer16")
    assert_normalize_type("Int16", "Int16")
  end

  def test_normalize_integer32
    assert_normalize_type("Int32", "int")
    assert_normalize_type("Int32", "integer")
    assert_normalize_type("Int32", "int32")
    assert_normalize_type("Int32", "integer32")
    assert_normalize_type("Int32", "Int32")
  end

  def test_normalize_integer64
    assert_normalize_type("Int64", "int64")
    assert_normalize_type("Int64", "integer64")
    assert_normalize_type("Int64", "Int64")
  end

  def test_normalize_unsigned_integer8
    assert_normalize_type("UInt8", "uint8")
    assert_normalize_type("UInt8", "unsigned_integer8")
    assert_normalize_type("UInt8", "UInt8")
  end

  def test_normalize_unsigned_integer16
    assert_normalize_type("UInt16", "uint16")
    assert_normalize_type("UInt16", "unsigned_integer16")
    assert_normalize_type("UInt16", "UInt16")
  end

  def test_normalize_unsigned_integer32
    assert_normalize_type("UInt32", "uint")
    assert_normalize_type("UInt32", "unsigned_integer")
    assert_normalize_type("UInt32", "uint32")
    assert_normalize_type("UInt32", "unsigned_integer32")
    assert_normalize_type("UInt32", "UInt32")
  end

  def test_normalize_unsigned_integer64
    assert_normalize_type("UInt64", "uint64")
    assert_normalize_type("UInt64", "unsigned_integer64")
    assert_normalize_type("UInt64", "UInt64")
  end

  def test_normalize_float
    assert_normalize_type("Float", "float")
    assert_normalize_type("Float", "Float")
  end

  def test_normalize_time
    assert_normalize_type("Time", "datetime")
    assert_normalize_type("Time", "timestamp")
    assert_normalize_type("Time", "time")
    assert_normalize_type("Time", "date")
    assert_normalize_type("Time", "Time")
  end

  def test_normalize_boolean
    assert_normalize_type("Bool", "boolean")
    assert_normalize_type("Bool", "Bool")
  end

  def test_normalize_tokyo_geo_point
    assert_normalize_type("TokyoGeoPoint", "tokyo_geo_point")
    assert_normalize_type("TokyoGeoPoint", "TokyoGeoPoint")
  end

  def test_normalize_wgs84_geo_point
    assert_normalize_type("WGS84GeoPoint", "geo_point")
    assert_normalize_type("WGS84GeoPoint", "wgs84_geo_point")
    assert_normalize_type("WGS84GeoPoint", "WGS84GeoPoint")
  end

  def test_normalize_delimit
    assert_normalize_type("TokenDelimit", "delimit")
    assert_normalize_type("TokenDelimit", "token_delimit")
    assert_normalize_type("TokenDelimit", "TokenDelimit")
  end

  def test_normalize_unigram
    assert_normalize_type("TokenUnigram", "unigram")
    assert_normalize_type("TokenUnigram", "token_unigram")
    assert_normalize_type("TokenUnigram", "TokenUnigram")
  end

  def test_normalize_bigram
    assert_normalize_type("TokenBigram", "bigram")
    assert_normalize_type("TokenBigram", "token_bigram")
    assert_normalize_type("TokenBigram", "TokenBigram")
  end

  def test_normalize_bigram_split_symbol
    assert_normalize_type("TokenBigramSplitSymbol", "bigram_split_symbol")
    assert_normalize_type("TokenBigramSplitSymbol", "token_bigram_split_symbol")
    assert_normalize_type("TokenBigramSplitSymbol", "TokenBigramSplitSymbol")
  end

  def test_normalize_bigram_split_symbol_alpha
    assert_normalize_type("TokenBigramSplitSymbolAlpha",
                          "bigram_split_symbol_alpha")
    assert_normalize_type("TokenBigramSplitSymbolAlpha",
                          "token_bigram_split_symbol_alpha")
    assert_normalize_type("TokenBigramSplitSymbolAlpha",
                          "TokenBigramSplitSymbolAlpha")
  end

  def test_normalize_bigram_split_symbol_alpha_digit
    assert_normalize_type("TokenBigramSplitSymbolAlphaDigit",
                          "bigram_split_symbol_alpha_digit")
    assert_normalize_type("TokenBigramSplitSymbolAlphaDigit",
                          "token_bigram_split_symbol_alpha_digit")
    assert_normalize_type("TokenBigramSplitSymbolAlphaDigit",
                          "TokenBigramSplitSymbolAlphaDigit")
  end

  def test_normalize_bigram_ignore_blank
    assert_normalize_type("TokenBigramIgnoreBlank", "bigram_ignore_blank")
    assert_normalize_type("TokenBigramIgnoreBlank", "token_bigram_ignore_blank")
    assert_normalize_type("TokenBigramIgnoreBlank", "TokenBigramIgnoreBlank")
  end

  def test_normalize_bigram_ignore_blank_split_symbol
    assert_normalize_type("TokenBigramIgnoreBlankSplitSymbol",
                          "bigram_ignore_blank_split_symbol")
    assert_normalize_type("TokenBigramIgnoreBlankSplitSymbol",
                          "token_bigram_ignore_blank_split_symbol")
    assert_normalize_type("TokenBigramIgnoreBlankSplitSymbol",
                          "TokenBigramIgnoreBlankSplitSymbol")
  end

  def test_normalize_bigram_ignore_blank_split_symbol_alpha
    assert_normalize_type("TokenBigramIgnoreBlankSplitSymbolAlpha",
                          "bigram_ignore_blank_split_symbol_alpha")
    assert_normalize_type("TokenBigramIgnoreBlankSplitSymbolAlpha",
                          "token_bigram_ignore_blank_split_symbol_alpha")
    assert_normalize_type("TokenBigramIgnoreBlankSplitSymbolAlpha",
                          "TokenBigramIgnoreBlankSplitSymbolAlpha")
  end

  def test_normalize_bigram_ignore_blank_split_symbol_alpha_digit
    assert_normalize_type("TokenBigramIgnoreBlankSplitSymbolAlphaDigit",
                          "bigram_ignore_blank_split_symbol_alpha_digit")
    assert_normalize_type("TokenBigramIgnoreBlankSplitSymbolAlphaDigit",
                          "token_bigram_ignore_blank_split_symbol_alpha_digit")
    assert_normalize_type("TokenBigramIgnoreBlankSplitSymbolAlphaDigit",
                          "TokenBigramIgnoreBlankSplitSymbolAlphaDigit")
  end

  def test_normalize_trigram
    assert_normalize_type("TokenTrigram", "trigram")
    assert_normalize_type("TokenTrigram", "token_trigram")
    assert_normalize_type("TokenTrigram", "TokenTrigram")
  end

  def test_normalize_mecab
    assert_normalize_type("TokenMecab", "mecab")
    assert_normalize_type("TokenMecab", "token_mecab")
    assert_normalize_type("TokenMecab", "TokenMecab")
  end

  private
  def assert_normalize_type(expected, type)
    assert_equal(expected, Groonga::Schema.normalize_type(type))
  end
end
