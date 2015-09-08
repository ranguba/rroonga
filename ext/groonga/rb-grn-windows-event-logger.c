/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2015  Kouhei Sutou <kou@clear-code.com>

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
 * Document-class: Groonga::WindowsEventLogger
 *
 * It's a module for using Windows Event Log as log output.
 */

/*
 * @overload register(event_source_name, options={})
 *
 *   Registers Windows Event Log based logger that uses
 *   `event_source_name` as event source name.
 *
 *   @param event_source_name [String] The event source name.
 *   @param options [::Hash]
 *   @option options :context [Groonga::Context] (Groonga::Context.default)
 *     The context to be set logger.
 *
 *   @return [void]
 *
 *   @since 5.0.5
 */
static VALUE
rb_grn_windows_event_logger_s_register (int argc,
                                        VALUE *argv,
                                        VALUE klass)
{
    VALUE rb_event_source_name;
    VALUE rb_options;
    VALUE rb_context;
    const char *event_source_name;
    grn_ctx *context;
    grn_rc rc;

    rb_scan_args(argc, argv, "11", &rb_event_source_name, &rb_options);
    rb_event_source_name = rb_grn_convert_to_string(rb_event_source_name);
    event_source_name = StringValueCStr(rb_event_source_name);

    rb_grn_scan_options(rb_options,
                        "context", &rb_context,
                        NULL);
    context = rb_grn_context_ensure(&rb_context);

    rc = grn_windows_event_logger_set(context, event_source_name);
    rb_grn_context_check(context, rb_event_source_name);
    rb_grn_rc_check(rc, rb_event_source_name);

    return Qnil;
}

void
rb_grn_init_windows_event_logger (VALUE mGrn)
{
    VALUE mGrnWindowsEventLogger;

    mGrnWindowsEventLogger = rb_define_module_under(mGrn, "WindowsEventLogger");

    rb_define_singleton_method(mGrnWindowsEventLogger, "register",
                               rb_grn_windows_event_logger_s_register, -1);
}
