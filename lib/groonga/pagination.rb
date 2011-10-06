# -*- coding: utf-8 -*-
#
# Copyright (C) 2010-2011  Kouhei Sutou <kou@clear-code.com>
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
    # Table#paginate で小さすぎるページ番号を指定した場合に
    # 発生する。

    # 指定したページ番号。
    attr_reader :page
    # 有効なページ。Range。
    attr_reader :available_pages
    def initialize(page, available_pages)
      @page = page
      @available_pages = available_pages
      super("too small page: #{@page}: " +
            "available pages: #{@available_pages.inspect}")
    end
  end

  class TooLargePage < Error
    # Table#paginate で大きすぎるページ番号を指定した場合に
    # 発生する。

    # 指定したページ番号。
    attr_reader :page
    # 有効なページ。Range。
    attr_reader :available_pages
    def initialize(page, available_pages)
      @page = page
      @available_pages = available_pages
      super("too large page: #{@page}: " +
            "available pages: #{@available_pages.inspect}")
    end
  end

  class TooSmallPageSize < Error
    # Table#paginate で小さすぎるページサイズを指定した場合
    # に発生する。

    # 指定したページサイズ。
    attr_reader :page_size
    # 有効なページサイズ。Range。
    attr_reader :available_page_sizes
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
    # 説明文（descriptionカラム）を「Ruby」で全文検索し、
    # 検索結果をスコアの高い順にソートして、10項目ずつ表示する
    # 場合は以下のようになる。
    #
    #   query = "Ruby"
    #   entries = Groonga["entries"]
    #   selected_entries = entries.select do |record|
    #     entry.description =~ query
    #   end
    #   paged_entries = selected_entries.paginate([["_score", :desc]],
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
    # _options_ に指定可能な値は以下の通り。
    #
    # @param [::Hash] options The name and value
    #   pairs. Omitted names are initialized as the default value.
    # @option options [Integer] :size (10) The size
    #
    #   1ページあたりに表示する最大項目数。
    # @option options [Integer] :page (1) The page
    #
    #   ページ番号。ページ番号は0ベースではなく1ベースであることに注意。
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
  # ページ番号やレコードが何番目かは0ベースではなく1ベースで
  # あることに注意すること。
  module Pagination
    # 現在のページ番号。
    attr_reader :current_page
    # 1ページあたりのレコード数。
    attr_reader :page_size
    # 全ページ数。
    attr_reader :n_pages
    # 全レコード数。
    attr_reader :n_records

    # 2ページ以上ある場合は +true+ を返す。
    def have_pages?
      @n_pages > 1
    end

    # 最初のページ番号。常に1を返す。
    def first_page
      1
    end

    # 現在のページが最初のページなら +true+ を返す。
    def first_page?
      @current_page == first_page
    end

    # 最後のページ番号。
    def last_page
      @n_pages
    end

    # 現在のページが最後のページなら +true+ を返す。
    def last_page?
      @current_page == last_page
    end

    # 次のページがあるなら +true+ を返す。
    def have_next_page?
      @current_page < @n_pages
    end

    # 次のページ番号を返す。次のページがない場合は +nil+
    # を返す。
    def next_page
      have_next_page? ? @current_page + 1 : nil
    end

    # 前のページがあるなら +true+ を返す。
    def have_previous_page?
      @current_page > 1
    end

    # 前のページ番号を返す。前のページがない場合は +nil+
    # を返す。
    def previous_page
      have_previous_page? ? @current_page - 1 : nil
    end

    # 1ページあたりのレコード数を返す。
    def n_records_in_page
      size
    end

    # 現在のページに含まれているレコードのうち、先頭のレコー
    # ドが何番目のレコードかを返す。0ベースではなく1ベースで
    # あることに注意。つまり、最初のレコードは0番目のレコー
    # ドではなく、1番目のレコードになる。
    #
    # レコードが1つもない場合は +nil+ を返す。
    def start_offset
      return nil if @n_records.zero?
      1 + (@current_page - 1) * @page_size
    end

    # 現在のページに含まれているレコードのうち、最後のレコー
    # ドが何番目のレコードかを返す。0ベースではなく1ベースで
    # あることに注意。つまり、最初のレコードは0番目のレコー
    # ドではなく、1番目のレコードになる。
    #
    # レコードが1つもない場合は +nil+ を返す。
    def end_offset
      return nil if @n_records.zero?
      [start_offset + @page_size - 1, @n_records].min
    end

    # 最初のページから最後のページまでを含んだRangeを返す。
    #
    # 例えば、10ページある場合は以下を返す。
    #   1..10
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
