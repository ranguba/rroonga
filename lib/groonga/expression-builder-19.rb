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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

module Groonga
  module ExpressionBuildable
    class ColumnValueExpressionBuilder
      def !=(other)
        NotEqualExpressionBuilder.new(self, normalize(other))
      end
    end

    # @private
    class NotEqualExpressionBuilder < BinaryExpressionBuilder
      def initialize(column_value_builder, value)
        super(Groonga::Operation::NOT_EQUAL, column_value_builder, value)
      end
    end
  end

  # @private
  class ColumnExpressionBuilder
    def !=(other)
      column_value_builder != other
    end
  end
end
