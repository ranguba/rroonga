# Copyright (C) 2009-2012  Kouhei Sutou <kou@clear-code.com>
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

class VersionTest < Test::Unit::TestCase
  def test_build_version_format
    assert_equal("X.X.X", normalize(Groonga.build_version))
  end

  def test_build_version_value
    assert_equal(Groonga.build_version, Groonga::BUILD_VERSION.join("."))
  end

  def test_version_format
    if /-/ =~ Groonga.version
      assert_equal("X.X.X-XXX", normalize(Groonga.version))
    else
      assert_equal("X.X.X", normalize(Groonga.version))
    end
  end

  def test_version_value
    major, minor, micro, tag = Groonga::VERSION
    versions = [major, minor, micro]
    assert_equal(Groonga.version, [versions.join("."), tag].compact.join("-"))
  end

  def test_bindings_version_format
    assert_equal("X.X.X", normalize(Groonga.bindings_version))
  end

  def test_bindings_version_value
    assert_equal(Groonga.bindings_version, Groonga::BINDINGS_VERSION.join("."))
  end

  private
  def normalize(version)
    versions_component, tag_component = version.split(/-/, 2)
    versions = versions_component.split(/\./)
    normalized_versions = versions.collect do |component|
      component.gsub(/\d+/, "X")
    end
    if tag_component
      normalized_tag = "XXX"
    else
      normalized_tag = nil
    end
    [normalized_versions.join("."), normalized_tag].compact.join("-")
  end
end
