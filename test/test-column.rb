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

class ColumnTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    setup_database

    setup_users_table
    setup_bookmarks_table
    setup_indexes
  end

  def setup_users_table
    @users_path = @tables_dir + "users"
    @users = Groonga::Array.create(:name => "Users",
                                   :path => @users_path.to_s)

    @users_name_column_path = @columns_dir + "name"
    @users_name_column =
      @users.define_column("name", "ShortText",
                           :path => @users_name_column_path.to_s)
  end

  def setup_bookmarks_table
    @bookmarks_path = @tables_dir + "bookmarks"
    @bookmarks = Groonga::Array.create(:name => "Bookmarks",
                                       :path => @bookmarks_path.to_s)

    @uri_column_path = @columns_dir + "uri"
    @bookmarks_uri = @bookmarks.define_column("uri", "ShortText",
                                              :path => @uri_column_path.to_s)

    @comment_column_path = @columns_dir + "comment"
    @bookmarks_comment =
      @bookmarks.define_column("comment", "Text",
                               :path => @comment_column_path.to_s)

    @content_column_path = @columns_dir + "content"
    @bookmarks_content = @bookmarks.define_column("content", "LongText")

    @user_column_path = @columns_dir + "user"
    @bookmarks_user = @bookmarks.define_column("user", @users)
  end

  def setup_indexes
    @bookmarks_index_path = @tables_dir + "bookmarks-index"
    @bookmarks_index =
      Groonga::PatriciaTrie.create(:name => "BookmarksIndex",
                                   :path => @bookmarks_index_path.to_s,
                                   :key_type => "ShortText")
    @bookmarks_index.default_tokenizer = "TokenBigram"

    @content_index_column_path = @columns_dir + "content-index"
    @bookmarks_index_content =
      @bookmarks_index.define_index_column("content", @bookmarks,
                                           :with_section => true,
                                           :with_weight => true,
                                           :with_position => true,
                                           :path => @content_index_column_path.to_s)

    @uri_index_column_path = @columns_dir + "uri-index"
    @bookmarks_index_uri =
      @bookmarks_index.define_index_column("uri", @bookmarks,
                                           :path => @uri_index_column_path.to_s)
  end

  def test_source_info
    @bookmarks_index_content.sources = [@bookmarks_content]

    groonga = @bookmarks.add
    groonga["content"] = "<html><body>groonga</body></html>"

    ruby = @bookmarks.add
    ruby["content"] = "<html><body>ruby</body></html>"

    assert_content_search([groonga], "groonga")
    assert_content_search([groonga, ruby], "html")

    assert_equal([@bookmarks_content], @bookmarks_index_content.sources)
  end

  def test_update_index_column
    groonga = @bookmarks.add
    groonga["content"] = "<html><body>groonga</body></html>"

    ruby = @bookmarks.add
    ruby["content"] = "<html><body>ruby</body></html>"

    @bookmarks_index_content[groonga.id] = groonga["content"]
    @bookmarks_index_content[ruby.id] = ruby["content"]

    assert_content_search([groonga], "groonga")
    assert_content_search([groonga, ruby], "html")
  end

  def test_range
    assert_equal(context[Groonga::Type::SHORT_TEXT], @bookmarks_uri.range)
    assert_equal(context[Groonga::Type::TEXT], @bookmarks_comment.range)
    assert_equal(context[Groonga::Type::LONG_TEXT], @bookmarks_content.range)
    assert_equal(@users, @bookmarks_user.range)
    assert_equal(@bookmarks, @bookmarks_index_content.range)
    assert_equal(@bookmarks, @bookmarks_index_uri.range)
  end

  def test_accessor
    posts = Groonga::Hash.create(:name => "Posts", :key_type => "ShortText")
    posts.define_column("body", "Text")
    comments = Groonga::Hash.create(:name => "Comments",
                                    :key_type => "ShortText")
    content = comments.define_column("content", "ShortText")
    comments.define_column("post", posts)

    index = Groonga::PatriciaTrie.create(:name => "Terms",
                                         :key_type => "ShortText")
    index.default_tokenizer = "TokenBigram"
    content_index = index.define_index_column("content_index", comments,
                                              :with_position => true)
    content_index.source = content

    first_post = posts.add("Hello!")
    first_post["body"] = "World"
    hobby = posts.add("My Hobby")
    hobby["body"] = "Drive and Eat"

    friend = comments.add("Me too")
    friend["content"] = "I'm also like drive"
    friend["post"] = hobby

    result = content_index.search("drive")
    assert_equal([["I'm also like drive", "My Hobby"]],
                 result.records.collect do |record|
                   [record[".content"], record[".post._key"]]
                 end)
  end

  def test_accessor_reference
    bookmark = @bookmarks.add
    assert_nil(@bookmarks_user[bookmark.id])

    daijiro = @users.add
    daijiro["name"] = "daijiro"
    @bookmarks_user[bookmark.id] = daijiro
    assert_equal(daijiro, @bookmarks_user[bookmark.id])
  end

  def test_array_set_with_key_of_table
    languages = Groonga::Hash.create(:name => "Languages",
                                     :key_type => "ShortText")
    sites = Groonga::Hash.create(:name => "Sites")
    sites.define_column("language", languages)

    languages.add("Ruby")
    taiyaki_ru = sites.add("http://taiyaki.ru/", :language => "Ruby")
    assert_equal("Ruby", taiyaki_ru[:language].key)
  end

  def test_local_name
    items = Groonga::Array.create(:name => "Items")
    title = items.define_column("title", "ShortText")
    assert_equal("Items.title", title.name)
    assert_equal("title", title.local_name)
  end

  def test_select_query
    populate_table_for_select

    result = @body.select("drive")
    assert_equal(["Drive and Eat"],
                 result.records.collect do |record|
                   record["body"]
                 end)
    assert_equal("#<Groonga::Expression noname($1:\"\")" +
                 "{2body GET_VALUE,0\"drive\",0MATCH}>",
                 result.expression.inspect)
  end

  def test_select_query_from_ctx
    populate_table_for_select

    body = Groonga::Context.default['Posts.body']
    # select twice.
    result = body.select("drive")
    assert_equal(["Drive and Eat"],
                 result.records.collect do |record|
                   record["body"]
                 end)

    result = body.select("drive")
    assert_equal(["Drive and Eat"],
                 result.records.collect do |record|
                   record["body"]
                 end)
  end

  def test_select_query_with_parser
    populate_table_for_select

    result = @body.select("body @ \"drive\"", :syntax => :script)
    assert_equal(["Drive and Eat"],
                 result.records.collect do |record|
                   record["body"]
                 end)
    assert_equal("#<Groonga::Expression noname($1:\"\")" +
                 "{2body GET_VALUE,0\"drive\",0MATCH}>",
                 result.expression.inspect)
  end

  def test_select_expression
    populate_table_for_select

    expression = Groonga::Expression.new
    variable = expression.define_variable(:domain => @posts)
    expression.append_object(variable)
    expression.parse("body:@drive", :syntax => :query)
    expression.compile
    result = @body.select(expression)
    assert_equal(["Drive and Eat"],
                 result.records.collect do |record|
                   record["body"]
                 end)
    assert_equal("#<Groonga::Expression noname($1:\"\")" +
                 "{0\"\",2body GET_VALUE,0\"drive\",0MATCH}>",
                 result.expression.inspect)
  end

  def test_select_with_block
    populate_table_for_select

    result = @body.select do |column|
      column =~ "drive"
    end
    assert_equal(["Drive and Eat"],
                 result.records.collect do |record|
                   record["body"]
                 end)
    assert_equal("#<Groonga::Expression noname($1:\"\")" +
                 "{1\"\",2body GET_VALUE,0\"drive\",0MATCH}>",
                 result.expression.inspect)
  end

  def test_set_time
    posts = Groonga::Hash.create(:name => "Posts", :key_type => "ShortText")
    posts.define_column("issued", "Time")

    post = posts.add("hello", :issued => 123456)
    assert_equal(Time.at(123456), post[".issued"])
    post = posts.add("groonga", :issued => 1251380635)
    assert_equal(Time.parse("2009-08-27 22:43:55"), post[".issued"])
    post = posts.add("mroonga", :issued => 1251380635.1234567)
    assert_in_delta(Time.at(1251380635.1234567).usec, post[".issued"].usec, 10)
  end

  def test_set_nil_to_time
    posts = Groonga::Hash.create(:name => "Posts", :key_type => "ShortText")
    posts.define_column("issued", "Time")

    post = posts.add("hello", :issued => nil)
    assert_equal(Time.at(0), post["issued"])
  end

  def test_bool
    posts = Groonga::Hash.create(:name => "Posts", :key_type => "ShortText")
    posts.define_column("hidden", "Bool")

    post = posts.add("hello")
    assert_false(post["hidden"])

    post["hidden"] = true
    assert_true(post["hidden"])

    post["hidden"] = false
    assert_false(post["hidden"])
  end

  def test_indexes
    Groonga::Schema.define do |schema|
      schema.create_table("Comments") do |table|
        table.short_text("title")
      end
    end
    title = Groonga["Comments.title"]
    assert_equal([], title.indexes)

    Groonga::Schema.define do |schema|
      schema.create_table("Terms",
                          :type => :patricia_trie,
                          :default_tokenizer => "TokenBigram") do |table|
        table.index("Comments.title")
      end
    end
    assert_equal([Groonga["Terms.Comments_title"]],
                 title.indexes)

    Groonga::Schema.define do |schema|
      schema.create_table("Titles",
                          :type => :hash) do |table|
        table.index("Comments.title")
      end
    end
    assert_equal([Groonga["Titles.Comments_title"],
                  Groonga["Terms.Comments_title"]],
                 title.indexes)
  end

  def test_builtin?
    assert_not_predicate(@users_name_column, :builtin?)
  end

  private
  def assert_content_search(expected_records, term)
    records = @bookmarks_index_content.search(term).records
    expected_contents = expected_records.collect do |record|
      record["content"]
    end
    actual_contents = records.collect do |record|
      record.key["content"]
    end
    assert_equal(expected_contents, actual_contents)
  end

  def populate_table_for_select
    @posts = Groonga::Hash.create(:name => "Posts", :key_type => "ShortText")
    @body = @posts.define_column("body", "Text")

    index = Groonga::PatriciaTrie.create(:name => "Terms",
                                         :key_type => "ShortText",
                                         :key_normalize => true)
    index.default_tokenizer = "TokenBigram"
    index.define_index_column("body_index", @posts,
                              :with_position => true,
                              :source => @body)

    @posts.add("Hello!", :body => "World")
    @posts.add("My Hobby", :body => "Drive and Eat")
  end
end
