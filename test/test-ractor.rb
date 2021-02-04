# Copyright (C) 2021  Sutou Kouhei <kou@clear-code.com>
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

class RactorTest < Test::Unit::TestCase
  include GroongaTestUtils

  setup
  def need_ractor
    omit("Ractor is needed") unless defined?(Ractor)
  end

  setup :setup_database

  setup
  def setup_tables
    Groonga::Schema.define do |schema|
      schema.create_table("Comments") do |table|
        table.text("content")
      end

      schema.create_table("Terms",
                          type: :patricia_trie,
                          default_tokenizer: "TokenNgram",
                          normalizer: "NormalizerNFKC130") do |table|
        table.index("Comments.content", with_position: true)
      end
    end
  end

  setup
  def setup_records
    comments = Groonga["Comments"]
    comments.add(content: "Hello World")
    comments.add(content: "Groonga is fast!")
    comments.add(content: "Rroonga is the Groonga bindings")
  end

  test "select" do
    ractor = Ractor.new(@database_path) do |database_path|
      Groonga::Context.open(encoding: nil) do |context|
        context.open_database(database_path) do
          comments = context["Comments"]
          matched_comments = comments.select do |comment|
            comment.content.match("Groonga")
          end
          matched_comments.collect(&:content)
        end
      end
    end
    assert_equal(["Groonga is fast!", "Rroonga is the Groonga bindings"],
                 ractor.take)
  end
end
