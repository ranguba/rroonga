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
    def paginate(sort_keys, options={})
      _size = size
      page_size = options[:size] || 10
      minimum_size = [1, _size].min
      if page_size < 1
        raise TooSmallPageSize.new(page_size, minimum_size.._size)
      end

      n_pages = (_size / page_size.to_f).ceil
      available_pages = 0..n_pages
      page = options[:page] || 1
      if page < 1
        raise TooSmallPage.new(page, 1..n_pages)
      elsif n_pages < page
        raise TooLargePage.new(page, 1..n_pages)
      end

      offset = (page - 1) * page_size
      limit = page_size
      records = sort(sort_keys, :offset => offset, :limit => limit)
      records.extend(Pagination)
      records.set_pagination_info(page, page_size, _size)
      records
    end
  end

  # ページネーション機能を追加するモジュール。
  module Pagination
    attr_reader :current_page, :page_size, :n_pages, :n_records

    def set_pagination_info(current_page, page_size, n_records)
      @current_page = current_page
      @page_size = page_size
      @n_records = n_records
      @n_pages = (@n_records / @page_size.to_f).ceil
    end

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

    def record_range_in_page
      start_offset = 1 + (@current_page - 1) * @page_size
      end_offset = [start_offset + @page_size - 1, @n_records].min
      start_offset..end_offset
    end
  end
end
