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

class VariableSizeColumnTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database

    setup_users_table
    setup_users
  end

  def setup_users_table
    @users_path = @tables_dir + "users"
    @users = Groonga::Array.create(:name => "Users",
                                   :path => @users_path.to_s)

    @users_name_column_path = @columns_dir + "name"
    @name = @users.define_column("name", "ShortText",
                                 :path => @users_name_column_path.to_s)

    @users_friends_column_path = @columns_dir + "friends"
    @friends = @users.define_column("friends", @users,
                                    :type => :vector,
                                    :path => @users_friends_column_path.to_s)

    @users_nick_names_column_path = @columns_dir + "nick_names"
    @nick_names =
      @users.define_column("nick_names", "ShortText",
                           :type => :vector,
                           :path => @users_nick_names_column_path.to_s)
  end

  def setup_users
    @morita = @users.add(:name => "mori daijiro")
    @gunyara_kun = @users.add(:name => "Tasuku SUENAGA")
    @yu = @users.add(:name => "Yutaro Shimamura")
  end

  def test_index?
    assert_not_predicate(@nick_names, :index?)
  end

  def test_vector?
    assert_predicate(@nick_names, :vector?)
  end

  def test_scalar?
    assert_not_predicate(@nick_names, :scalar?)
  end

  def test_compressed?
    description = @users.define_column("description", "ShortText",
                                       :compress => :zlib)
    if context.support_zlib?
      assert_predicate(description, :compressed?)
    else
      assert_not_predicate(description, :compressed?)
    end
  end

  def test_compressed_zlib?
    description = @users.define_column("description", "ShortText",
                                       :compress => :zlib)
    if context.support_zlib?
      assert_send([description, :compressed?, :zlib])
    else
      assert_not_send([description, :compressed?, :zlib])
    end
  end

  def test_compressed_lzo?
    description = @users.define_column("description", "ShortText",
                                       :compress => :lzo)
    if context.support_lzo?
      assert_send([description, :compressed?, :lzo])
    else
      assert_not_send([description, :compressed?, :lzo])
    end
  end

  def test_inspect
    assert_equal("#<Groonga::VariableSizeColumn " +
                 "id: <#{@name.id}>, " +
                 "name: <Users.name>, " +
                 "path: <#{@users_name_column_path}>, " +
                 "domain: <Users>, " +
                 "range: <ShortText>, " +
                 "flags: <>" +
                 ">",
                 @name.inspect)
  end

  def test_domain
    assert_equal(@users, @name.domain)
  end

  def test_table
    assert_equal(@users, @name.table)
  end

  def test_vector_append
    assert_equal([], @morita["friends"])
    @morita.append("friends", @yu)
    assert_equal([@yu], @morita["friends"])
    @morita.append("friends", @gunyara_kun)
    assert_equal([@yu, @gunyara_kun], @morita["friends"])
  end

  def test_vector_prepend
    assert_equal([], @morita["friends"])
    @morita.prepend("friends", @yu)
    assert_equal([@yu], @morita["friends"])
    @morita.prepend("friends", @gunyara_kun)
    assert_equal([@gunyara_kun, @yu], @morita["friends"])
  end

  def test_string_vector
    omit("append/prepend for non table domain column " +
         "isn't supported by groonga.")
    assert_equal([], @morita["nick_names"])
    @morita.append("nick_names", "morita")
    assert_equal(["morita"], @morita["nick_names"])
    @morita.prepend("nick_names", "moritapo")
    assert_equal(["moritapo", "morita"], @morita["nick_names"])
  end

  def test_defrag
    1000.times do |i|
      @users.add(:name => "user #{i}" * 1000)
    end
    assert_equal(3, @name.defrag)
  end
end
