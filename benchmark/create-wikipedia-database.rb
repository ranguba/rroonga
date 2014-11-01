# Wikipedia data: http://download.wikimedia.org/jawiki/latest/jawiki-latest-pages-articles.xml.bz2

require "time"
require "fileutils"
require "groonga"

require "nokogiri"

class WikipediaExtractor
  def initialize(listener)
    @listener = listener
  end

  def extract(input)
    extractor = Extractor.new(@listener)
    parser = Nokogiri::XML::SAX::Parser.new(extractor)
    parser.parse(input)
  end

  class Extractor
    def initialize(listener)
      @listener = listener
      @name_stack = []
      @text_stack = []
      @contributor_stack = []
      @page = {}
    end

    def start_document
    end

    def end_document
    end

    def start_element_namespace(name, attrs=[], prefix=nil, uri=nil, ns=[])
      @name_stack << name
      @text_stack << ""
      case @name_stack.join(".")
      when "mediawiki.page"
        @page = {}
      when "mediawiki.page.revision.contributor"
        @contributor_stack << {}
      end
    end

    def end_element_namespace(name, prefix=nil, uri=nil)
      case @name_stack.join(".")
      when "mediawiki.page"
        @listener.page(@page)
      when "mediawiki.page.title"
        title = @text_stack.last
        @page[:title] = @listener.title(title) || title
      when "mediawiki.page.revision.timestamp"
        timestamp = Time.parse(@text_stack.last)
        @page[:timestamp] = @listener.timestamp(timestamp) || timestamp
      when "mediawiki.page.revision.contributor"
        contributor = @contributor_stack.pop
        @page[:contributor] = @listener.contributor(contributor) || contributor
      when "mediawiki.page.revision.contributor.id"
        @contributor_stack.last[:id] = Integer(@text_stack.last)
      when "mediawiki.page.revision.contributor.username"
        @contributor_stack.last[:name] = @text_stack.last
      when "mediawiki.page.revision.text"
        content = @text_stack.last
        @page[:content] = @listener.content(content) || content
      end
      @name_stack.pop
      @text_stack.pop
    end

    def characters(string)
      elements_without_interested_text = [
                                          "mediawiki", "siteinfo", "case",
                                          "namespaces", "revisions",
                                          "contributor",
                                         ]
      return if elements_without_interested_text.include?(@name_stack.last)
      @text_stack.last << string
    end

    def xmldecl(*arguments, &block)
    end

    def comment(string)
    end

    def warning(string)
    end

    def error(string)
    end

    def cdata_block(string)
    end
  end
end

class WikipediaImporter
  def initialize(groonga_loader)
    @groonga_loader = groonga_loader
  end

  def title(title)
  end

  def timestamp(timestamp)
  end

  def contributor(contributor)
  end

  def content(content)
  end

  def page(page)
    @groonga_loader.load(page)
  end
end

module TimeDrilldownable
  def define_time_columns(table)
    table.int32("year")
    table.int32("month")
    table.short_text("date")
    table.int32("wday")
    table.int32("hour")
  end

  def add_time(table, key, time)
    table[key]["year"] = time.year
    table[key]["month"] = time.month
    table[key]["date"] = time.strftime("%m/%d")
    table[key]["wday"] = time.wday
    table[key]["hour"] = time.hour
  end
end

class GroongaLoader
  include TimeDrilldownable

  def initialize
    FileUtils.rm_rf("/tmp/wikipedia-db")
    FileUtils.mkdir_p("/tmp/wikipedia-db")
    @context = Groonga::Context.new
    @context.create_database("/tmp/wikipedia-db/db")

    Groonga::Schema.define(:context => @context) do |schema|
      schema.create_table("Users", :type => :hash, :key_type => "Int64") do |table|
        table.short_text("name")
      end

      schema.create_table("Documents", :type => :patricia_trie, :key_type => "ShortText") do |table|
        table.long_text("content")
        table.time("timestamp")
        define_time_columns(table)
        table.reference("last_contributor", "Users")
        table.column("links", "Documents", :type => :vector)
      end

      schema.create_table("Terms", :type => :hash, :default_tokenizer => "TokenBigram") do |table|
        table.index("Documents._key")
        table.index("Documents.content")
      end
    end
    @documents = @context["Documents"]
    @users = @context["Users"]
    @terms = @context["Terms"]
  end

  LOCK_TIMEOUT_SECONDS = 10
  def lock
    @context.database.lock(:timeout => LOCK_TIMEOUT_SECONDS * 1000) do
      yield
    end
  end

  def load(page)
    lock do
      do_load(page)
    end
  end

  def do_load(page)
    content = page.delete(:content)
    timestamp = page.delete(:timestamp)
    title = page.delete(:title)
    contributor = page.delete(:contributor)

    puts "loading: #{title}"
    @documents.add(title, :content => content, :timestamp => timestamp)
    load_links(title, content)
    add_time(@documents, title, timestamp)

    if not contributor.empty?
      @documents.add(title, :last_contributor => contributor[:id])
      @users[contributor[:id]][:name] = contributor[:name]
    end
  end

  def load_links(title, content)
    links = content.scan(/\[\[.*?\]\]/)
    links = links.collect do |link|
      link.sub(/\A\[\[/, "").sub(/\]\]\z/, "").sub(/\|[^\|]+\z/, "")
    end
    @documents.add(title, :links => links)
  end
end

if __FILE__ == $0
  extractor = WikipediaExtractor.new(WikipediaImporter.new(GroongaLoader.new))
  extractor.extract(ARGF)
end
