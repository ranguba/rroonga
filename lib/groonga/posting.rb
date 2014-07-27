# -*- coding: utf-8 -*-
#
# Copyright (C) 2011-2012  Kouhei Sutou <kou@clear-code.com>
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

require 'English'

module Groonga
  # This class keeps posting information.
  #
  # @since 1.2.1
  class Posting
    # The record ID.
    attr_accessor :record_id

    # The section ID.
    attr_accessor :section_id

    # The term ID.
    attr_accessor :term_id

    # The position.
    attr_accessor :position

    # The term frequency.
    attr_accessor :term_frequency

    # The weight.
    attr_accessor :weight

    # The number of rest posting information for the term ID.
    attr_accessor :n_rest_postings

    # @return [Groonga::Table] The table of the record ID.
    attr_reader :table

    # @return [Groonga::Table] The table of the term ID.
    attr_reader :lexicon

    # Creates a new Posting.
    #
    # @return The new Posting.
    def initialize(parameters=nil)
      update(parameters || {})
    end

    # Updates all values.
    #
    # @param [::Hash] parameters The name and value
    #   pairs. Omitted names are initialized as the default value.
    # @option parameters [Integer] :record_id The record_id.
    # @option parameters [Integer] :section_id The section_id.
    # @option parameters [Integer] :term_id The term_id.
    # @option parameters [Integer] :position The position.
    # @option parameters [Integer] :term_frequency The term_frequency.
    # @option parameters [Integer] :weight The weight.
    # @option parameters [Integer] :n_rest_postings The n_rest_postings.
    # @option parameters [Groonga::Table] :table The table of the record ID.
    # @option parameters [Groonga::Table] :lexicon The table of the term ID.
    def update(parameters)
      @record_id = parameters[:record_id] || nil
      @section_id = parameters[:section_id] || nil
      @term_id = parameters[:term_id] || nil
      @position = parameters[:position] || 0
      @term_frequency = parameters[:term_frequency] || 0
      @weight = parameters[:weight] || 0
      @n_rest_postings = parameters[:n_rest_postings] || 0
      @table = parameters[:table]
      @lexicon = parameters[:lexicon]
    end

    # Returns Hash created from attributes.
    def to_hash
      {
        :record_id => @record_id,
        :section_id => @section_id,
        :term_id => @term_id,
        :position => @position,
        :term_frequency => @term_frequency,
        :weight => @weight,
        :n_rest_postings => @n_rest_postings
      }
    end

    # @return [Groonga::Record, nil] The record for the record ID.
    #   If table isn't assosiated, nil is returned.
    #
    # @since 2.0.6
    def record
      return nil unless @table
      Record.new(@table, @record_id)
    end

    # @return [Groonga::Record, nil] The record for the term ID.
    #   If lexicon isn't assosiated, nil is returned.
    #
    # @since 2.0.6
    def term
      return nil unless @lexicon
      Record.new(@lexicon, @term_id)
    end
  end
end
