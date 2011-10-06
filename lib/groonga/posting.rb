# -*- coding: utf-8 -*-
#
# Copyright (C) 2011  Kouhei Sutou <kou@clear-code.com>
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
    def update(parameters)
      @record_id = parameters[:record_id] || nil
      @section_id = parameters[:section_id] || nil
      @term_id = parameters[:term_id] || nil
      @position = parameters[:position] || 0
      @term_frequency = parameters[:term_frequency] || 0
      @weight = parameters[:weight] || 0
      @n_rest_postings = parameters[:n_rest_postings] || 0
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
  end
end
