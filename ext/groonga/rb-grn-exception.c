/* -*- coding: utf-8; c-file-style: "ruby" -*- */
/*
  Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "rb-grn.h"

VALUE rb_eGrnError;
VALUE rb_eGrnClosed;
VALUE rb_eGrnNoSuchColumn;

static VALUE eGrnEndOfData;
static VALUE eGrnUnknownError;
static VALUE eGrnOperationNotPermitted;
static VALUE eGrnNoSuchFileOrDirectory;
static VALUE eGrnNoSuchProcess;
static VALUE eGrnInterruptedFunctionCall;
static VALUE eGrnInputOutputError;
static VALUE eGrnNoSuchDeviceOrAddress;
static VALUE eGrnArgumentListTooLong;
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
VALUE rb_eGrnInvalidArgument;
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
static VALUE eGrnSocketNotInitialized;
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
static VALUE eGrnStackOverFlow;
static VALUE eGrnSyntaxError;
static VALUE eGrnRetryMax;
static VALUE eGrnIncompatibleFileFormat;
static VALUE eGrnUpdateNotAllowed;
static VALUE eGrnTooSmallOffset;
static VALUE eGrnTooLargeOffset;
static VALUE eGrnTooSmallLimit;
static VALUE eGrnCASError;
static VALUE eGrnUnsupportedCommandVersion;

VALUE
rb_grn_rc_to_exception (grn_rc rc)
{
    VALUE exception = Qnil;

    switch (rc) {
      case GRN_SUCCESS:
	return Qnil;
        break;
      case GRN_END_OF_DATA:
        exception = eGrnEndOfData;
        break;
      case GRN_UNKNOWN_ERROR:
        exception = eGrnUnknownError;
        break;
      case GRN_OPERATION_NOT_PERMITTED:
        exception = eGrnOperationNotPermitted;
        break;
      case GRN_NO_SUCH_FILE_OR_DIRECTORY:
        exception = eGrnNoSuchFileOrDirectory;
        break;
      case GRN_NO_SUCH_PROCESS:
        exception = eGrnNoSuchProcess;
        break;
      case GRN_INTERRUPTED_FUNCTION_CALL:
        exception = eGrnInterruptedFunctionCall;
        break;
      case GRN_INPUT_OUTPUT_ERROR:
        exception = eGrnInputOutputError;
        break;
      case GRN_NO_SUCH_DEVICE_OR_ADDRESS:
        exception = eGrnNoSuchDeviceOrAddress;
        break;
      case GRN_ARG_LIST_TOO_LONG:
        exception = eGrnArgumentListTooLong;
        break;
      case GRN_EXEC_FORMAT_ERROR:
        exception = eGrnExecFormatError;
        break;
      case GRN_BAD_FILE_DESCRIPTOR:
        exception = eGrnBadFileDescriptor;
        break;
      case GRN_NO_CHILD_PROCESSES:
        exception = eGrnNoChildProcesses;
        break;
      case GRN_RESOURCE_TEMPORARILY_UNAVAILABLE:
        exception = eGrnResourceTemporarilyUnavailable;
        break;
      case GRN_NOT_ENOUGH_SPACE:
        exception = eGrnNotEnoughSpace;
        break;
      case GRN_PERMISSION_DENIED:
        exception = eGrnPermissionDenied;
        break;
      case GRN_BAD_ADDRESS:
        exception = eGrnBadAddress;
        break;
      case GRN_RESOURCE_BUSY:
        exception = eGrnResourceBusy;
        break;
      case GRN_FILE_EXISTS:
        exception = eGrnFileExists;
        break;
      case GRN_IMPROPER_LINK:
        exception = eGrnImproperLink;
        break;
      case GRN_NO_SUCH_DEVICE:
        exception = eGrnNoSuchDevice;
        break;
      case GRN_NOT_A_DIRECTORY:
        exception = eGrnNotADirectory;
        break;
      case GRN_IS_A_DIRECTORY:
        exception = eGrnIsADirectory;
        break;
      case GRN_INVALID_ARGUMENT:
        exception = rb_eGrnInvalidArgument;
        break;
      case GRN_TOO_MANY_OPEN_FILES_IN_SYSTEM:
        exception = eGrnTooManyOpenFilesInSystem;
        break;
      case GRN_TOO_MANY_OPEN_FILES:
        exception = eGrnTooManyOpenFiles;
        break;
      case GRN_INAPPROPRIATE_I_O_CONTROL_OPERATION:
        exception = eGrnInappropriateIOControlOperation;
        break;
      case GRN_FILE_TOO_LARGE:
        exception = eGrnFileTooLarge;
        break;
      case GRN_NO_SPACE_LEFT_ON_DEVICE:
        exception = eGrnNoSpaceLeftOnDevice;
        break;
      case GRN_INVALID_SEEK:
        exception = eGrnInvalidSeek;
        break;
      case GRN_READ_ONLY_FILE_SYSTEM:
        exception = eGrnReadOnlyFileSystem;
        break;
      case GRN_TOO_MANY_LINKS:
        exception = eGrnTooManyLinks;
        break;
      case GRN_BROKEN_PIPE:
        exception = eGrnBrokenPipe;
        break;
      case GRN_DOMAIN_ERROR:
        exception = eGrnDomainError;
        break;
      case GRN_RESULT_TOO_LARGE:
        exception = eGrnResultTooLarge;
        break;
      case GRN_RESOURCE_DEADLOCK_AVOIDED:
        exception = eGrnResourceDeadlockAvoided;
        break;
      case GRN_NO_MEMORY_AVAILABLE:
        exception = eGrnNoMemoryAvailable;
        break;
      case GRN_FILENAME_TOO_LONG:
        exception = eGrnFilenameTooLong;
        break;
      case GRN_NO_LOCKS_AVAILABLE:
        exception = eGrnNoLocksAvailable;
        break;
      case GRN_FUNCTION_NOT_IMPLEMENTED:
        exception = eGrnFunctionNotImplemented;
        break;
      case GRN_DIRECTORY_NOT_EMPTY:
        exception = eGrnDirectoryNotEmpty;
        break;
      case GRN_ILLEGAL_BYTE_SEQUENCE:
        exception = eGrnIllegalByteSequence;
        break;
      case GRN_SOCKET_NOT_INITIALIZED:
        exception = eGrnSocketNotInitialized;
        break;
      case GRN_OPERATION_WOULD_BLOCK:
        exception = eGrnOperationWouldBlock;
        break;
      case GRN_ADDRESS_IS_NOT_AVAILABLE:
        exception = eGrnAddressIsNotAvailable;
        break;
      case GRN_NETWORK_IS_DOWN:
        exception = eGrnNetworkIsDown;
        break;
      case GRN_NO_BUFFER:
        exception = eGrnNoBuffer;
        break;
      case GRN_SOCKET_IS_ALREADY_CONNECTED:
        exception = eGrnSocketIsAlreadyConnected;
        break;
      case GRN_SOCKET_IS_NOT_CONNECTED:
        exception = eGrnSocketIsNotConnected;
        break;
      case GRN_SOCKET_IS_ALREADY_SHUTDOWNED:
        exception = eGrnSocketIsAlreadyShutdowned;
        break;
      case GRN_OPERATION_TIMEOUT:
        exception = eGrnOperationTimeout;
        break;
      case GRN_CONNECTION_REFUSED:
        exception = eGrnConnectionRefused;
        break;
      case GRN_RANGE_ERROR:
        exception = eGrnRangeError;
        break;
      case GRN_TOKENIZER_ERROR:
        exception = eGrnTokenizerError;
        break;
      case GRN_FILE_CORRUPT:
        exception = eGrnFileCorrupt;
        break;
      case GRN_INVALID_FORMAT:
        exception = eGrnInvalidFormat;
        break;
      case GRN_OBJECT_CORRUPT:
        exception = eGrnObjectCorrupt;
        break;
      case GRN_TOO_MANY_SYMBOLIC_LINKS:
        exception = eGrnTooManySymbolicLinks;
        break;
      case GRN_NOT_SOCKET:
        exception = eGrnNotSocket;
        break;
      case GRN_OPERATION_NOT_SUPPORTED:
        exception = eGrnOperationNotSupported;
        break;
      case GRN_ADDRESS_IS_IN_USE:
        exception = eGrnAddressIsInUse;
        break;
      case GRN_ZLIB_ERROR:
        exception = eGrnZLibError;
        break;
      case GRN_LZO_ERROR:
        exception = eGrnLZOError;
        break;
      case GRN_STACK_OVER_FLOW:
        exception = eGrnStackOverFlow;
        break;
      case GRN_SYNTAX_ERROR:
        exception = eGrnSyntaxError;
        break;
      case GRN_RETRY_MAX:
        exception = eGrnRetryMax;
        break;
      case GRN_INCOMPATIBLE_FILE_FORMAT:
        exception = eGrnIncompatibleFileFormat;
        break;
      case GRN_UPDATE_NOT_ALLOWED:
        exception = eGrnUpdateNotAllowed;
        break;
      case GRN_TOO_SMALL_OFFSET:
        exception = eGrnTooSmallOffset;
        break;
      case GRN_TOO_LARGE_OFFSET:
        exception = eGrnTooLargeOffset;
        break;
      case GRN_TOO_SMALL_LIMIT:
        exception = eGrnTooSmallLimit;
        break;
      case GRN_CAS_ERROR:
        exception = eGrnCASError;
        break;
      case GRN_UNSUPPORTED_COMMAND_VERSION:
        exception = eGrnUnsupportedCommandVersion;
        break;
    }

    if (NIL_P(exception))
        rb_raise(rb_eGrnError, "invalid return code: %d", rc);

    return exception;
}

const char *
rb_grn_rc_to_message (grn_rc rc)
{
    const char *message = NULL;

    switch (rc) {
      case GRN_SUCCESS:
	return NULL;
	break;
      case GRN_END_OF_DATA:
        message = "end of data";
        break;
      case GRN_UNKNOWN_ERROR:
        message = "unknown error";
        break;
      case GRN_OPERATION_NOT_PERMITTED:
        message = "operation not permitted";
        break;
      case GRN_NO_SUCH_FILE_OR_DIRECTORY:
        message = "no such file or directory";
        break;
      case GRN_NO_SUCH_PROCESS:
        message = "no such process";
        break;
      case GRN_INTERRUPTED_FUNCTION_CALL:
        message = "interrupted function call";
        break;
      case GRN_INPUT_OUTPUT_ERROR:
        message = "input output error";
        break;
      case GRN_NO_SUCH_DEVICE_OR_ADDRESS:
        message = "no such device or address";
        break;
      case GRN_ARG_LIST_TOO_LONG:
        message = "argument list too long";
        break;
      case GRN_EXEC_FORMAT_ERROR:
        message = "exec format error";
        break;
      case GRN_BAD_FILE_DESCRIPTOR:
        message = "bad file descriptor";
        break;
      case GRN_NO_CHILD_PROCESSES:
        message = "no child processes";
        break;
      case GRN_RESOURCE_TEMPORARILY_UNAVAILABLE:
        message = "resource temporarily unavailable";
        break;
      case GRN_NOT_ENOUGH_SPACE:
        message = "not enough space";
        break;
      case GRN_PERMISSION_DENIED:
        message = "permission denied";
        break;
      case GRN_BAD_ADDRESS:
        message = "bad address";
        break;
      case GRN_RESOURCE_BUSY:
        message = "resource busy";
        break;
      case GRN_FILE_EXISTS:
        message = "file exists";
        break;
      case GRN_IMPROPER_LINK:
        message = "improper link";
        break;
      case GRN_NO_SUCH_DEVICE:
        message = "no such device";
        break;
      case GRN_NOT_A_DIRECTORY:
        message = "not a directory";
        break;
      case GRN_IS_A_DIRECTORY:
        message = "is a directory";
        break;
      case GRN_INVALID_ARGUMENT:
        message = "invalid argument";
        break;
      case GRN_TOO_MANY_OPEN_FILES_IN_SYSTEM:
        message = "too many open files in system";
        break;
      case GRN_TOO_MANY_OPEN_FILES:
        message = "too many open files";
        break;
      case GRN_INAPPROPRIATE_I_O_CONTROL_OPERATION:
        message = "inappropriate I/O control operation";
        break;
      case GRN_FILE_TOO_LARGE:
        message = "file too large";
        break;
      case GRN_NO_SPACE_LEFT_ON_DEVICE:
        message = "no space left on device";
        break;
      case GRN_INVALID_SEEK:
        message = "invalid seek";
        break;
      case GRN_READ_ONLY_FILE_SYSTEM:
        message = "read only file system";
        break;
      case GRN_TOO_MANY_LINKS:
        message = "too many links";
        break;
      case GRN_BROKEN_PIPE:
        message = "broken pipe";
        break;
      case GRN_DOMAIN_ERROR:
        message = "domain error";
        break;
      case GRN_RESULT_TOO_LARGE:
        message = "result too large";
        break;
      case GRN_RESOURCE_DEADLOCK_AVOIDED:
        message = "resource deadlock avoided";
        break;
      case GRN_NO_MEMORY_AVAILABLE:
        message = "no memory available";
        break;
      case GRN_FILENAME_TOO_LONG:
        message = "filename too long";
        break;
      case GRN_NO_LOCKS_AVAILABLE:
        message = "no locks available";
        break;
      case GRN_FUNCTION_NOT_IMPLEMENTED:
        message = "function not implemented";
        break;
      case GRN_DIRECTORY_NOT_EMPTY:
        message = "directory not empty";
        break;
      case GRN_ILLEGAL_BYTE_SEQUENCE:
        message = "illegal byte sequence";
        break;
      case GRN_SOCKET_NOT_INITIALIZED:
        message = "socket not initialized";
        break;
      case GRN_OPERATION_WOULD_BLOCK:
        message = "operation would block";
        break;
      case GRN_ADDRESS_IS_NOT_AVAILABLE:
        message = "address is not available";
        break;
      case GRN_NETWORK_IS_DOWN:
        message = "network is down";
        break;
      case GRN_NO_BUFFER:
        message = "no buffer";
        break;
      case GRN_SOCKET_IS_ALREADY_CONNECTED:
        message = "socket is already connected";
        break;
      case GRN_SOCKET_IS_NOT_CONNECTED:
        message = "socket is not connected";
        break;
      case GRN_SOCKET_IS_ALREADY_SHUTDOWNED:
        message = "socket is already shutdowned";
        break;
      case GRN_OPERATION_TIMEOUT:
        message = "operation timeout";
        break;
      case GRN_CONNECTION_REFUSED:
        message = "connection refused";
        break;
      case GRN_RANGE_ERROR:
        message = "range error";
        break;
      case GRN_TOKENIZER_ERROR:
        message = "tokenizer error";
        break;
      case GRN_FILE_CORRUPT:
        message = "file corrupt";
        break;
      case GRN_INVALID_FORMAT:
        message = "invalid format";
        break;
      case GRN_OBJECT_CORRUPT:
        message = "object corrupt";
        break;
      case GRN_TOO_MANY_SYMBOLIC_LINKS:
        message = "too many symbolic links";
        break;
      case GRN_NOT_SOCKET:
        message = "not socket";
        break;
      case GRN_OPERATION_NOT_SUPPORTED:
        message = "operation not supported";
        break;
      case GRN_ADDRESS_IS_IN_USE:
        message = "address is in use";
        break;
      case GRN_ZLIB_ERROR:
        message = "ZLib error";
        break;
      case GRN_LZO_ERROR:
        message = "LZO error";
        break;
      case GRN_STACK_OVER_FLOW:
        message = "stack over flow";
        break;
      case GRN_SYNTAX_ERROR:
        message = "syntax error";
        break;
      case GRN_RETRY_MAX:
        message = "retry max";
        break;
      case GRN_INCOMPATIBLE_FILE_FORMAT:
        message = "incompatible file format";
        break;
      case GRN_UPDATE_NOT_ALLOWED:
        message = "update isn't allowed";
        break;
      case GRN_TOO_SMALL_OFFSET:
        message = "too small offset";
        break;
      case GRN_TOO_LARGE_OFFSET:
        message = "too large offset";
        break;
      case GRN_TOO_SMALL_LIMIT:
        message = "too small limit";
        break;
      case GRN_CAS_ERROR:
        message = "CAS error";
        break;
      case GRN_UNSUPPORTED_COMMAND_VERSION:
        message = "unsupported command version";
        break;
    }

    if (!message)
        rb_raise(rb_eGrnError, "invalid return code: %d", rc);

    return message;
}

void
rb_grn_rc_check (grn_rc rc, VALUE related_object)
{
    VALUE exception;
    const char *message;

    exception = rb_grn_rc_to_exception(rc);
    if (NIL_P(exception))
	return;
    message = rb_grn_rc_to_message(rc);

    if (NIL_P(related_object))
	rb_raise(exception, "%s", message);
    else
	rb_raise(exception, "%s: %s", rb_grn_inspect(related_object),  message);
}

void
rb_grn_init_exception (VALUE mGrn)
{
    /*
     * Document-class: Groonga::Error
     *
     * rroongaが発生する例外のスーパークラス。
     */
    rb_eGrnError =
        rb_define_class_under(mGrn, "Error", rb_eStandardError);

    /*
     * Document-class: Groonga::Closed
     *
     * groongaレベルでは破棄されているが、Rubyレベルでは生き
     * ているオブジェクトにアクセスすると発生する。
     *
     * @since 1.2.3
     */
    rb_eGrnClosed = rb_define_class_under(mGrn, "Closed", rb_eGrnError);

    /*
     * Document-class: Groonga::ObjectClosed
     *
     * groongaレベルでは破棄されているが、Rubyレベルでは生き
     * ているオブジェクトにアクセスすると発生する。
     *
     * @deprecated since 1.2.3. Use Groonga::Closed instead.
     */
    rb_define_const(mGrn, "ObjectClosed", rb_eGrnClosed);

    /*
     * Document-class: Groonga::NoSuchColumn
     *
     * 存在しないカラムにアクセスすると発生する。
     */
    rb_eGrnNoSuchColumn =
        rb_define_class_under(mGrn, "NoSuchColumn", rb_eGrnError);

    /*
     * Document-class: Groonga::EndOfData
     *
     * データの終端に達したときに発生する。
     */
    eGrnEndOfData =
        rb_define_class_under(mGrn, "EndOfData", rb_eGrnError);

    /*
     * Document-class: Groonga::UnknownError
     *
     * 未知のエラーが発生したときに発生する。
     */
    eGrnUnknownError =
        rb_define_class_under(mGrn, "UnknownError", rb_eGrnError);

    /*
     * Document-class: Groonga::OperationNotPermitted
     *
     * 操作が許可されていないときに発生する。
     */
    eGrnOperationNotPermitted =
        rb_define_class_under(mGrn, "OperationNotPermitted", rb_eGrnError);

    /*
     * Document-class: Groonga::NoSuchFileOrDirectory
     *
     * ファイルまたはディレクトリがないときに発生する。
     */
    eGrnNoSuchFileOrDirectory =
        rb_define_class_under(mGrn, "NoSuchFileOrDirectory", rb_eGrnError);

    /*
     * Document-class: Groonga::NoSuchProcess
     *
     * プロセスがないときに発生する。
     */
    eGrnNoSuchProcess =
        rb_define_class_under(mGrn, "NoSuchProcess", rb_eGrnError);

    /*
     * Document-class: Groonga::InterruptedFunctionCall
     *
     * 関数の実行中に中断されたときに発生する。
     */
    eGrnInterruptedFunctionCall =
        rb_define_class_under(mGrn, "InterruptedFunctionCall", rb_eGrnError);

    /*
     * Document-class: Groonga::InputOutputError
     *
     * 入出力エラーが起きたときに発生する。
     */
    eGrnInputOutputError =
        rb_define_class_under(mGrn, "InputOutputError", rb_eGrnError);

    /*
     * Document-class: Groonga::NoSuchDeviceOrAddress
     *
     * デバイスまたはアドレスがないときに発生する。
     */
    eGrnNoSuchDeviceOrAddress =
        rb_define_class_under(mGrn, "NoSuchDeviceOrAddress", rb_eGrnError);

    /*
     * Document-class: Groonga::ArgumentListTooLong
     *
     * 引数の数が多すぎるときに発生する。
     */
    eGrnArgumentListTooLong =
        rb_define_class_under(mGrn, "ArgumentListTooLong", rb_eGrnError);

    /*
     * Document-class: Groonga::ExecFormatError
     *
     * 実行ファイルのフォーマットに問題があるときに発生する。
     */
    eGrnExecFormatError =
        rb_define_class_under(mGrn, "ExecFormatError", rb_eGrnError);

    /*
     * Document-class: Groonga::BadFileDescriptor
     *
     * ファイルディスクリプタに問題があるときに発生する。
     */
    eGrnBadFileDescriptor =
        rb_define_class_under(mGrn, "BadFileDescriptor", rb_eGrnError);

    /*
     * Document-class: Groonga::NoChildProcesses
     *
     * 子プロセスがないときに発生する。
     */
    eGrnNoChildProcesses =
        rb_define_class_under(mGrn, "NoChildProcesses", rb_eGrnError);

    /*
     * Document-class: Groonga::ResourceTemporarilyUnavailable
     *
     * 一時的にリソースを利用できないときに発生する。
     */
    eGrnResourceTemporarilyUnavailable =
        rb_define_class_under(mGrn, "ResourceTemporarilyUnavailable",
			      rb_eGrnError);

    /*
     * Document-class: Groonga::NotEnoughSpace
     *
     * 領域が足りないときに発生する。
     */
    eGrnNotEnoughSpace =
        rb_define_class_under(mGrn, "NotEnoughSpace", rb_eGrnError);

    /*
     * Document-class: Groonga::PermissionDenied
     *
     * 許可がないときに発生する。
     */
    eGrnPermissionDenied =
        rb_define_class_under(mGrn, "PermissionDenied", rb_eGrnError);

    /*
     * Document-class: Groonga::BadAddress
     *
     * アドレスに問題があるときに発生する。
     */
    eGrnBadAddress =
        rb_define_class_under(mGrn, "BadAddress", rb_eGrnError);

    /*
     * Document-class: Groonga::ResourceBusy
     *
     * リソースが使用中のときに発生する。
     */
    eGrnResourceBusy =
        rb_define_class_under(mGrn, "ResourceBusy", rb_eGrnError);

    /*
     * Document-class: Groonga::FileExists
     *
     * ファイルが存在しているときに発生する。
     */
    eGrnFileExists =
        rb_define_class_under(mGrn, "FileExists", rb_eGrnError);

    /*
     * Document-class: Groonga::ImproperLink
     *
     * リンクに問題があるときに発生する。
     */
    eGrnImproperLink =
        rb_define_class_under(mGrn, "ImproperLink", rb_eGrnError);

    /*
     * Document-class: Groonga::NoSuchDevice
     *
     * デバイスがないときに発生する。
     */
    eGrnNoSuchDevice =
        rb_define_class_under(mGrn, "NoSuchDevice", rb_eGrnError);

    /*
     * Document-class: Groonga::NotADirectory
     *
     * ディレクトリではないときに発生する。
     */
    eGrnNotADirectory =
        rb_define_class_under(mGrn, "NotADirectory", rb_eGrnError);

    /*
     * Document-class: Groonga::IsADirectory
     *
     * ディレクトリのときに発生する。
     */
    eGrnIsADirectory =
        rb_define_class_under(mGrn, "IsADirectory", rb_eGrnError);

    /*
     * Document-class: Groonga::InvalidArgument
     *
     * 引数に問題があるときに発生する。
     */
    rb_eGrnInvalidArgument =
        rb_define_class_under(mGrn, "InvalidArgument", rb_eGrnError);

    /*
     * Document-class: Groonga::TooManyOpenFilesInSystem
     *
     * システム全体で開いているファイルが多すぎるときに発生する。
     */
    eGrnTooManyOpenFilesInSystem =
        rb_define_class_under(mGrn, "TooManyOpenFilesInSystem", rb_eGrnError);

    /*
     * Document-class: Groonga::TooManyOpenFiles
     *
     * 開いているファイルが多すぎるときに発生する。
     */
    eGrnTooManyOpenFiles =
        rb_define_class_under(mGrn, "TooManyOpenFiles", rb_eGrnError);

    /*
     * Document-class: Groonga::InappropriateIOControlOperation
     *
     * IO制御に問題があるときに発生する。
     */
    eGrnInappropriateIOControlOperation =
        rb_define_class_under(mGrn, "InappropriateIOControlOperation", rb_eGrnError);

    /*
     * Document-class: Groonga::FileTooLarge
     *
     * ファイルが大きすぎるときに発生する。
     */
    eGrnFileTooLarge =
        rb_define_class_under(mGrn, "FileTooLarge", rb_eGrnError);

    /*
     * Document-class: Groonga::NoSpaceLeftOnDevice
     *
     * デバイスに空いている領域がないときに発生する。
     */
    eGrnNoSpaceLeftOnDevice =
        rb_define_class_under(mGrn, "NoSpaceLeftOnDevice", rb_eGrnError);

    /*
     * Document-class: Groonga::InvalidSeek
     *
     * seekに問題があるときに発生する。
     */
    eGrnInvalidSeek =
        rb_define_class_under(mGrn, "InvalidSeek", rb_eGrnError);

    /*
     * Document-class: Groonga::ReadOnlyFileSystem
     *
     * ファイルシステムが読込専用のときに発生する。
     */
    eGrnReadOnlyFileSystem =
        rb_define_class_under(mGrn, "ReadOnlyFileSystem", rb_eGrnError);

    /*
     * Document-class: Groonga::TooManyLinks
     *
     * リンクが多すぎるときに発生する。
     */
    eGrnTooManyLinks =
        rb_define_class_under(mGrn, "TooManyLinks", rb_eGrnError);

    /*
     * Document-class: Groonga::BrokenPipe
     *
     * パイプが壊れているときに発生する。
     */
    eGrnBrokenPipe =
        rb_define_class_under(mGrn, "BrokenPipe", rb_eGrnError);

    /*
     * Document-class: Groonga::DomainError
     *
     * 対象領域に問題があるときに発生する。
     */
    eGrnDomainError =
        rb_define_class_under(mGrn, "DomainError", rb_eGrnError);

    /*
     * Document-class: Groonga::ResultTooLarge
     *
     * 結果が多すぎるときに発生する。
     */
    eGrnResultTooLarge =
        rb_define_class_under(mGrn, "ResultTooLarge", rb_eGrnError);

    /*
     * Document-class: Groonga::ResourceDeadlockAvoided
     *
     * リソースのデッドロックを回避したときに発生する。
     */
    eGrnResourceDeadlockAvoided =
        rb_define_class_under(mGrn, "ResourceDeadlockAvoided", rb_eGrnError);

    /*
     * Document-class: Groonga::NoMemoryAvailable
     *
     * メモリが足りないときに発生する。
     */
    eGrnNoMemoryAvailable =
        rb_define_class_under(mGrn, "NoMemoryAvailable", rb_eGrnError);

    /*
     * Document-class: Groonga::FilenameTooLong
     *
     * ファイル名が長すぎるときに発生する。
     */
    eGrnFilenameTooLong =
        rb_define_class_under(mGrn, "FilenameTooLong", rb_eGrnError);

    /*
     * Document-class: Groonga::NoLocksAvailable
     *
     * ロックがないときに発生する。
     */
    eGrnNoLocksAvailable =
        rb_define_class_under(mGrn, "NoLocksAvailable", rb_eGrnError);

    /*
     * Document-class: Groonga::FunctionNotImplemented
     *
     * 関数を実装していないときに発生する。
     */
    eGrnFunctionNotImplemented =
        rb_define_class_under(mGrn, "FunctionNotImplemented", rb_eGrnError);

    /*
     * Document-class: Groonga::DirectoryNotEmpty
     *
     * ディレクトリが空でないときに発生する。
     */
    eGrnDirectoryNotEmpty =
        rb_define_class_under(mGrn, "DirectoryNotEmpty", rb_eGrnError);

    /*
     * Document-class: Groonga::IllegalByteSequence
     *
     * バイト列に問題があるときに発生する。
     */
    eGrnIllegalByteSequence =
        rb_define_class_under(mGrn, "IllegalByteSequence", rb_eGrnError);

    /*
     * Document-class: Groonga::SocketNotInitialized
     *
     * ソケットが初期化されていないときに発生する。
     */
    eGrnSocketNotInitialized =
        rb_define_class_under(mGrn, "SocketNotInitialized", rb_eGrnError);

    /*
     * Document-class: Groonga::OperationWouldBlock
     *
     * 操作がブロックする可能性があるときに発生する。
     */
    eGrnOperationWouldBlock =
        rb_define_class_under(mGrn, "OperationWouldBlock", rb_eGrnError);

    /*
     * Document-class: Groonga::AddressIsNotAvailable
     *
     * アドレスを利用できないときに発生する。
     */
    eGrnAddressIsNotAvailable =
        rb_define_class_under(mGrn, "AddressIsNotAvailable", rb_eGrnError);

    /*
     * Document-class: Groonga::NetworkIsDown
     *
     * ネットワークに問題があるときに発生する。
     */
    eGrnNetworkIsDown =
        rb_define_class_under(mGrn, "NetworkIsDown", rb_eGrnError);

    /*
     * Document-class: Groonga::NoBuffer
     *
     * バッファがないときに発生する。
     */
    eGrnNoBuffer =
        rb_define_class_under(mGrn, "NoBuffer", rb_eGrnError);

    /*
     * Document-class: Groonga::SocketIsAlreadyConnected
     *
     * ソケットが接続済みのときに発生する。
     */
    eGrnSocketIsAlreadyConnected =
        rb_define_class_under(mGrn, "SocketIsAlreadyConnected", rb_eGrnError);

    /*
     * Document-class: Groonga::SocketIsNotConnected
     *
     * ソケットが接続されていないときに発生する。
     */
    eGrnSocketIsNotConnected =
        rb_define_class_under(mGrn, "SocketIsNotConnected", rb_eGrnError);

    /*
     * Document-class: Groonga::SocketIsAlreadyShutdowned
     *
     * ソケットがすでに閉じられているときに発生する。
     */
    eGrnSocketIsAlreadyShutdowned =
        rb_define_class_under(mGrn, "SocketIsAlreadyShutdowned", rb_eGrnError);

    /*
     * Document-class: Groonga::OperationTimeout
     *
     * 操作がタイムアウトしたときに発生する。
     */
    eGrnOperationTimeout =
        rb_define_class_under(mGrn, "OperationTimeout", rb_eGrnError);

    /*
     * Document-class: Groonga::ConnectionRefused
     *
     * 接続を拒否されたときに発生する。
     */
    eGrnConnectionRefused =
        rb_define_class_under(mGrn, "ConnectionRefused", rb_eGrnError);

    /*
     * Document-class: Groonga::RangeError
     *
     * 範囲外のときに発生する。
     */
    eGrnRangeError =
        rb_define_class_under(mGrn, "RangeError", rb_eGrnError);

    /*
     * Document-class: Groonga::TokenizerError
     *
     * トークナイザーに問題があったときに発生する。
     */
    eGrnTokenizerError =
        rb_define_class_under(mGrn, "TokenizerError", rb_eGrnError);

    /*
     * Document-class: Groonga::FileCorrupt
     *
     * ファイルに問題があったときに発生する。
     */
    eGrnFileCorrupt =
        rb_define_class_under(mGrn, "FileCorrupt", rb_eGrnError);

    /*
     * Document-class: Groonga::InvalidFormat
     *
     * フォーマットに問題があったときに発生する。
     */
    eGrnInvalidFormat =
        rb_define_class_under(mGrn, "InvalidFormat", rb_eGrnError);

    /*
     * Document-class: Groonga::ObjectCorrupt
     *
     * オブジェクトに問題があったときに発生する。
     */
    eGrnObjectCorrupt =
        rb_define_class_under(mGrn, "ObjectCorrupt", rb_eGrnError);

    /*
     * Document-class: Groonga::TooManySymbolicLinks
     *
     * シンボリックリンクが多すぎるときに発生する。
     */
    eGrnTooManySymbolicLinks =
        rb_define_class_under(mGrn, "TooManySymbolicLinks", rb_eGrnError);

    /*
     * Document-class: Groonga::NotSocket
     *
     * ソケットではないときに発生する。
     */
    eGrnNotSocket =
        rb_define_class_under(mGrn, "NotSocket", rb_eGrnError);

    /*
     * Document-class: Groonga::OperationNotSupported
     *
     * 操作がサポートされていないときに発生する。
     */
    eGrnOperationNotSupported =
        rb_define_class_under(mGrn, "OperationNotSupported", rb_eGrnError);

    /*
     * Document-class: Groonga::AddressIsInUse
     *
     * アドレスが使用中のときに発生する。
     */
    eGrnAddressIsInUse =
        rb_define_class_under(mGrn, "AddressIsInUse", rb_eGrnError);

    /*
     * Document-class: Groonga::ZLibError
     *
     * ZLibに問題があるときに発生する。
     */
    eGrnZLibError =
        rb_define_class_under(mGrn, "ZLibError", rb_eGrnError);

    /*
     * Document-class: Groonga::LZOError
     *
     * LZOに問題があるときに発生する。
     */
    eGrnLZOError =
        rb_define_class_under(mGrn, "LZOError", rb_eGrnError);

    /*
     * Document-class: Groonga::StackOverFlow
     *
     * スタックオーバーフロー時に発生する。
     */
    eGrnStackOverFlow =
        rb_define_class_under(mGrn, "StackOverFlow", rb_eGrnError);

    /*
     * Document-class: Groonga::SyntaxError
     *
     * 構文に問題があるときに発生する。
     */
    eGrnSyntaxError =
        rb_define_class_under(mGrn, "SyntaxError", rb_eGrnError);

    /*
     * Document-class: Groonga::RetryMax
     *
     * 再試行回数が最大数に達したときに発生する。
     */
    eGrnRetryMax =
        rb_define_class_under(mGrn, "RetryMax", rb_eGrnError);

    /*
     * Document-class: Groonga::IncompatibleFileFormat
     *
     * 互換性のないファイルフォーマットを読み込んだときに発生する。
     */
    eGrnIncompatibleFileFormat =
        rb_define_class_under(mGrn, "IncompatibleFileFormat", rb_eGrnError);

    /*
     * Document-class: Groonga::UpdateNotAllowed
     *
     * 更新操作が許可されていないのに更新操作を行ったときに発生する。
     */
    eGrnUpdateNotAllowed =
        rb_define_class_under(mGrn, "UpdateNotAllowed", rb_eGrnError);

    /*
     * Document-class: Groonga::TooSmallOffset
     *
     * offset値が小さすぎるときに発生する。
     */
    eGrnTooSmallOffset =
        rb_define_class_under(mGrn, "TooSmallOffset", rb_eGrnError);

    /*
     * Document-class: Groonga::TooLargeOffset
     *
     * offset値が大きすぎるときに発生する。
     */
    eGrnTooLargeOffset =
        rb_define_class_under(mGrn, "TooLargeOffset", rb_eGrnError);

    /*
     * Document-class: Groonga::TooSmallLimit
     *
     * limit値が小さすぎるときに発生する。
     */
    eGrnTooSmallLimit =
        rb_define_class_under(mGrn, "TooSmallLimit", rb_eGrnError);

    /*
     * Document-class: Groonga::CASError
     *
     * CAS（Compare and Swap）が失敗したときに発生する。
     */
    eGrnCASError = rb_define_class_under(mGrn, "CASError", rb_eGrnError);

    /*
     * Document-class: Groonga::UnsupportedCommandVersion
     *
     * 未サポートのコマンドバージョンを指定したときに発生する。
     */
    eGrnUnsupportedCommandVersion =
	rb_define_class_under(mGrn, "UnsupportedCommandVersion", rb_eGrnError);
}
