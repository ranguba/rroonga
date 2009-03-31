/* -*- c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-groonga-private.h"

static VALUE eGrnError;
static VALUE eGrnEndOfData;
static VALUE eGrnUnknownError;
static VALUE eGrnOperationNotPermitted;
static VALUE eGrnNoSuchFileOrDirectory;
static VALUE eGrnNoSuchProcess;
static VALUE eGrnInterruptedFunctionCall;
static VALUE eGrnInputOutputError;
static VALUE eGrnNoSuchDeviceOrAddress;
static VALUE eGrnArgListTooLong;
static VALUE eGrnExecFormatError;
static VALUE eGrnBadFileDescriptor;
static VALUE eGrnNoChildProcesses;
static VALUE eGrnResourceTemporarilyUnavailable;
static VALUE eGrnNotEnoughSpace;
static VALUE eGrnPermissionDenied;
static VALUE eGrnBadAddress;
static VALUE eGrnResourceBusy;
static VALUE eGrnFileExists;
static VALUE eGrnImproperLink;
static VALUE eGrnNoSuchDevice;
static VALUE eGrnNotADirectory;
static VALUE eGrnIsADirectory;
static VALUE eGrnInvalidArgument;
static VALUE eGrnTooManyOpenFilesInSystem;
static VALUE eGrnTooManyOpenFiles;
static VALUE eGrnInappropriateIOControlOperation;
static VALUE eGrnFileTooLarge;
static VALUE eGrnNoSpaceLeftOnDevice;
static VALUE eGrnInvalidSeek;
static VALUE eGrnReadOnlyFileSystem;
static VALUE eGrnTooManyLinks;
static VALUE eGrnBrokenPipe;
static VALUE eGrnDomainError;
static VALUE eGrnResultTooLarge;
static VALUE eGrnResourceDeadlockAvoided;
static VALUE eGrnNoMemoryAvailable;
static VALUE eGrnFilenameTooLong;
static VALUE eGrnNoLocksAvailable;
static VALUE eGrnFunctionNotImplemented;
static VALUE eGrnDirectoryNotEmpty;
static VALUE eGrnIllegalByteSequence;
static VALUE eGrnSocketNotInitialised;
static VALUE eGrnOperationWouldBlock;
static VALUE eGrnAddressIsNotAvailable;
static VALUE eGrnNetworkIsDown;
static VALUE eGrnNoBuffer;
static VALUE eGrnSocketIsAlreadyConnected;
static VALUE eGrnSocketIsNotConnected;
static VALUE eGrnSocketIsAlreadyShutdowned;
static VALUE eGrnOperationTimeout;
static VALUE eGrnConnectionRefused;
static VALUE eGrnRangeError;
static VALUE eGrnTokenizerError;
static VALUE eGrnFileCorrupt;
static VALUE eGrnInvalidFormat;
static VALUE eGrnObjectCorrupt;
static VALUE eGrnTooManySymbolicLinks;
static VALUE eGrnNotSocket;
static VALUE eGrnOperationNotSupported;
static VALUE eGrnAddressIsInUse;
static VALUE eGrnZLibError;
static VALUE eGrnLZOError;

void
rb_grn_check_rc (grn_rc rc)
{
    VALUE exception = Qnil;
    const char *message;

    switch (rc) {
    case GRN_SUCCESS:
        return;
        break;
    case GRN_END_OF_DATA:
        exception = eGrnEndOfData;
        message = "end of data";
        break;
    case GRN_UNKNOWN_ERROR:
        exception = eGrnUnknownError;
        message = "unknown error";
        break;
    case GRN_OPERATION_NOT_PERMITTED:
        exception = eGrnOperationNotPermitted;
        message = "operation not permitted";
        break;
    case GRN_NO_SUCH_FILE_OR_DIRECTORY:
        exception = eGrnNoSuchFileOrDirectory;
        message = "no such file or directory";
        break;
    case GRN_NO_SUCH_PROCESS:
        exception = eGrnNoSuchProcess;
        message = "no such process";
        break;
    case GRN_INTERRUPTED_FUNCTION_CALL:
        exception = eGrnInterruptedFunctionCall;
        message = "interrupted function call";
        break;
    case GRN_INPUT_OUTPUT_ERROR:
        exception = eGrnInputOutputError;
        message = "input output error";
        break;
    case GRN_NO_SUCH_DEVICE_OR_ADDRESS:
        exception = eGrnNoSuchDeviceOrAddress;
        message = "no such device or address";
        break;
    case GRN_ARG_LIST_TOO_LONG:
        exception = eGrnArgListTooLong;
        message = "arg list too long";
        break;
    case GRN_EXEC_FORMAT_ERROR:
        exception = eGrnExecFormatError;
        message = "exec format error";
        break;
    case GRN_BAD_FILE_DESCRIPTOR:
        exception = eGrnBadFileDescriptor;
        message = "bad file descriptor";
        break;
    case GRN_NO_CHILD_PROCESSES:
        exception = eGrnNoChildProcesses;
        message = "no child processes";
        break;
    case GRN_RESOURCE_TEMPORARILY_UNAVAILABLE:
        exception = eGrnResourceTemporarilyUnavailable;
        message = "resource temporarily unavailable";
        break;
    case GRN_NOT_ENOUGH_SPACE:
        exception = eGrnNotEnoughSpace;
        message = "not enough space";
        break;
    case GRN_PERMISSION_DENIED:
        exception = eGrnPermissionDenied;
        message = "permission denied";
        break;
    case GRN_BAD_ADDRESS:
        exception = eGrnBadAddress;
        message = "bad address";
        break;
    case GRN_RESOURCE_BUSY:
        exception = eGrnResourceBusy;
        message = "resource busy";
        break;
    case GRN_FILE_EXISTS:
        exception = eGrnFileExists;
        message = "file exists";
        break;
    case GRN_IMPROPER_LINK:
        exception = eGrnImproperLink;
        message = "improper link";
        break;
    case GRN_NO_SUCH_DEVICE:
        exception = eGrnNoSuchDevice;
        message = "no such device";
        break;
    case GRN_NOT_A_DIRECTORY:
        exception = eGrnNotADirectory;
        message = "not a directory";
        break;
    case GRN_IS_A_DIRECTORY:
        exception = eGrnIsADirectory;
        message = "is a directory";
        break;
    case GRN_INVALID_ARGUMENT:
        exception = eGrnInvalidArgument;
        message = "invalid argument";
        break;
    case GRN_TOO_MANY_OPEN_FILES_IN_SYSTEM:
        exception = eGrnTooManyOpenFilesInSystem;
        message = "too many open files in system";
        break;
    case GRN_TOO_MANY_OPEN_FILES:
        exception = eGrnTooManyOpenFiles;
        message = "too many open files";
        break;
    case GRN_INAPPROPRIATE_I_O_CONTROL_OPERATION:
        exception = eGrnInappropriateIOControlOperation;
        message = "inappropriate i o control operation";
        break;
    case GRN_FILE_TOO_LARGE:
        exception = eGrnFileTooLarge;
        message = "file too large";
        break;
    case GRN_NO_SPACE_LEFT_ON_DEVICE:
        exception = eGrnNoSpaceLeftOnDevice;
        message = "no space left on device";
        break;
    case GRN_INVALID_SEEK:
        exception = eGrnInvalidSeek;
        message = "invalid seek";
        break;
    case GRN_READ_ONLY_FILE_SYSTEM:
        exception = eGrnReadOnlyFileSystem;
        message = "read only file system";
        break;
    case GRN_TOO_MANY_LINKS:
        exception = eGrnTooManyLinks;
        message = "too many links";
        break;
    case GRN_BROKEN_PIPE:
        exception = eGrnBrokenPipe;
        message = "broken pipe";
        break;
    case GRN_DOMAIN_ERROR:
        exception = eGrnDomainError;
        message = "domain error";
        break;
    case GRN_RESULT_TOO_LARGE:
        exception = eGrnResultTooLarge;
        message = "result too large";
        break;
    case GRN_RESOURCE_DEADLOCK_AVOIDED:
        exception = eGrnResourceDeadlockAvoided;
        message = "resource deadlock avoided";
        break;
    case GRN_NO_MEMORY_AVAILABLE:
        exception = eGrnNoMemoryAvailable;
        message = "no memory available";
        break;
    case GRN_FILENAME_TOO_LONG:
        exception = eGrnFilenameTooLong;
        message = "filename too long";
        break;
    case GRN_NO_LOCKS_AVAILABLE:
        exception = eGrnNoLocksAvailable;
        message = "no locks available";
        break;
    case GRN_FUNCTION_NOT_IMPLEMENTED:
        exception = eGrnFunctionNotImplemented;
        message = "function not implemented";
        break;
    case GRN_DIRECTORY_NOT_EMPTY:
        exception = eGrnDirectoryNotEmpty;
        message = "directory not empty";
        break;
    case GRN_ILLEGAL_BYTE_SEQUENCE:
        exception = eGrnIllegalByteSequence;
        message = "illegal byte sequence";
        break;
    case GRN_SOCKET_NOT_INITIALISED:
        exception = eGrnSocketNotInitialised;
        message = "socket not initialised";
        break;
    case GRN_OPERATION_WOULD_BLOCK:
        exception = eGrnOperationWouldBlock;
        message = "operation would block";
        break;
    case GRN_ADDRESS_IS_NOT_AVAILABLE:
        exception = eGrnAddressIsNotAvailable;
        message = "address is not available";
        break;
    case GRN_NETWORK_IS_DOWN:
        exception = eGrnNetworkIsDown;
        message = "network is down";
        break;
    case GRN_NO_BUFFER:
        exception = eGrnNoBuffer;
        message = "no buffer";
        break;
    case GRN_SOCKET_IS_ALREADY_CONNECTED:
        exception = eGrnSocketIsAlreadyConnected;
        message = "socket is already connected";
        break;
    case GRN_SOCKET_IS_NOT_CONNECTED:
        exception = eGrnSocketIsNotConnected;
        message = "socket is not connected";
        break;
    case GRN_SOCKET_IS_ALREADY_SHUTDOWNED:
        exception = eGrnSocketIsAlreadyShutdowned;
        message = "socket is already shutdowned";
        break;
    case GRN_OPERATION_TIMEOUT:
        exception = eGrnOperationTimeout;
        message = "operation timeout";
        break;
    case GRN_CONNECTION_REFUSED:
        exception = eGrnConnectionRefused;
        message = "connection refused";
        break;
    case GRN_RANGE_ERROR:
        exception = eGrnRangeError;
        message = "range error";
        break;
    case GRN_TOKENIZER_ERROR:
        exception = eGrnTokenizerError;
        message = "tokenizer error";
        break;
    case GRN_FILE_CORRUPT:
        exception = eGrnFileCorrupt;
        message = "file corrupt";
        break;
    case GRN_INVALID_FORMAT:
        exception = eGrnInvalidFormat;
        message = "invalid format";
        break;
    case GRN_OBJECT_CORRUPT:
        exception = eGrnObjectCorrupt;
        message = "object corrupt";
        break;
    case GRN_TOO_MANY_SYMBOLIC_LINKS:
        exception = eGrnTooManySymbolicLinks;
        message = "too many symbolic links";
        break;
    case GRN_NOT_SOCKET:
        exception = eGrnNotSocket;
        message = "not socket";
        break;
    case GRN_OPERATION_NOT_SUPPORTED:
        exception = eGrnOperationNotSupported;
        message = "operation not supported";
        break;
    case GRN_ADDRESS_IS_IN_USE:
        exception = eGrnAddressIsInUse;
        message = "address is in use";
        break;
    case GRN_ZLIB_ERROR:
        exception = eGrnZLibError;
        message = "ZLib error";
        break;
    case GRN_LZO_ERROR:
        exception = eGrnLZOError;
        message = "LZO error";
        break;
    default:
        rb_raise(eGrnError, "invalid return code: %d", rc);
        break;
    }

    rb_raise(exception, "%s", message);
}

void
rb_grn_init_exception (VALUE mGroonga)
{
    eGrnError =
        rb_define_class_under(mGroonga, "Error", rb_eStandardError);
    eGrnEndOfData =
        rb_define_class_under(mGroonga, "EndOfData", eGrnError);
    eGrnUnknownError =
        rb_define_class_under(mGroonga, "UnknownError", eGrnError);
    eGrnOperationNotPermitted =
        rb_define_class_under(mGroonga, "OperationNotPermitted", eGrnError);
    eGrnNoSuchFileOrDirectory =
        rb_define_class_under(mGroonga, "NoSuchFileOrDirectory", eGrnError);
    eGrnNoSuchProcess =
        rb_define_class_under(mGroonga, "NoSuchProcess", eGrnError);
    eGrnInterruptedFunctionCall =
        rb_define_class_under(mGroonga, "InterruptedFunctionCall", eGrnError);
    eGrnInputOutputError =
        rb_define_class_under(mGroonga, "InputOutputError", eGrnError);
    eGrnNoSuchDeviceOrAddress =
        rb_define_class_under(mGroonga, "NoSuchDeviceOrAddress", eGrnError);
    eGrnArgListTooLong =
        rb_define_class_under(mGroonga, "ArgListTooLong", eGrnError);
    eGrnExecFormatError =
        rb_define_class_under(mGroonga, "ExecFormatError", eGrnError);
    eGrnBadFileDescriptor =
        rb_define_class_under(mGroonga, "BadFileDescriptor", eGrnError);
    eGrnNoChildProcesses =
        rb_define_class_under(mGroonga, "NoChildProcesses", eGrnError);
    eGrnResourceTemporarilyUnavailable =
        rb_define_class_under(mGroonga, "ResourceTemporarilyUnavailable", eGrnError);
    eGrnNotEnoughSpace =
        rb_define_class_under(mGroonga, "NotEnoughSpace", eGrnError);
    eGrnPermissionDenied =
        rb_define_class_under(mGroonga, "PermissionDenied", eGrnError);
    eGrnBadAddress =
        rb_define_class_under(mGroonga, "BadAddress", eGrnError);
    eGrnResourceBusy =
        rb_define_class_under(mGroonga, "ResourceBusy", eGrnError);
    eGrnFileExists =
        rb_define_class_under(mGroonga, "FileExists", eGrnError);
    eGrnImproperLink =
        rb_define_class_under(mGroonga, "ImproperLink", eGrnError);
    eGrnNoSuchDevice =
        rb_define_class_under(mGroonga, "NoSuchDevice", eGrnError);
    eGrnNotADirectory =
        rb_define_class_under(mGroonga, "NotADirectory", eGrnError);
    eGrnIsADirectory =
        rb_define_class_under(mGroonga, "IsADirectory", eGrnError);
    eGrnInvalidArgument =
        rb_define_class_under(mGroonga, "InvalidArgument", eGrnError);
    eGrnTooManyOpenFilesInSystem =
        rb_define_class_under(mGroonga, "TooManyOpenFilesInSystem", eGrnError);
    eGrnTooManyOpenFiles =
        rb_define_class_under(mGroonga, "TooManyOpenFiles", eGrnError);
    eGrnInappropriateIOControlOperation =
        rb_define_class_under(mGroonga, "InappropriateIOControlOperation", eGrnError);
    eGrnFileTooLarge =
        rb_define_class_under(mGroonga, "FileTooLarge", eGrnError);
    eGrnNoSpaceLeftOnDevice =
        rb_define_class_under(mGroonga, "NoSpaceLeftOnDevice", eGrnError);
    eGrnInvalidSeek =
        rb_define_class_under(mGroonga, "InvalidSeek", eGrnError);
    eGrnReadOnlyFileSystem =
        rb_define_class_under(mGroonga, "ReadOnlyFileSystem", eGrnError);
    eGrnTooManyLinks =
        rb_define_class_under(mGroonga, "TooManyLinks", eGrnError);
    eGrnBrokenPipe =
        rb_define_class_under(mGroonga, "BrokenPipe", eGrnError);
    eGrnDomainError =
        rb_define_class_under(mGroonga, "DomainError", eGrnError);
    eGrnResultTooLarge =
        rb_define_class_under(mGroonga, "ResultTooLarge", eGrnError);
    eGrnResourceDeadlockAvoided =
        rb_define_class_under(mGroonga, "ResourceDeadlockAvoided", eGrnError);
    eGrnNoMemoryAvailable =
        rb_define_class_under(mGroonga, "NoMemoryAvailable", eGrnError);
    eGrnFilenameTooLong =
        rb_define_class_under(mGroonga, "FilenameTooLong", eGrnError);
    eGrnNoLocksAvailable =
        rb_define_class_under(mGroonga, "NoLocksAvailable", eGrnError);
    eGrnFunctionNotImplemented =
        rb_define_class_under(mGroonga, "FunctionNotImplemented", eGrnError);
    eGrnDirectoryNotEmpty =
        rb_define_class_under(mGroonga, "DirectoryNotEmpty", eGrnError);
    eGrnIllegalByteSequence =
        rb_define_class_under(mGroonga, "IllegalByteSequence", eGrnError);
    eGrnSocketNotInitialised =
        rb_define_class_under(mGroonga, "SocketNotInitialised", eGrnError);
    eGrnOperationWouldBlock =
        rb_define_class_under(mGroonga, "OperationWouldBlock", eGrnError);
    eGrnAddressIsNotAvailable =
        rb_define_class_under(mGroonga, "AddressIsNotAvailable", eGrnError);
    eGrnNetworkIsDown =
        rb_define_class_under(mGroonga, "NetworkIsDown", eGrnError);
    eGrnNoBuffer =
        rb_define_class_under(mGroonga, "NoBuffer", eGrnError);
    eGrnSocketIsAlreadyConnected =
        rb_define_class_under(mGroonga, "SocketIsAlreadyConnected", eGrnError);
    eGrnSocketIsNotConnected =
        rb_define_class_under(mGroonga, "SocketIsNotConnected", eGrnError);
    eGrnSocketIsAlreadyShutdowned =
        rb_define_class_under(mGroonga, "SocketIsAlreadyShutdowned", eGrnError);
    eGrnOperationTimeout =
        rb_define_class_under(mGroonga, "OperationTimeout", eGrnError);
    eGrnConnectionRefused =
        rb_define_class_under(mGroonga, "ConnectionRefused", eGrnError);
    eGrnRangeError =
        rb_define_class_under(mGroonga, "RangeError", eGrnError);
    eGrnTokenizerError =
        rb_define_class_under(mGroonga, "TokenizerError", eGrnError);
    eGrnFileCorrupt =
        rb_define_class_under(mGroonga, "FileCorrupt", eGrnError);
    eGrnInvalidFormat =
        rb_define_class_under(mGroonga, "InvalidFormat", eGrnError);
    eGrnObjectCorrupt =
        rb_define_class_under(mGroonga, "ObjectCorrupt", eGrnError);
    eGrnTooManySymbolicLinks =
        rb_define_class_under(mGroonga, "TooManySymbolicLinks", eGrnError);
    eGrnNotSocket =
        rb_define_class_under(mGroonga, "NotSocket", eGrnError);
    eGrnOperationNotSupported =
        rb_define_class_under(mGroonga, "OperationNotSupported", eGrnError);
    eGrnAddressIsInUse =
        rb_define_class_under(mGroonga, "AddressIsInUse", eGrnError);
    eGrnZLibError =
        rb_define_class_under(mGroonga, "ZLibError", eGrnError);
    eGrnLZOError =
        rb_define_class_under(mGroonga, "LZOError", eGrnError);
}
