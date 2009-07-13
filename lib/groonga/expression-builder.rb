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
  module ExpressionBuildable
    attr_accessor :builder, :expression, :variable
    def initialize(*args)
      @builder = self
      @expression = nil
      @variable = nil
      @table = nil
      @name = nil
      @query = nil
      @default_column = nil
    end

    def build
      @expression = Expression.new(:table => @table,
                                   :name => @name,
                                   :query => @query,
                                   :default_column => @default_column)
      if block_given?
        @variable = @expression.define_variable(:domain => @table)
        @expression.append_object(@variable)
        yield(self)
        @expression.compile
      end
      @expression
    end
  end

  class RecordExpressionBuilder
    include ExpressionBuildable

    def initialize(table, name)
      super
      @table = table
      @name = name
    end

    def [](name)
      column = @table.column(name)
      if column.nil?
        message = "unknown column <#{name.inspect}> " +
          "for table <#{@table.inspect}>"
        raise ArgumentError, message
      end
      builder = ColumnExpressionBuilder.new(column, nil, nil)
      builder.builder = @builder
      builder.expression = @expression
      builder.variable = @variable
      builder
    end

    def &(other)
      @expression.append_operation(Groonga::Operation::AND, 2)
      @builder
    end
  end

  class ColumnExpressionBuilder
    include ExpressionBuildable

    def initialize(column, name, query)
      super
      @table = column.table
      @column = column
      @default_column = column.local_name
      @name = name
      @query = query
    end

    def ==(other)
      @expression.append_object(@variable)
      @expression.append_constant(@column.local_name)
      @expression.append_operation(Groonga::Operation::OBJECT_GET_VALUE, 2)
      @expression.append_constant(other)
      @expression.append_operation(Groonga::Operation::EQUAL, 2)
      @builder
    end

    def =~(other)
      @expression.append_object(@variable)
      @expression.append_constant(@column.local_name)
      @expression.append_operation(Groonga::Operation::OBJECT_GET_VALUE, 2)
      @expression.append_constant(other)
      @expression.append_operation(Groonga::Operation::MATCH, 2)
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
