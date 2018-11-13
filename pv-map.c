/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "pv-map.h"

struct _PvMap
{
    GObject parent_instance;
};

G_DEFINE_TYPE (PvMap, pv_map, G_TYPE_OBJECT)

void
pv_map_class_init (PvMapClass *klass)
{
}

void
pv_map_init (PvMap *self)
{
}

PvMap *
pv_map_new (void)
{
    return g_object_new (pv_map_get_type (), NULL);
}
