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
  # Measures statistic.
  class StatisticMeasurer
    MAX_N_ADDITIONAL_PATHS = 4096

    # @param path [String, nil] Measures disk usage of the path.
    # @return [Integer] 0 if path is @nil@, disk usage of the path otherwise.
    def measure_disk_usage(path)
      return 0 if path.nil?

      usage = File.size(path)
      1.step(MAX_N_ADDITIONAL_PATHS) do |i|
        additional_path = "%s.%03X" % [path, i]
        break unless File.exist?(additional_path)
        usage += File.size(additional_path)
      end
      usage
    end
  end
end
