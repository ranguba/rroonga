/* -*- coding: utf-8; mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2012  Kouhei Sutou <kou@clear-code.com>

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

VALUE rb_cGrnTokyoGeoPoint;
VALUE rb_cGrnWGS84GeoPoint;

VALUE
rb_grn_tokyo_geo_point_new (int latitude, int longitude)
{
    return rb_grn_tokyo_geo_point_new_raw(INT2NUM(latitude), INT2NUM(longitude));
}

VALUE
rb_grn_wgs84_geo_point_new (int latitude, int longitude)
{
    return rb_grn_wgs84_geo_point_new_raw(INT2NUM(latitude), INT2NUM(longitude));
}

VALUE
rb_grn_tokyo_geo_point_new_raw (VALUE latitude, VALUE longitude)
{
    return rb_funcall(rb_cGrnTokyoGeoPoint, rb_intern("new"), 2,
                      latitude, longitude);
}

VALUE
rb_grn_wgs84_geo_point_new_raw (VALUE latitude, VALUE longitude)
{
    return rb_funcall(rb_cGrnWGS84GeoPoint, rb_intern("new"), 2,
                      latitude, longitude);
}

void
rb_grn_init_geo_point (VALUE mGrn)
{
    rb_cGrnTokyoGeoPoint = rb_const_get(mGrn, rb_intern("TokyoGeoPoint"));
    rb_cGrnWGS84GeoPoint = rb_const_get(mGrn, rb_intern("WGS84GeoPoint"));
}
