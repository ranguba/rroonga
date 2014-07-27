# -*- coding: utf-8 -*-
#
# Copyright (C) 2013  Kouhei Sutou <kou@clear-code.com>
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

module Groonga
  # Represents sub records of a {Record}. Grouped result set by
  # {Table#group} only has sub records.
  #
  # {SubRecords} acts like an ::Array.
  class SubRecords
    include Enumerable

    # The record that has this sub records.
    attr_reader :record

    # Creates a sub records container for the _record_.
    #
    # Normally, users don't need to instantiate {SubRecords}
    # directly. {Record#sub_records} creates and returns a
    # {SubRecords}.
    #
    # @param record [Record] The record that has this sub records.
    def initialize(record)
      @record = record
    end

    # @yield [record] Gives a sub record to the block.
    # @yieldparam record [Record] A sub record.
    # @return [void]
    def each(&block)
      @record.table.each_sub_record(@record.record_raw_id, &block)
    end

    # @return [Array<Record>] Sub records as ::Array.
    def to_a
      @sub_records ||= super
    end

    # Acts as ::Array
    alias_method :to_ary, :to_a

    # @param index [Integer] A 0-origin index.
    # @return [Record] A sub record at _index_.
    def [](index)
      to_a[index]
    end
  end
end

