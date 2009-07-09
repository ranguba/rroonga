# -*- coding: utf-8 -*-
#
# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
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
  class ExpressionBuilder
    def initialize(table)
      @table = table
    end

    def build
      @expression = Expression.new
      @variable = @expression.define_variable
      @variable.value = Groonga::Record.new(@table, 0)
      @expression.append_object(@variable)
      yield(self)
      @expression.compile
      @expression
    end

    def record
      Record.new(self, @expression, @table, @variable)
    end

    def &(other)
      @expression.append_operation(Groonga::Operation::AND, 2)
      self
    end

    class Record
      def initialize(builder, expression, table, variable)
        @builder = builder
        @expression = expression
        @table = table
        @variable = variable
      end

      def [](name)
        Column.new(@builder, @expression, @table.column(name), @variable)
      end
    end

    class Column
      def initialize(builder, expression, column, variable)
        @builder = builder
        @expression = expression
        @column = column
        @variable = variable
      end

      def ==(other)
        @expression.append_object(@variable)
        @expression.append_constant(@column.local_name)
        @expression.append_operation(Groonga::Operation::OBJECT_GET_VALUE, 2)
        @expression.append_constant(other)
        @expression.append_operation(Groonga::Operation::EQUAL, 2)
        @builder
      end

      def <(other)
        @expression.append_object(@variable)
        @expression.append_constant(@column.local_name)
        @expression.append_operation(Groonga::Operation::OBJECT_GET_VALUE, 2)
        @expression.append_constant(other)
        @expression.append_operation(Groonga::Operation::LESS, 2)
        @builder
      end

      def <=(other)
        @expression.append_object(@variable)
        @expression.append_constant(@column.local_name)
        @expression.append_operation(Groonga::Operation::OBJECT_GET_VALUE, 2)
        @expression.append_constant(other)
        @expression.append_operation(Groonga::Operation::LESS_EQUAL, 2)
        @builder
      end

      def >(other)
        @expression.append_object(@variable)
        @expression.append_constant(@column.local_name)
        @expression.append_operation(Groonga::Operation::OBJECT_GET_VALUE, 2)
        @expression.append_constant(other)
        @expression.append_operation(Groonga::Operation::GREATER, 2)
        @builder
      end

      def >=(other)
        @expression.append_object(@variable)
        @expression.append_constant(@column.local_name)
        @expression.append_operation(Groonga::Operation::OBJECT_GET_VALUE, 2)
        @expression.append_constant(other)
        @expression.append_operation(Groonga::Operation::GREATER_EQUAL, 2)
        @builder
      end
    end
  end
end
