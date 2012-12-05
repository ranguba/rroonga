# -*- coding: utf-8 -*-
#
# Copyright (C) 2012  Kouhei Sutou <kou@clear-code.com>
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

module Groonga
  class IndexColumn
    def dump(output)
      dumper = IndexColumnDumper.new(self, output)
      dumper.dump
    end
  end

  class IndexColumnDumper
    def initialize(column, output)
      @column = column
      @output = output
      @sources = @column.sources
    end

    def dump
      dump_column_info
      dump_indexes
    end

    private
    def dump_column_info
      column_info_items = [
        "name: #{@column.name}",
        "domain: #{@column.domain.name}",
        "range: #{@column.range.name}",
        "have_section: #{@column.with_section?}",
        "have_weight: #{@column.with_weight?}",
        "have_position: #{@column.with_position?}",
      ]
      column_info = column_info_items.join("\t")
      @output.write("#{column_info}\n")
    end

    def dump_indexes
      dump_posting_header
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

    def dump_posting_header
      header_items = [
        "term",
        "weight",
        "position",
        "term_frequency",
        "record",
      ]
      header = header_items.join("\t")
      @output.write("  #{header}\n")
    end

    def dump_postings(postings)
      sorted_postings = postings.sort_by do |posting|
        [source_column_name(posting), record_key(posting), posting.position]
      end
      sorted_postings.each do |posting|
        dump_posting(posting)
      end
    end

    def dump_posting(posting)
      found_record = "#{posting.table.name}[#{posting.record.record_id}]"
      posting_info_items = [
        "<#{term(posting)}>",
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
