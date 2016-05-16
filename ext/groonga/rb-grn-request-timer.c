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
 * Document-module: Groonga::RequestTimer
 *
 * This module provides API for canceling requests after the specified
 * time.
 */

VALUE rb_mGrnRequestTimer;

/*
 * Registers a request with timeout. If the request isn't finished
 * after the specified timeout, the request is canceled.
 *
 * @example Register a request that will be timed out after 2.9 seconds
 *   request_id = "request-29"
 *   timeout_in_second = 2.9
 *   Groonga::RequestTimer.register(request_id, timeout_in_second)
 *
 * @overload register(request_id, timeout=nil)
 *   @param request_id [String] The ID of request to be registered.
 *   @param timeout (nil) [Float, nil] The timeout in second. If
 *      `timeout` is `nil`, {Groonga::RequestTimer.default_timeout} is
 *      used.
 *   @return [Groonga::RequestTimerID] The ID of the request timer.
 *
 * @since 6.0.2
 */
static VALUE
rb_grn_request_timer_s_register (int argc, VALUE *argv, VALUE module)
{
    VALUE rb_request_id;
    VALUE rb_timeout;
    const char *request_id;
    unsigned int request_id_size;
    double timeout;
    void *timer_id;

    rb_scan_args(argc, argv, "11", &rb_request_id, &rb_timeout);

    request_id = StringValuePtr(rb_request_id);
    request_id_size = RSTRING_LEN(rb_request_id);
    if (NIL_P(rb_timeout)) {
      timeout = grn_get_default_request_timeout();
    } else {
      timeout = NUM2DBL(rb_timeout);
    }
    timer_id = grn_request_timer_register(request_id, request_id_size, timeout);

    return GRN_REQUEST_TIMER_ID2RVAL(timer_id);
}

/*
 * Unregisters the specified request timer.
 *
 * @example Unregister a request timer by ID
 *   timer_id = Groonga::RequestTimer.register("request-29", 2.9)
 *   Groonga::RequestTimer.unregister(timer_id)
 *
 * @overload unregister(timer_id)
 *   @param timer_id [Groonga::RequestTimerID] The ID of request timer
 *      to be unregistered.
 *   @return [void]
 *
 * @since 6.0.2
 */
static VALUE
rb_grn_request_timer_s_unregister (VALUE module, VALUE rb_timer_id)
{
    void *timer_id;

    timer_id = RVAL2GRN_REQUEST_TIMER_ID(rb_timer_id);
    grn_request_timer_unregister(timer_id);

    return Qnil;
}

/*
 * Gets the default timeout used by request timer.
 *
 * @example Gets the default timeout
 *   Groonga::RequestTimer.default_timeout
 *
 * @overload default_timeout
 *   @return [Float] The default timeout used by request timer.
 *
 * @since 6.0.2
 */
static VALUE
rb_grn_request_timer_s_get_default_timeout (VALUE module)
{
    double timeout;

    timeout = grn_get_default_request_timeout();

    return rb_float_new(timeout);
}

/*
 * Sets the default timeout used by request timer.
 *
 * @example Sets the default timeout
 *   Groonga::RequestTimer.default_timeout = 2.9
 *
 * @overload default_timeout=(timeout)
 *   @return [Float] The default timeout used by request timer. If
 *     `timeout` is `0.0`, the default timeout is disabled.
 *   @return [void]
 *
 * @since 6.0.2
 */
static VALUE
rb_grn_request_timer_s_set_default_timeout (VALUE module, VALUE rb_timeout)
{
    double timeout;

    timeout = NUM2DBL(rb_timeout);
    grn_set_default_request_timeout(timeout);

    return Qnil;
}

void
rb_grn_init_request_timer (VALUE mGrn)
{
    rb_mGrnRequestTimer = rb_define_module_under(mGrn, "RequestTimer");

    rb_define_singleton_method(rb_mGrnRequestTimer, "register",
                               rb_grn_request_timer_s_register, -1);
    rb_define_singleton_method(rb_mGrnRequestTimer, "unregister",
                               rb_grn_request_timer_s_unregister, 1);
    rb_define_singleton_method(rb_mGrnRequestTimer, "default_timeout",
                               rb_grn_request_timer_s_get_default_timeout, 0);
    rb_define_singleton_method(rb_mGrnRequestTimer, "default_timeout=",
                               rb_grn_request_timer_s_set_default_timeout, 1);
}
