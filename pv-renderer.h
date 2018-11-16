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

#include "pv-map.h"

G_DECLARE_FINAL_TYPE (PvRenderer, pv_renderer, PV, RENDERER, GObject)

PvRenderer *pv_renderer_new        (void);

void        pv_renderer_set_map    (PvRenderer *renderer,
                                    PvMap      *map);

void        pv_renderer_set_camera (PvRenderer *renderer,
                                    gfloat      x,
                                    gfloat      y,
                                    gfloat      z,
                                    gfloat      target_x,
                                    gfloat      target_y,
                                    gfloat      target_z);

void        pv_renderer_render     (PvRenderer *renderer,
                                    guint       width,
                                    guint       height);
