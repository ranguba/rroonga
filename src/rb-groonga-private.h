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

#ifndef __RB_GROONGA_PRIVATE_H__
#define __RB_GROONGA_PRIVATE_H__

#include <ruby.h>

#include <groonga.h>

#if defined(__cplusplus)
#  define RB_GROONGA_BEGIN_DECLS extern "C" {
#  define RB_GROONGA_END_DECLS }
#else
#  define RB_GROONGA_BEGIN_DECLS
#  define RB_GROONGA_END_DECLS
#endif

RB_GROONGA_BEGIN_DECLS

#if __GNUC__ >= 4
#  define RB_GROONGA_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#else
#  define RB_GROONGA_GNUC_NULL_TERMINATED
#endif

#if defined(RUBY_GROONGA_PLATFORM_WIN32) && !defined(RB_GROONGA_PLATFORM_WIN32)
#  define RB_GROONGA_PLATFORM_WIN32 RUBY_GROONGA_PLATFORM_WIN32
#endif

#if defined(RUBY_GROONGA_STATIC_COMPILATION) && !defined(RB_GROONGA_STATIC_COMPILATION)
#  define RB_GROONGA_STATIC_COMPILATION RUBY_GROONGA_STATIC_COMPILATION
#endif

#if defined(RB_GROONGA_PLATFORM_WIN32) && !defined(RB_GROONGA_STATIC_COMPILATION)
#  ifdef RB_GROONGA_COMPILATION
#    define RB_GROONGA_VAR __declspec(dllexport)
#  else
#    define RB_GROONGA_VAR extern __declspec(dllimport)
#  endif
#else
#  define RB_GROONGA_VAR extern
#endif

RB_GROONGA_VAR VALUE rb_eGrnError;

void           rb_grn_init_utils                    (VALUE mGroonga);
void           rb_grn_init_exception                (VALUE mGroonga);
void           rb_grn_init_encoding                 (VALUE mGroonga);
void           rb_grn_init_context                  (VALUE mGroonga);

VALUE          rb_grn_rc_to_exception               (grn_rc rc);
const char    *rb_grn_rc_to_message                 (grn_rc rc);
void           rb_grn_check_rc                      (grn_rc rc);

void           rb_grn_context_check                 (grn_ctx *context);

const char    *rb_grn_inspect                       (VALUE object);
void           rb_grn_scan_options                  (VALUE options, ...)
                                                     RB_GROONGA_GNUC_NULL_TERMINATED;

#define RB_GRN_INTERN(string)         (ID2SYM(rb_intern(string)))

#define RVAL2CBOOL(object)            (RTEST(object))
#define CBOOL2RVAL(boolean)           ((boolean) ? Qtrue : Qfalse)

#define RVAL2GRNCONTEXT(object)       (rb_grn_context_from_ruby_object(object))
#define GRNCONTEXT2RVAL(context)      (rb_grn_context_to_ruby_object(context))

#define RVAL2GRNENCODING(object)      (rb_grn_encoding_from_ruby_object(object))
#define GRNENCODING2RVAL(encoding)    (rb_grn_encoding_to_ruby_object(encoding))

grn_ctx       *rb_grn_context_from_ruby_object      (VALUE object);
VALUE          rb_grn_context_to_ruby_object        (grn_ctx *context);

grn_encoding   rb_grn_encoding_from_ruby_object     (VALUE object);
VALUE          rb_grn_encoding_to_ruby_object       (grn_encoding encoding);



RB_GROONGA_END_DECLS

#endif
