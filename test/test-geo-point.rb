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

class GeoPointTest < Test::Unit::TestCase
  include GroongaTestUtils

  class ValueConverterTest < self
    def test_msec_to_degree
      assert_equal(35.6813819, msec_to_degree(128452975))
    end

    def test_degree_to_msec
      assert_equal(128452975, degree_to_msec(35.6813819))
    end

    private
    def msec_to_degree(msec)
      Groonga::GeoPointValueConverter.msec_to_degree(msec)
    end

    def degree_to_msec(msec)
      Groonga::GeoPointValueConverter.degree_to_msec(msec)
    end
  end

  class BaseTest < self
    class MsecTest < self
      def test_to_s
        geo_point = Groonga::GeoPoint.new(128452975, 503157902)
        assert_equal("128452975x503157902", geo_point.to_s)
      end

      def test_to_degree
        geo_point = Groonga::GeoPoint.new(128452975, 503157902)
        assert_equal("35.6813819x139.7660839", geo_point.to_degree.to_s)
      end
    end

    class DegreeTest < self
      def test_to_s
        geo_point = Groonga::GeoPoint.new(35.6813819, 139.7660839)
        assert_equal("35.6813819x139.7660839", geo_point.to_s)
      end

      def test_to_msec
        geo_point = Groonga::GeoPoint.new(35.6813819, 139.7660839)
        assert_equal("128452975x503157902", geo_point.to_msec.to_s)
      end
    end
  end
end
