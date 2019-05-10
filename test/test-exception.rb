# Copyright (C) 2009-2014  Kouhei Sutou <kou@clear-code.com>
# Copyright (C) 2014-2016  Masafumi Yokoyama <yokoyama@clear-code.com>
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

class ExceptionTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_defined
    assert_not_const_defined(Groonga, :Success)

    assert_const_defined(Groonga, :Error)
    assert_const_defined(Groonga, :ObjectClosed)

    assert_const_defined(Groonga, :EndOfData)
    assert_const_defined(Groonga, :UnknownError)
    assert_const_defined(Groonga, :OperationNotPermitted)
    assert_const_defined(Groonga, :NoSuchFileOrDirectory)
    assert_const_defined(Groonga, :NoSuchProcess)
    assert_const_defined(Groonga, :InterruptedFunctionCall)
    assert_const_defined(Groonga, :InputOutputError)
    assert_const_defined(Groonga, :NoSuchDeviceOrAddress)
    assert_const_defined(Groonga, :ArgumentListTooLong)
    assert_const_defined(Groonga, :ExecFormatError)
    assert_const_defined(Groonga, :BadFileDescriptor)
    assert_const_defined(Groonga, :NoChildProcesses)
    assert_const_defined(Groonga, :ResourceTemporarilyUnavailable)
    assert_const_defined(Groonga, :NotEnoughSpace)
    assert_const_defined(Groonga, :PermissionDenied)
    assert_const_defined(Groonga, :BadAddress)
    assert_const_defined(Groonga, :ResourceBusy)
    assert_const_defined(Groonga, :FileExists)
    assert_const_defined(Groonga, :ImproperLink)
    assert_const_defined(Groonga, :NoSuchDevice)
    assert_const_defined(Groonga, :NotADirectory)
    assert_const_defined(Groonga, :IsADirectory)
    assert_const_defined(Groonga, :InvalidArgument)
    assert_const_defined(Groonga, :TooManyOpenFilesInSystem)
    assert_const_defined(Groonga, :TooManyOpenFiles)
    assert_const_defined(Groonga, :InappropriateIOControlOperation)
    assert_const_defined(Groonga, :FileTooLarge)
    assert_const_defined(Groonga, :NoSpaceLeftOnDevice)
    assert_const_defined(Groonga, :InvalidSeek)
    assert_const_defined(Groonga, :ReadOnlyFileSystem)
    assert_const_defined(Groonga, :TooManyLinks)
    assert_const_defined(Groonga, :BrokenPipe)
    assert_const_defined(Groonga, :DomainError)
    assert_const_defined(Groonga, :ResultTooLarge)
    assert_const_defined(Groonga, :ResourceDeadlockAvoided)
    assert_const_defined(Groonga, :NoMemoryAvailable)
    assert_const_defined(Groonga, :FilenameTooLong)
    assert_const_defined(Groonga, :NoLocksAvailable)
    assert_const_defined(Groonga, :FunctionNotImplemented)
    assert_const_defined(Groonga, :DirectoryNotEmpty)
    assert_const_defined(Groonga, :IllegalByteSequence)
    assert_const_defined(Groonga, :SocketNotInitialized)
    assert_const_defined(Groonga, :OperationWouldBlock)
    assert_const_defined(Groonga, :AddressIsNotAvailable)
    assert_const_defined(Groonga, :NetworkIsDown)
    assert_const_defined(Groonga, :NoBuffer)
    assert_const_defined(Groonga, :SocketIsAlreadyConnected)
    assert_const_defined(Groonga, :SocketIsNotConnected)
    assert_const_defined(Groonga, :SocketIsAlreadyShutdowned)
    assert_const_defined(Groonga, :OperationTimeout)
    assert_const_defined(Groonga, :ConnectionRefused)
    assert_const_defined(Groonga, :RangeError)
    assert_const_defined(Groonga, :TokenizerError)
    assert_const_defined(Groonga, :FileCorrupt)
    assert_const_defined(Groonga, :InvalidFormat)
    assert_const_defined(Groonga, :ObjectCorrupt)
    assert_const_defined(Groonga, :TooManySymbolicLinks)
    assert_const_defined(Groonga, :NotSocket)
    assert_const_defined(Groonga, :OperationNotSupported)
    assert_const_defined(Groonga, :AddressIsInUse)
    assert_const_defined(Groonga, :ZLibError)
    assert_const_defined(Groonga, :LZ4Error)
    assert_const_defined(Groonga, :StackOverFlow)
    assert_const_defined(Groonga, :SyntaxError)
    assert_const_defined(Groonga, :RetryMax)
    assert_const_defined(Groonga, :IncompatibleFileFormat)
    assert_const_defined(Groonga, :UpdateNotAllowed)
    assert_const_defined(Groonga, :TooSmallOffset)
    assert_const_defined(Groonga, :TooLargeOffset)
    assert_const_defined(Groonga, :TooSmallLimit)
    assert_const_defined(Groonga, :CASError)
    assert_const_defined(Groonga, :UnsupportedCommandVersion)
    assert_const_defined(Groonga, :NormalizerError)
    assert_const_defined(Groonga, :TokenFilterError)
    assert_const_defined(Groonga, :CommandError)
    assert_const_defined(Groonga, :PluginError)
    assert_const_defined(Groonga, :ScorerError)
    assert_const_defined(Groonga, :ZstdError)
  end
end

class TooManyOpenFilesTest < Test::Unit::TestCase
  include GroongaTestUtils

  def setup
    @sub_context = nil
    unless Process.const_defined?(:RLIMIT_NOFILE)
      omit("No Process::RLIMIT_NOFILE")
    end
    setup_database
    @sub_context = create_sub_context
  end

  def teardown
    return if @sub_context.nil?
    @sub_context.database.close
    @sub_context.close
  end

  def test_database_each
    setup_users

    assert_error do
      over_limit do
        @sub_context.database.each do
        end
      end
    end
  end

  def test_context_array_reference
    setup_users

    assert_error do
      over_limit do
        @sub_context["Users"]
      end
    end
  end

  def test_table_columns
    setup_schema do |schema|
      schema.create_table("Users") do |table|
        table.short_text("name")
      end
    end

    table = @sub_context["Users"]
    assert_error do
      over_limit do
        table.columns
      end
    end
  end

  def test_column_sources
    setup_schema do |schema|
      schema.create_table("Users") do |table|
        table.short_text("name")
      end
      schema.create_table("Bookmarks") do |table|
        table.reference("user", "Users")
      end
      schema.create_table("Users") do |table|
        table.index("Bookmarks.user")
      end
    end

    column = @sub_context["Users"].column("Bookmarks_user")
    assert_error do
      over_limit do
        column.sources
      end
    end
  end

  def test_string_key_type
    setup_users

    assert_error do
      over_limit do
        create_reference_table("Users")
      end
    end
  end

  def test_id_key_type
    setup_users
    users_id = Groonga["Users"].id

    assert_error do
      over_limit do
        create_reference_table(users_id)
      end
    end
  end

  private
  def setup_schema
    Groonga::Schema.define do |schema|
      yield(schema)
    end
  end

  def setup_users
    setup_schema do |schema|
      schema.create_table("Users")
    end
  end

  def create_sub_context
    context = Groonga::Context.new
    context.open_database(Groonga::Context.default.database.path)
    context
  end

  def over_limit
    unavailable = 0
    original, max = Process.getrlimit(open_files)

    Process.setrlimit(open_files, unavailable, max)
    begin
      yield
    ensure
      Process.setrlimit(open_files, original, max)
    end
  end

  def open_files
    Process::RLIMIT_NOFILE
  end

  def assert_error
    assert_raise(Groonga::TooManyOpenFiles) do
      yield
    end
  end

  def create_reference_table(key)
    Groonga::Hash.create(:context => @sub_context,
                         :name => "Bookmarks",
                         :key_type => key)
  end
end
