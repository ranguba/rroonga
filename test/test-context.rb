# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
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

class ContextTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_default
    context = Groonga::Context.default
    assert_not_predicate(context, :use_ql?)
    assert_not_predicate(context, :batch_mode?)
    assert_equal(Groonga::Encoding.default, context.encoding)
  end

  def test_default_options
    Groonga::Context.default_options = {
      :use_ql => true,
      :batch_mode => true,
      :encoding => :utf8,
    }
    context = Groonga::Context.default
    assert_predicate(context, :use_ql?)
    assert_predicate(context, :batch_mode?)
    assert_equal(Groonga::Encoding::UTF8, context.encoding)
  end

  def test_use_ql?
    context = Groonga::Context.new
    assert_not_predicate(context, :use_ql?)

    context = Groonga::Context.new(:use_ql => true)
    assert_predicate(context, :use_ql?)
  end

  def test_batch_mode?
    context = Groonga::Context.new
    assert_not_predicate(context, :batch_mode?)

    context = Groonga::Context.new(:batch_mode => true)
    assert_predicate(context, :batch_mode?)
  end

  def test_encoding
    context = Groonga::Context.new
    assert_equal(Groonga::Encoding.default, context.encoding)

    context = Groonga::Context.new(:encoding => :utf8)
    assert_equal(Groonga::Encoding::UTF8, context.encoding)
  end

  def test_inspect
    context = Groonga::Context.new(:use_ql => true,
                                   :batch_mode => true,
                                   :encoding => Groonga::Encoding::UTF8)
    assert_equal("#<Groonga::Context " +
                 "use_ql: <true>, " +
                 "batch_mode: <true>, " +
                 "encoding: <:utf8>, " +
                 "database: <nil>>",
                 context.inspect)
  end

  def test_inspect_with_database
    db = Groonga::Database.create
    context = Groonga::Context.default
    assert_equal("#<Groonga::Context " +
                 "use_ql: <false>, " +
                 "batch_mode: <false>, " +
                 "encoding: <#{Groonga::Encoding.default.inspect}>, " +
                 "database: <#{db.inspect}>>",
                 context.inspect)
  end

  def test_array_reference
    Groonga::Database.create
    context = Groonga::Context.default
    assert_equal("<int>", context["<int>"].name)
  end
end
