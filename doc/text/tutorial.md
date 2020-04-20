# Tutorial

This page introduce how to use Rroonga via a simple application making.

## Install

You can install Rroonga in your compter with RubyGems.

    % sudo gem install rroonga

## Create Database

Let's create database for simple bookmark application.
Please execute irb with loading Rroonga with this command:

    % irb --simple-prompt -r groonga
    >>

Then, try to create database in a file.

    >> Groonga::Database.create(:path => "/tmp/bookmark.db")
    => #<Groonga::Database ...>

From now, the created database is used implicitly.
You don't have to be aware of it after you created a database first.

## Define table

Groonga supports 4 types of tables.

Groonga::Hash
:   Hash table. It manages records via each primary key. It supports
    very quickly exact match search.

Groonga::PatriciaTrie
:   Patricia Trie. It supports some search such as predictive search and
    common prefix search, but it provides a little slowly exact match search
    than Groonga::Hash. It provides cursor to take records in ascending
    or descending order.

Groonga::DoubleArrayTrie
:   Double Array Trie. It requires large spaces rather than other
    tables, but it can update key without ID change. It provides exract
    match search, predictive search and common prefix search and cursor
    like Groonga::PatriciaTrie.

Groonga::Array
:   Array. It doesn't have primary keys. It manages records by ID.

Now, you use Groonga::Hash and create the table named `Items`. The type
of its primary key is String.

    >> Groonga::Schema.create_table("Items", :type => :hash)
    => [...]

You have `Items` table by this code.
You can refer the defined table with Groonga.[] like below:

    >> items = Groonga["Items"]
    => #<Groonga::Hash ...>

You can treat it like Hash.
For example, let's type `items.size` to get the number of records in
the table.

    >> items.size
    => 0

## Add records

Let's add records to `Items` table.

    >> items.add("http://en.wikipedia.org/wiki/Ruby")
    => #<Groonga::Record ...>
    >> items.add("http://www.ruby-lang.org/")
    => #<Groonga::Record ...>

Please check the number of records. It increases from 0 to 2.

    >> items.size
    => 2

If you can get record by primary key, type like below:

    >> items["http://en.wikipedia.org/wiki/Ruby"]
    => #<Groonga::Record ...>

## Full text search

Let's add item's title to full text search.

first, you add the `Text` type column "`title`" to `Items` table.

    >> Groonga::Schema.change_table("Items") do |table|
    ?>     table.text("title")
    >>   end
    => [...]

Defined columns is named as `#{TABLE_NAME}.#{COLUMN_NAME}`.
You can refer them with {Groonga.[]} as same as tables.

    >> title_column = Groonga["Items.title"]
    => #<Groonga::VariableSizeColumn ...>


Secondly, let's add the table containing terms from splited from texts.
Then you define the `Terms` for it.

    >> Groonga::Schema.create_table("Terms",
    ?>                              :type => :patricia_trie,
    ?>                              :normalizer => :NormalizerAuto,
    ?>                              :default_tokenizer => "TokenBigram")

You specify `:default_tokenzier => "TokenBigram"` for "Tokenizer" in
the above code.
"Tokenizer" is the object to split terms from texts. The default value
for it is none.
Full text search requires a tokenizer, so you specify "Bigram", a type
of N-gram.
Full text search with N-gram uses splited N characters and their
position in texts. "N" in N-gram specifies the number of each terms.
Groonga supports Unigram (N=1), Bigram (N=2) and Trigram (N=3).

You also specify `:normalizer => :NormalizerAuto` to search texts with
ignoring the case.

Now, you ready table for terms, so you define the index of
`Items.tiltle` column.

    >> Groonga::Schema.change_table("Terms") do |table|
    ?>     table.index("Items.title")
    >>   end
    => [...]

You may feel a few unreasonable code. The index of `Items` table's
column is defined as the column in `Terms`.

When a record is added to `Items`, groonga adds records associated
each terms in it to `Terms` automatically.


`Terms` is a few particular table, but you can add some columns to term
table such as `Terms` and manage many attributes of each terms. It is
very useful to process particular search.

Now, you finished table definition.
Let's put some values to `title` of each record you added before.

    >> items["http://en.wikipedia.org/wiki/Ruby"].title = "Ruby"
    => "Ruby"
    >> items["http://www.ruby-lang.org/"].title = "Ruby Programming Language"
    "Ruby Programming Language"

Now, you can do full text search like above:

    >> ruby_items = items.select {|record| record.title =~ "Ruby"}
    => #<Groonga::Hash ..., normalizer: (nil)>

Groonga returns the search result as Groonga::Hash.
Keys in this hash table is records of hitted `Items`.

    >> ruby_items.collect {|record| record.key.key}
    => ["http://en.wikipedia.org/wiki/Ruby", "http://www.ruby-lang.org/"]

In above example, you get records in `Items` with `record.key`, and
keys of them with `record.key.key`.

You can access a refered key in records briefly with `record["_key"]`.

    >> ruby_items.collect {|record| record["_key"]}
    => ["http://en.wikipedia.org/wiki/Ruby", "http://www.ruby-lang.org/"]

## Improve the simple bookmark application

Let's try to improve this simple application a little. You can create
bookmark application for multi users and they can comment to each
bookmarks.

First, you add tables for users and for comments like below:

![Sample schema](images/sample-schema.png)

Let's add the table for users, `Users`.

    >> Groonga::Schema.create_table("Users", :type => :hash) do |table|
    ?>     table.text("name")
    >>   end
    => [...]


Next, let's add the table for comments as `Comments`.

    >> Groonga::Schema.create_table("Comments") do |table|
    ?>     table.reference("item")
    >>   table.reference("author", "Users")
    >>   table.text("content")
    >>   table.time("issued")
    >>   end
    => [...]

Then you define the index of `content` column in `Comments` for full
text search.

    >> Groonga::Schema.change_table("Terms") do |table|
    ?>     table.index("Comments.content")
    >>   end
    => [...]

You finish table definition by above code.

Secondly, you add some users to `Users`.

    >> users = Groonga["Users"]
    => #<Groonga::Hash ...>
    >> users.add("alice", :name => "Alice")
    => #<Groonga::Record ...>
    >> users.add("bob", :name => "Bob")
    => #<Groonga::Record ...>

Now, let's write the process to bookmark by a user.
You assume that the user, `moritan`, bookmark a page including
infomation related Ruby.

First, you check if the page has been added `Items` already.

    >> items.has_key?("http://www.ruby-doc.org/")
    => false

The page hasn't been added, so you add it to `Items`.

    >> items.add("http://www.ruby-doc.org/",
    ?>           :title => "Ruby-Doc.org: Documenting the Ruby Language")
=> #<Groonga::Record ...>

Next, you add the record to `Comments`. This record contains this page
as its `item` column.

    >> require "time"
    => true
    >> comments = Groonga["Comments"]
    => #<Groonga::Array ...>
    >> comments.add(:item => "http://www.ruby-doc.org/",
    ?>              :author => "alice",
    ?>              :content => "Ruby documents",
    ?>              :issued => Time.parse("2010-11-20T18:01:22+09:00"))
    => #<Groonga::Record ...>

## Define methods for this process

For usefull, you define methods for above processes.

    >> @items = items
    => #<Groonga::Hash ...>
    >> @comments = comments
    => #<Groonga::Array ...>
    >> def add_bookmark(url, title, author, content, issued)
    >>   item = @items[url] || @items.add(url, :title => title)
    >>   @comments.add(:item => item,
    ?>                 :author => author,
    ?>                 :content => content,
    ?>                 :issued => issued)
    >>   end
    => nil

You assign `items` and `comments` to each instance variable, so you can
use them in `add_bookmark` method.

`add_bookmark` executes processes like below:

* Check if the record associated the page exists in `Items` table.
* If not, add the record to it.
* Add the record to `Comments` table.

With this method, lets bookmark some pages.

    >> add_bookmark("https://rubygems.org/",
    ?>              "RubyGems.org | your community gem host", "alice", "Ruby gems",
    ?>              Time.parse("2010-10-07T14:18:28+09:00"))
    => #<Groonga::Record ...>
    >> add_bookmark("http://ranguba.org/",
    ?>              "Fulltext search by Ruby with groonga - Ranguba", "bob",
    ?>              "Ruby groonga fulltextsearch",
    ?>              Time.parse("2010-11-11T12:39:59+09:00"))
    => #<Groonga::Record ...>
    >> add_bookmark("http://www.ruby-doc.org/",
    ?>              "ruby-doc", "bob", "ruby documents",
    ?>              Time.parse("2010-07-28T20:46:23+09:00"))
    => #<Groonga::Record ...>

## Full text search part 2

Let's do full text search for added records.

    >> records = comments.select do |record|
    ?>     record["content"] =~ "Ruby"
    >>   end
    => #<Groonga::Hash ...>
    >> records.each do |record|
    ?>     comment = record
    >>   p [comment.id,
    ?>       comment.issued,
    ?>       comment.item.title,
    ?>       comment.author.name,
    ?>       comment.content]
    >>   end
    [1, 2010-11-20 18:01:22 +0900, "Ruby-Doc.org: Documenting the Ruby Language", "Alice", "Ruby documents"]
    [2, 2010-10-07 14:18:28 +0900, "RubyGems.org | your community gem host", "Alice", "Ruby gems"]
    [3, 2010-11-11 12:39:59 +0900, "Fulltext search by Ruby with groonga - Ranguba", "Bob", "Ruby groonga fulltextsearch"]
    [4, 2010-07-28 20:46:23 +0900, "Ruby-Doc.org: Documenting the Ruby Language", "Bob", "ruby documents"]

You can access the columns with the same name method as each them.
These methods suport to access the complex data type.
(In usually RDB, you should namage JOIN tables, `Items`, `Comments`,
`Users`.)

The search is finished when the first sentence in this codes. The
results of this search is the object as records set.

    >> records
    #<Groonga::Hash ..., size: <4>>

You can arrange this records set before output.
For example, sort these records in the descending order by date.

    >> records.sort([{:key => "issued", :order => "descending"}]).each do |record|
    ?>     comment = record
    >>   p [comment.id,
    ?>       comment.issued,
    ?>       comment.item.title,
    ?>       comment.author.name,
    ?>       comment.content]
    >>   end
    [1, 2010-11-20 18:01:22 +0900, "Ruby-Doc.org: Documenting the Ruby Language", "Alice", "Ruby documents"]
    [2, 2010-11-11 12:39:59 +0900, "Fulltext search by Ruby with groonga - Ranguba", "Bob", "Ruby groonga fulltextsearch"]
    [3, 2010-10-07 14:18:28 +0900, "RubyGems.org | your community gem host", "Alice", "Ruby gems"]
    [4, 2010-07-28 20:46:23 +0900, "Ruby-Doc.org: Documenting the Ruby Language", "Bob", "ruby documents"]
    => [...]

Let's group the result by each item for easy view.

    >> records.group("item").each do |record|
    ?>     item = record.key
    >>   p [record.n_sub_records,
    ?>       item.key,
    ?>       item.title]
    >>   end
    [2, "http://www.ruby-doc.org/", "Ruby-Doc.org: Documenting the Ruby Language"]
    [1, "https://rubygems.org/", "RubyGems.org | your community gem host"]
    [1, "http://ranguba.org/", "Fulltext search by Ruby with groonga - Ranguba"]
    => nil

`n_sub_records` is the number of records in each group.
It is similar value as count() function of a query including "GROUP
BY" in SQL.

## more complex search

Now, you challenge the more useful search.

You should calcurate goodness of fit of search explicitly.

You can use `Items.title` and `Comments.content` as search targets now.
`Items.title` is the a few reliable information taken from each
original pages. On the other hands, `Comments.content` is the less
reliable information because this depends on users of bookmark
application.

Then, you search records with this policy:

* Search item matched `Items.title` or `Comments.content`.
* Add 10 times heavier weight to socres of each record matched
  `Items.title` than ones of `Comments.comment`.
* If multi `comment` of one item are matched keyword, specify the sum
  of scores of each `coments` as score of the item.

On this policy, you try to type below:

    >> ruby_comments = @comments.select {|record| record.content =~ "Ruby"}
    => #<Groonga::Hash ..., size: <4>
    >> ruby_items = @items.select do |record|
    ?>     target = record.match_target do |match_record|
    ?>       match_record.title * 10
    >>     end
    >>   target =~ "Ruby"
    >>   end
    #<Groonga::Hash ..., size: <4>>

You group the results of *ruby_comments* in each item and union
*ruby_items* .

    >> ruby_items = ruby_comments.group("item").union!(ruby_items)
    #<Groonga::Hash ..., size: <5>>
    >> ruby_items.sort([{:key => "_score", :order => "descending"}]).each do |record|
    >>   p [record.score, record.title]
    >> end
    [22, "Ruby-Doc.org: Documenting the Ruby Language"]
    [11, "Fulltext search by Ruby with groonga - Ranguba"]
    [10, "Ruby Programming Language"]
    [10, "Ruby"]
    [1, "RubyGems.org | your community gem host"]

Then, you get the result.
