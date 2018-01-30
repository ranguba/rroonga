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
  class Column
    # @param [Groonga::Operator] operator (Groonga::Operator::MATCH)
    # @return [Array<Groonga::IndexColumn>] Indexes on `column` which can
    #    execute `operator`.
    # @since 1.0.9
    #
    # @deprecated since 6.0.0. Use {Groonga::Column#find_indexes} instead.
    def indexes(operator=nil)
      operator ||= Operator::MATCH
      find_indexes(:operator => operator).collect do |index|
        index.column
      end
    end
  end
end
