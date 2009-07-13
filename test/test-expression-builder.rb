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

class ExpressionBuilderTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database
  setup :setup_tables
  setup :setup_data

  def setup_tables
    @users = Groonga::Hash.create(:name => "<users>")
    @name = @users.define_column("name", "<shorttext>")
    @hp = @users.define_column("hp", "<uint>")

    @terms = Groonga::PatriciaTrie.create(:name => "<terms>",
                                          :default_tokenizer => "TokenBigram")
    @terms.define_index_column("user-name", @users, :source => @name)
  end

  def setup_data
    @morita = @users.add("morita",
                         :name => "mori daijiro",
                         :hp => 100)
    @gunyara_kun = @users.add("gunyara-kun",
                              :name => "Tasuku SUENAGA",
                              :hp => 150)
    @yu = @users.add("yu",
                     :name => "Yutaro Shimamura",
                     :hp => 200)
  end

  def test_equal
    result = @users.select do |record|
      record["name"] == "mori daijiro"
    end
    assert_equal(["morita"],
                 result.collect {|record| record.key.key})
  end

  def test_not_equal
    omit("not supported yet!!!")
    result = @users.select do |record|
      record["name"] != "mori daijiro"
    end
    assert_equal(["gunyara-kun", "yu"],
                 result.collect {|record| record.key.key})
  end

  def test_less
    result = @users.select do |record|
      record["hp"] < 150
    end
    assert_equal(["morita"], result.collect {|record| record.key.key})
  end

  def test_less_equal
    result = @users.select do |record|
      record["hp"] <= 150
    end
    assert_equal(["morita", "gunyara-kun"],
                 result.collect {|record| record.key.key})
  end

  def test_greater
    result = @users.select do |record|
      record["hp"] > 150
    end
    assert_equal(["yu"], result.collect {|record| record.key.key})
  end

  def test_greater_equal
    result = @users.select do |record|
      record["hp"] >= 150
    end
    assert_equal(["gunyara-kun", "yu"],
                 result.collect {|record| record.key.key})
  end

  def test_and
    result = @users.select do |record|
      (record["hp"] > 100) & (record["hp"] <= 200)
    end
    assert_equal(["gunyara-kun", "yu"],
                 result.collect {|record| record.key.key})
  end

  def test_match
    result = @users.select do |record|
      record["name"] =~ "ro"
    end
    assert_equal(["morita", "yu"],
                 result.collect {|record| record.key.key})
  end
end
