# -*- coding: utf-8 -*-
#
# Copyright (C) 2012  Kouhei Sutou <kou@clear-code.com>
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
  module GeoPointValueConverter
    module_function

    MSEC_PER_SEC = 1000 * 1000
    N_SIGNIFICANT_DIGITS = (Math.log10(MSEC_PER_SEC) + 1).truncate
    def msec_to_degree(msec)
      degree_integer_part, degree_fraction_part = msec.divmod(3600 * 1000)
      degree = degree_integer_part + (degree_fraction_part.to_f / (3600 * 1000))
      degree.round(N_SIGNIFICANT_DIGITS)
    end

    def degree_to_msec(degree)
      (degree * 3600 * 1000).round
    end

    def msec?(value)
      value.is_a?(Integer)
    end

    def degree?(value)
      value.is_a?(Float)
    end

    def to_degree(value)
      return value if degree?(value)
      msec_to_degree(value)
    end

    def to_msec(value)
      return value if msec?(value)
      degree_to_msec(value)
    end
  end
end
