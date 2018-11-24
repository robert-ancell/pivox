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

#include "pv-block-type.h"

G_DECLARE_FINAL_TYPE (PvMap, pv_map, PV, MAP, GObject)

PvMap       *pv_map_new            (guint        width,
                                    guint        height,
                                    guint        depth);

guint        pv_map_get_width      (PvMap       *map);

guint        pv_map_get_height     (PvMap       *map);

guint        pv_map_get_depth      (PvMap       *map);

void         pv_map_add_block_type (PvMap       *map,
                                    PvBlockType *block_type);

PvBlockType *pv_map_get_block_type (PvMap       *map,
                                    const gchar *name);

void         pv_map_set_block      (PvMap       *map,
                                    guint        x,
                                    guint        y,
                                    guint        z,
                                    PvBlockType *type);

PvBlockType *pv_map_get_block      (PvMap       *map,
                                    guint        x,
                                    guint        y,
                                    guint        z);
