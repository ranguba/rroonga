# Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

class ExceptionTest < Test::Unit::TestCase
  include GroongaTestUtils

  def test_defined
    assert_not_const_defined(Groonga, :Success)

    assert_const_defined(Groonga, :EndOfData)
    assert_const_defined(Groonga, :UnknownError)
    assert_const_defined(Groonga, :OperationNotPermitted)
    assert_const_defined(Groonga, :NoSuchFileOrDirectory)
    assert_const_defined(Groonga, :NoSuchProcess)
    assert_const_defined(Groonga, :InterruptedFunctionCall)
    assert_const_defined(Groonga, :InputOutputError)
    assert_const_defined(Groonga, :NoSuchDeviceOrAddress)
    assert_const_defined(Groonga, :ArgListTooLong)
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
    assert_const_defined(Groonga, :LZOError)
  end
end
