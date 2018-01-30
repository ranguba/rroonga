# -*- coding: utf-8 -*-
#
# Copyright (C) 2012-2013  Kouhei Sutou <kou@clear-code.com>
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

require "fileutils"
require "cgi"

module Groonga
  class IndexColumn
    def dump(output_directory)
      dumper = IndexColumnDumper.new(self, output_directory)
      dumper.dump
    end
  end

  class IndexColumnDumper
    def initialize(column, output_directory)
      @column = column
      @output_directory = output_directory
      @sources = @column.sources
    end

    def dump
      dump_indexes
    end

    private
    def dump_indexes
      @column.table.open_cursor do |table_cursor|
        @column.open_cursor(table_cursor) do |cursor|
          postings = []
          cursor.each do |posting|
            if postings.empty?
              postings << posting
              next
            end

            current_term_posting = postings.first
            unless same_term_posting?(current_term_posting, posting)
              dump_postings(postings)
              postings.clear
            end

            postings << posting
          end
          dump_postings(postings)
        end
      end
    end

    def same_term_posting?(posting1, posting2)
      posting1.term_id == posting2.term_id
    end

    def dump_file_info(posting)
      items = [
        "index: #{@column.name}",
        "term: <#{posting.term.key}>",
        "domain: #{@column.domain.name}",
        "range: #{@column.range.name}",
        "have_section: #{@column.with_section?}",
        "have_weight: #{@column.with_weight?}",
        "have_position: #{@column.with_position?}",
      ]
      info = items.join("\t")
      @output.write("#{info}\n")
    end

    def dump_posting_header
      header_items = [
        "weight",
        "position",
        "term_frequency",
        "record",
      ]
      header = header_items.join("\t")
      @output.write("  #{header}\n")
    end

    def encode_term(term)
      CGI.escape(term.to_s)
    end

    def dump_postings(postings)
      return if postings.empty?

      distinctive_posting = postings.first
      term = distinctive_posting.term.key
      encoded_term = encode_term(term)
      output_dir = File.join(@output_directory, @column.name)
      output_path = File.join(output_dir, "#{encoded_term}.dump")
      FileUtils.mkdir_p(output_dir)
      File.open(output_path, "w") do |output|
        @output = output
        dump_file_info(distinctive_posting)
        dump_posting_header
        sorted_postings = postings.sort_by do |posting|
          [source_column_name(posting), record_key(posting), posting.position]
        end
        sorted_postings.each do |posting|
          dump_posting(posting)
        end
      end
    end

    def dump_posting(posting)
      found_record = "#{posting.table.name}[#{posting.record.record_id}]"
      posting_info_items = [
        "#{posting.weight}",
        "#{posting.position}",
        "#{posting.term_frequency}",
        "#{found_record}.#{source_column_name(posting)}",
      ]
      posting_info = posting_info_items.join("\t")
      @output.write("  #{posting_info}\n")
    end

    def term(posting)
      posting.term.key
    end

    def record_key(posting)
      posting.record.key || default_key(posting)
    end

    def default_key(posting)
      type = posting.table.domain
      return 0 if type.is_a?(Groonga::Table)

      case type.name
      when "ShortText", "Text", "LongText"
        ""
      when "TokyoGeoPoint"
        Groonga::TokyoGeoPoint.new(0, 0)
      when "WGS84GeoPoint"
        Groonga::WGS84GeoPoint.new(0, 0)
      when "Bool"
        true
      when "Time"
        Time.at(0)
      else
        0
      end
    end

    def source_column_name(posting)
      source = @sources[posting.section_id - 1]
      if source.nil?
        "<invalid section: #{posting.section_id}>"
      elsif source.is_a?(Groonga::Table)
        "_key"
      else
        source.local_name
      end
    end
  end
end
