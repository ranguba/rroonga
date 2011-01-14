require 'fileutils'
require 'groonga'

require 'nokogiri'

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

module Groonga
  class Context
    def create_patricia_trie(name, options=nil, &block)
      create_table(PatriciaTrie, name, options, &block)
    end

    def create_hash(name, options=nil, &block)
      create_table(Hash, name, options, &block)
    end

    private
    def create_table(table_class, name, options=nil, &block)
      options ||= {}
      options = options.merge(:name => name, :context => self)
      table_class.create(options, &block)
    end
  end
end

module TimeDrilldownable
  def define_columns(table)
    table.define_column("year", "Int32")
    table.define_column("month", "Int32")
    table.define_column("date", "ShortText")
    table.define_column("wday", "Int32")
    table.define_column("hour", "Int32")
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
    @documents = @context.create_patricia_trie("Documents")
    @documents.define_column("content", "LongText")
    @documents.define_column("timestamp", "Time")
    define_columns(@documents)

    @terms = @context.create_hash("Terms", :default_tokenizer => "TokenBigram")
    @terms.define_index_column("title", @documents, :source => "Documents._key") # :(
    @terms.define_index_column("content", @documents, :source => "Documents.content")
  end

  def load(page)
    content = page.delete(:content)
    timestamp = page.delete(:timestamp)
    title = page.delete(:title)
    @documents.add(title, :content => content, :timestamp => timestamp)
    add_time(@documents, title, timestamp)
    pp page
  end
end

begin
  extractor = WikipediaExtractor.new(WikipediaImporter.new(GroongaLoader.new))
  extractor.extract(ARGF)
rescue Exception => error
  pp error
  pp error.backtrace
end
