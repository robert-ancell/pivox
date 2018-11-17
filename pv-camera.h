/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#pragma once

#include <glib-object.h>

G_DECLARE_FINAL_TYPE (PvCamera, pv_camera, PV, CAMERA, GObject)

PvCamera *pv_camera_new           (void);

void      pv_camera_set_position  (PvCamera *camera,
                                   gfloat    x,
                                   gfloat    y,
                                   gfloat    z);

void      pv_camera_get_position  (PvCamera *camera,
                                   gfloat   *x,
                                   gfloat   *y,
                                   gfloat   *z);

void      pv_camera_set_direction (PvCamera *camera,
                                   gfloat    x,
                                   gfloat    y,
                                   gfloat    z);

void      pv_camera_set_target    (PvCamera *camera,
                                   gfloat    x,
                                   gfloat    y,
                                   gfloat    z);

void      pv_camera_transform     (PvCamera *camera,
                                   gint      width,
                                   gint      height,
                                   gint      uniform_location);
