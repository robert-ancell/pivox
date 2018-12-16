/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "pv-map-generator.h"

G_DEFINE_TYPE (PvMapGenerator, pv_map_generator, G_TYPE_OBJECT)

void
pv_map_generator_class_init (PvMapGeneratorClass *klass)
{
}

void
pv_map_generator_init (PvMapGenerator *self)
{
}

PvMapGenerator *
pv_map_generator_new (void)
{
    return g_object_new (pv_map_generator_get_type (), NULL);
}

PvMap *
pv_map_generator_generate (PvMapGenerator *self)
{
    g_return_val_if_fail (PV_IS_MAP_GENERATOR (self), NULL);

    return PV_MAP_GENERATOR_GET_CLASS (self)->generate (self);
}
