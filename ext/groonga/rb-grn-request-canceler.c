/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2016  Kouhei Sutou <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "rb-grn.h"

/*
 * Document-module: Groonga::RequestCanceler
 *
 * This module provides API for canceling requests.
 */

VALUE rb_mGrnRequestCanceler;

/*
 * Registers a request. The request can be canceled by
 * {Groonga::RequestCanceler.cancel}.
 *
 * @example Register a request
 *   request_id = "request-29"
 *   Groonga::RequestCanceler.register(request_id)
 *
 * @overload register(request_id)
 *   @param request_id [String] The ID of request to be registered.
 *   @return [void]
 *
 * @since 6.0.2
 */
static VALUE
rb_grn_request_canceler_s_register (int argc, VALUE *argv, VALUE module)
{
    VALUE rb_request_id;
    VALUE rb_options;
    VALUE rb_context;
    const char *request_id;
    unsigned int request_id_size;
    grn_ctx *context;

    rb_scan_args(argc, argv, "11", &rb_request_id, &rb_options);
    rb_grn_scan_options(rb_options,
                        "context", &rb_context,
                        NULL);
    context = rb_grn_context_ensure(&rb_context);

    request_id = StringValuePtr(rb_request_id);
    request_id_size = RSTRING_LEN(rb_request_id);
    grn_request_canceler_register(context, request_id, request_id_size);

    return Qnil;
}

/*
 * Unregisters the specified request.
 *
 * @example Unregister a request by ID
 *   request_id = "request-29"
 *   Groonga::RequestCanceler.unregister(request_id)
 *
 * @overload unregister(request_id)
 *   @param request_id [String] The ID of request to be unregistered.
 *   @return [void]
 *
 * @since 6.0.2
 */
static VALUE
rb_grn_request_canceler_s_unregister (int argc, VALUE *argv, VALUE module)
{
    VALUE rb_request_id;
    VALUE rb_options;
    VALUE rb_context;
    const char *request_id;
    unsigned int request_id_size;
    grn_ctx *context;

    rb_scan_args(argc, argv, "11", &rb_request_id, &rb_options);
    rb_grn_scan_options(rb_options,
                        "context", &rb_context,
                        NULL);
    context = rb_grn_context_ensure(&rb_context);

    request_id = StringValuePtr(rb_request_id);
    request_id_size = RSTRING_LEN(rb_request_id);
    grn_request_canceler_unregister(context, request_id, request_id_size);

    return Qnil;
}

/*
 * Cancels the specified request.
 *
 * @example Cancels a request by ID
 *   request_id = "request-29"
 *   Groonga::RequestCanceler.cancel(request_id)
 *
 * @overload cancel(request_id)
 *   @param request_id [String] The ID of request to be canceled.
 *   @return [Boolean] `true` if the request is canceled, `false` otherwise.
 *
 * @since 6.0.2
 */
static VALUE
rb_grn_request_canceler_s_cancel (VALUE module, VALUE rb_request_id)
{
    const char *request_id;
    unsigned int request_id_size;
    grn_bool canceled;

    request_id = StringValuePtr(rb_request_id);
    request_id_size = RSTRING_LEN(rb_request_id);
    canceled = grn_request_canceler_cancel(request_id, request_id_size);

    return CBOOL2RVAL(canceled);
}

/*
 * Cancels all running requests.
 *
 * @example Cancels all requests
 *   Groonga::RequestCanceler.cancel_all
 *
 * @overload cancel_all
 *   @return [Boolean] `true` if one or more requests are canceled,
 *     `false` otherwise.
 *
 * @since 6.0.2
 */
static VALUE
rb_grn_request_canceler_s_cancel_all (VALUE module)
{
    grn_bool canceled;

    canceled = grn_request_canceler_cancel_all();

    return CBOOL2RVAL(canceled);
}

void
rb_grn_init_request_canceler (VALUE mGrn)
{
    rb_mGrnRequestCanceler = rb_define_module_under(mGrn, "RequestCanceler");

    rb_define_singleton_method(rb_mGrnRequestCanceler, "register",
                               rb_grn_request_canceler_s_register, -1);
    rb_define_singleton_method(rb_mGrnRequestCanceler, "unregister",
                               rb_grn_request_canceler_s_unregister, -1);
    rb_define_singleton_method(rb_mGrnRequestCanceler, "cancel",
                               rb_grn_request_canceler_s_cancel, 1);
    rb_define_singleton_method(rb_mGrnRequestCanceler, "cancel_all",
                               rb_grn_request_canceler_s_cancel_all, 0);
}
