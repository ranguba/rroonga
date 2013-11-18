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

class StatisticMeasurerTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    @measurer = Groonga::StatisticMeasurer.new
  end

  class DiskUsageTest < self
    def test_nil
      assert_equal(0, disk_usage(nil))
    end

    def test_path_only
      size = 5
      path = File.join(@tmp_dir, "db")
      write(path, "X" * size)
      assert_equal(size, disk_usage(path))
    end

    def test_additional_path
      size_per_file = 5
      path = File.join(@tmp_dir, "db")
      write(path, "X" * size_per_file)
      write("#{path}.001", "X" * size_per_file)
      write("#{path}.002", "X" * size_per_file)
      assert_equal(3 * size_per_file, disk_usage(path))
    end

    private
    def disk_usage(path)
      @measurer.measure_disk_usage(path)
    end

    def write(path, content)
      File.open(path, "w") do |file|
        file.write(content)
      end
    end
  end
end
