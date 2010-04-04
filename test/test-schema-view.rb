# Copyright (C) 2010  Kouhei Sutou <kou@clear-code.com>
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

class SchemaViewTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup :setup_database

  def test_create_view
    assert_nil(context["Entries"])
    Groonga::Schema.create_view("Entries") do |view|
    end
    assert_not_nil(context["Entries"])
  end

  def test_create_view_force
    Groonga::Schema.create_table("People") do |table|
      table.string("name")
    end
    context["People"].add(:name => "morita")

    Groonga::Schema.create_view("Entries") do |view|
      view.add("People")
    end
    assert_equal(["morita"],
                 context["Entries"].collect {|entry| entry["name"]})

    Groonga::Schema.create_view("People") do |view|
    end
    assert_equal(["morita"],
                 context["Entries"].collect {|entry| entry["name"]})

    Groonga::Schema.create_view("Entries", :force => true) do |view|
    end
    assert_equal([],
                 context["Entries"].collect {|entry| entry["name"]})
  end

  def test_remove_view
    Groonga::View.create(:name => "Entries")
    assert_not_nil(context["Entries"])
    Groonga::Schema.remove_view("Entries")
    assert_nil(context["Entries"])
  end

  def test_add
    Groonga::Schema.create_table("People") do |table|
      table.string("name")
    end
    context["People"].add(:name => "morita")

    Groonga::Schema.create_table("Dogs") do |table|
      table.string("name")
    end
    context["Dogs"].add(:name => "pochi")

    Groonga::Schema.create_view("Entries") do |view|
      view.add("People")
      view.add("Dogs")
    end
    assert_equal(["morita", "pochi"],
                 context["Entries"].collect {|entry| entry["name"]})
  end

  def test_define_lazy
    assert_nothing_raised do
      Groonga::Schema.define do |schema|
        schema.create_table("People") do |table|
          table.string("name")
        end

        schema.create_view("Entries") do |view|
          view.add("People")
        end
      end
    end
  end
end
