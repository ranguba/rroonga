# -*- coding: utf-8 -*-
#
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

module Groonga
  class TooSmallPage < Error
    attr_reader :page, :available_pages
    def initialize(page, available_pages)
      @page = page
      @available_pages = available_pages
      super("too small page: #{@page}: " +
            "available pages: #{@available_pages.inspect}")
    end
  end

  class TooLargePage < Error
    attr_reader :page, :available_pages
    def initialize(page, available_pages)
      @page = page
      @available_pages = available_pages
      super("too large page: #{@page}: " +
            "available pages: #{@available_pages.inspect}")
    end
  end

  class TooSmallPageSize < Error
    attr_reader :page_size, :available_page_sizes
    def initialize(page_size, available_page_sizes)
      @page_size = page_size
      @available_page_sizes = available_page_sizes
      super("too small page size: #{@page_size}: " +
            "available page sizes: #{@available_page_sizes.inspect}")
    end
  end

  class Table

    # ページネーション用便利メソッド。ページネーションをした
    # い場合は #sort よりも #paginate の方が便利。
    #
    # 説明文（descriptionカラム）を「Ruby」で全文検索し、検
    # 索結果をスコアの高い順にソートして、10項目ずつ表示する
    # 場合は以下のようになる。
    #
    #   query = "Ruby"
    #   entries = Groonga["entries"]
    #   selected_entries = entries.select do |record|
    #     entry.description =~ query
    #   end
    #   paged_entries = selected_entries.paginate(["_score", :desc],
    #                                             :page => 1,
    #                                             :size => 10)
    #
    # #sort と違い、返されるTableオブジェクトにはPagination
    # モジュールがextendされており、以下のようにページネーショ
    # ン情報を取得できる。
    #
    #   puts "#{paged_entries.n_records}件ヒット"
    #   puts "#{paged_entries.start_offset}-#{paged_entries.end_offset}件を表示"
    #   paged_entries.each do |entry|
    #     puts entry.description
    #   end
    #
    # _sort_keys_ には ソートに用いる情報を指定する。
    # 指定の仕方は #sort と同様なので、詳細は #sort を参照。
    #
    # _options_に指定可能な値は以下の通り。
    #
    # [+:size+]
    #   1ページあたりに表示する最大項目数。デフォルトは10。
    #
    # [+:page+]
    #   ページ番号。ページ番号は0ベースではなく1ベースである
    #   ことに注意。デフォルトは1ページ目。
    def paginate(sort_keys, options={})
      _size = size
      page_size = options[:size] || 10
      minimum_size = [1, _size].min
      if page_size < 1
        raise TooSmallPageSize.new(page_size, minimum_size.._size)
      end

      max_page = [(_size / page_size.to_f).ceil, 1].max
      page = options[:page] || 1
      if page < 1
        raise TooSmallPage.new(page, 1..max_page)
      elsif max_page < page
        raise TooLargePage.new(page, 1..max_page)
      end

      offset = (page - 1) * page_size
      limit = page_size
      records = sort(sort_keys, :offset => offset, :limit => limit)
      records.extend(Pagination)
      records.send(:set_pagination_info, page, page_size, _size)
      records
    end
  end

  # ページネーション機能を追加するモジュール。
  #
  # ページ番号など、0ベースではなく1ベースです。
  module Pagination
    attr_reader :current_page, :page_size, :n_pages, :n_records

    def have_pages?
      @n_pages > 1
    end

    def first_page
      1
    end

    def first_page?
      @current_page == first_page
    end

    def last_page
      @n_pages
    end

    def last_page?
      @current_page == last_page
    end

    def have_next_page?
      @current_page < @n_pages
    end

    def next_page
      have_next_page? ? @current_page + 1 : nil
    end

    def have_previous_page?
      @current_page > 1
    end

    def previous_page
      have_previous_page? ? @current_page - 1 : nil
    end

    def n_records_in_page
      size
    end

    def start_offset
      return nil if @n_records.zero?
      1 + (@current_page - 1) * @page_size
    end

    def end_offset
      return nil if @n_records.zero?
      [start_offset + @page_size - 1, @n_records].min
    end

    def pages
      first_page..last_page
    end

    private
    def set_pagination_info(current_page, page_size, n_records)
      @current_page = current_page
      @page_size = page_size
      @n_records = n_records
      @n_pages = [(@n_records / @page_size.to_f).ceil, 1].max
    end
  end
end
