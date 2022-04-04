# Copyright (C) 2009-2022  Sutou Kouhei <kou@clear-code.com>
# Copyright (C) 2015-2017  Masafumi Yokoyama <yokoyama@clear-code.com>
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

module RroongaBuild
  module RequiredGroongaVersion
    MAJOR = 12
    MINOR = 0
    MICRO = 2
    VERSION = [MAJOR, MINOR, MICRO]
    STRING = VERSION.join(".")
    RELEASED_DATE = Time.utc(2022, 3, 29)
  end

  module_function
  def local_groonga_base_dir
    File.join(File.dirname(__FILE__), "vendor")
  end

  def local_groonga_install_dir
    File.expand_path(File.join(local_groonga_base_dir, "local"))
  end

  def have_local_groonga?(package_name, major, minor, micro)
    return false unless File.exist?(File.join(local_groonga_install_dir, "lib"))

    prepend_pkg_config_path_for_local_groonga
    PKGConfig.have_package(package_name, major, minor, micro)
  end

  def prepend_pkg_config_path_for_local_groonga
    pkg_config_dir = File.join(local_groonga_install_dir, "lib", "pkgconfig")
    PKGConfig.add_path(pkg_config_dir)
  end

  def add_rpath_for_local_groonga
    lib_dir = File.join(local_groonga_install_dir, "lib")
    original_LDFLAGS = $LDFLAGS
    checking_for(checking_message("-Wl,-rpath is available")) do
      $LDFLAGS += " -Wl,-rpath,#{Shellwords.escape(lib_dir)}"
      available = try_compile("int main() {return 0;}")
      $LDFLAGS = original_LDFLAGS unless available
      available
    end
  end
end
