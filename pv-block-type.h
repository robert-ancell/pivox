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

G_DECLARE_FINAL_TYPE (PvBlockType, pv_block_type, PV, BLOCK_TYPE, GObject)

PvBlockType *pv_block_type_new       (const gchar *name);

void         pv_block_type_set_color (PvBlockType *type,
                                      gdouble      r,
                                      gdouble      g,
                                      gdouble      b);
