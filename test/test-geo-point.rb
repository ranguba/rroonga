# Copyright (C) 2012  Kouhei Sutou <kou@clear-code.com>
# Copyright (C) 2014  Masafumi Yokoyama <myokoym@gmail.com>
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
    class NewTest < self
      class StringTest < self
        class UnitTest < self
          def test_msec
            geo_point = Groonga::GeoPoint.new("128452975x503157902")
            assert_equal("128452975x503157902", geo_point.to_s)
          end

          def test_degree
            geo_point = Groonga::GeoPoint.new("35.6813819x139.7660839")
            assert_equal("35.6813819x139.7660839", geo_point.to_s)
          end
        end

        class SeparatorTest < self
          def test_x
            geo_point = Groonga::GeoPoint.new("128452975x503157902")
            assert_equal("128452975x503157902", geo_point.to_s)
          end

          def test_comma
            geo_point = Groonga::GeoPoint.new("128452975,503157902")
            assert_equal("128452975x503157902", geo_point.to_s)
          end
        end
      end

      def test_same_geodetic_system
        geo_point_base = Groonga::TokyoGeoPoint.new("128452975x503157902")
        geo_point = Groonga::TokyoGeoPoint.new(geo_point_base)
        assert_equal("128452975x503157902", geo_point.to_s)
      end

      def test_different_geodetic_system
        geo_point_base = Groonga::WGS84GeoPoint.new(35.6813819, 139.7660839)
        geo_point = Groonga::TokyoGeoPoint.new(geo_point_base)
        assert_equal("128441358x503169456", geo_point.to_msec.to_s)
      end
    end

    class MsecTest < self
      def test_to_s
        geo_point = Groonga::GeoPoint.new(128452975, 503157902)
        assert_equal("128452975x503157902", geo_point.to_s)
      end

      def test_to_msec
        geo_point = Groonga::GeoPoint.new(128452975, 503157902)
        assert_equal("128452975x503157902", geo_point.to_msec.to_s)
      end

      def test_to_degree
        geo_point = Groonga::GeoPoint.new(128452975, 503157902)
        assert_equal("35.6813819x139.7660839", geo_point.to_degree.to_s)
      end

      def test_msec?
        geo_point = Groonga::GeoPoint.new(128452975, 503157902)
        assert_true(geo_point.msec?)
      end

      def test_degree?
        geo_point = Groonga::GeoPoint.new(128452975, 503157902)
        assert_false(geo_point.degree?)
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

      def test_to_degree
        geo_point = Groonga::GeoPoint.new(35.6813819, 139.7660839)
        assert_equal("35.6813819x139.7660839", geo_point.to_degree.to_s)
      end

      def test_msec?
        geo_point = Groonga::GeoPoint.new(35.6813819, 139.7660839)
        assert_false(geo_point.msec?)
      end

      def test_degree?
        geo_point = Groonga::GeoPoint.new(35.6813819, 139.7660839)
        assert_true(geo_point.degree?)
      end
    end

    class MixTest < self
      def test_to_s
        geo_point = Groonga::GeoPoint.new(128452975, 139.7660839)
        assert_equal("128452975x139.7660839", geo_point.to_s)
      end

      def test_to_msec
        geo_point = Groonga::GeoPoint.new(128452975, 139.7660839)
        assert_equal("128452975x503157902", geo_point.to_msec.to_s)
      end

      def test_to_degree
        geo_point = Groonga::GeoPoint.new(128452975, 139.7660839)
        assert_equal("35.6813819x139.7660839", geo_point.to_degree.to_s)
      end

      def test_msec?
        geo_point = Groonga::GeoPoint.new(128452975, 139.7660839)
        assert_false(geo_point.msec?)
      end

      def test_degree?
        geo_point = Groonga::GeoPoint.new(128452975, 139.7660839)
        assert_false(geo_point.degree?)
      end
    end

    class EqualTest < self
      class SameGeodeticSystemTest < self
        def test_same_unit
          geo_point1 = Groonga::TokyoGeoPoint.new(35.6813819, 139.7660839)
          geo_point2 = Groonga::TokyoGeoPoint.new(35.6813819, 139.7660839)
          assert_equal(geo_point1, geo_point2)
        end

        def test_different_unit
          geo_point1 = Groonga::TokyoGeoPoint.new(35.6813819, 139.7660839)
          geo_point2 = Groonga::TokyoGeoPoint.new(128452975, 503157902)
          assert_equal(geo_point1, geo_point2)
        end
      end

      class DifferentGeodeticSystemTest < self
        def test_same_unit
          geo_point1 = Groonga::TokyoGeoPoint.new(35.6813819, 139.7660839)
          geo_point2 = Groonga::WGS84GeoPoint.new(35.6846084, 139.7628746)
          assert_equal(geo_point1, geo_point2)
        end

        def test_different_unit
          geo_point1 = Groonga::TokyoGeoPoint.new(35.6813819, 139.7660839)
          geo_point2 = Groonga::WGS84GeoPoint.new(128464590, 503146349)
          assert_equal(geo_point1, geo_point2)
        end
      end

      class StringTest < self
        def test_degree
          geo_point1 = Groonga::WGS84GeoPoint.new(35.6846084, 139.7628746)
          geo_point2 = "35.6846084x139.7628746"
          assert_equal(geo_point1, geo_point2)
        end

        def test_msec
          geo_point1 = Groonga::WGS84GeoPoint.new(128464590, 503146349)
          geo_point2 = "128464590x503146349"
          assert_equal(geo_point1, geo_point2)
        end
      end
    end
  end
end
