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

  class GeoPoint
    attr_accessor :latitude, :longitude
    def initialize(latitude, longitude)
      @latitude = latitude
      @longitude = longitude

    def degree?
      GeoPointValueConverter.degree?(latitude) and
        GeoPointValueConverter.degree?(longitude)
    end

    def msec?
      GeoPointValueConverter.msec?(latitude) and
        GeoPointValueConverter.msec?(longitude)
    end

    def to_degree
      return self if degree?
      self.class.new(GeoPointValueConverter.to_degree(latitude),
                     GeoPointValueConverter.to_degree(longitude))
    end

    def to_msec
      return self if msec?
      self.class.new(GeoPointValueConverter.to_msec(latitude),
                     GeoPointValueConverter.to_msec(longitude))
    end

    def to_s
      "#{latitude}x#{longitude}"
    end

    def inspect
      "#<#{self.class} #{to_s}>"
    end

    def ==(other)
      case other
      when String
        to_s == otehr
      when GeoPoint
        normalized_self = to_msec
        normalized_other = coerce(other).to_msec
        [normalized_self.latitude, normalized_self.longitude] ==
          [normalized_other.latitude, normalized_other.longitude]
      else
        false
      end
    end
  end

  class TokyoGeoPoint < GeoPoint
    def to_tokyo
      self
    end

    # TODO: write document
    #
    # TokyoGeoPoint <-> WGS84GeoPoint is based on
    # http://www.jalan.net/jw/jwp0200/jww0203.do
    #
    #   jx: longitude in degree in Tokyo Geodetic System.
    #   jy: latitude in degree in Tokyo Geodetic System.
    #   wx: longitude in degree in WGS 84.
    #   wy: latitude in degree in WGS 84.
    #
    #   jy = wy * 1.000106961 - wx * 0.000017467 - 0.004602017
    #   jx = wx * 1.000083049 + wy * 0.000046047 - 0.010041046
    #
    #   wy = jy - jy * 0.00010695 + jx * 0.000017464 + 0.0046017
    #   wx = jx - jy * 0.000046038 - jx * 0.000083043 + 0.010040
    def to_wgs84
      in_degree = to_degree
      wgs84_latitude_in_degree =
        in_degree.latitude -
        in_degree.latitude * 0.00010695 +
        in_degree.longitude * 0.000017464 +
        0.0046017
      wgs84_longitude_in_degree =
        in_degree.longitude -
        in_degree.latitude * 0.000046038 -
        in_degree.longitude * 0.000083043 +
        0.010040
      WGS84GeoPoint.new(wgs84_latitude_in_degree, wgs84_longitude_in_degree)
    end

    private
    def coerce(other_geo_point)
      other_geo_point.to_tokyo
    end
  end

  class WGS84GeoPoint < GeoPoint
    # @see TokyoGeoPoint#to_wgs84
    def to_tokyo
      in_degree = to_degree
      tokyo_latitude_in_degree =
        in_degree.latitude * 1.000106961 -
        in_degree.longitude * 0.000017467 -
        0.004602017
      tokyo_longitude_in_degree =
        in_degree.longitude * 1.000083049 +
        in_degree.latitude  * 0.000046047 -
        0.010041046
      TokyoGeoPoint.new(tokyo_latitude_in_degree, tokyo_longitude_in_degree)
    end

    def to_wgs84
      self
    end

    private
    def coerce(other_geo_point)
      other_geo_point.to_wgs84
    end
  end
end
