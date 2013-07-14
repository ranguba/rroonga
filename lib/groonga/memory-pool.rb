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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

module Groonga
  # @api private
  class MemoryPool
    def initialize
      @temporary_objects = {}
    end

    def register(object)
      return unless object.temporary?
      return unless object.is_a?(Groonga::Table)
      @temporary_objects[object] = true
    end

    def close
      @temporary_objects.each do |(object, _)|
        object.close unless object.closed?
      end
      @temporary_objects.clear
    end
  end
end
