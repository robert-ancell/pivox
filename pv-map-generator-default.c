/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "pv-map-generator-default.h"

struct _PvMapGeneratorDefault
{
    PvMapGenerator parent_instance;
};

G_DEFINE_TYPE (PvMapGeneratorDefault, pv_map_generator_default, pv_map_generator_get_type ())

static PvMap *
pv_map_generator_default_generate (PvMapGenerator *generator)
{
    PvMap *map = pv_map_new ();
    pv_map_set_width (map, 16);
    pv_map_set_height (map, 16);
    pv_map_set_depth (map, 16);
    pv_map_set_name (map, "Default Map");
    pv_map_set_description (map, "Default generated map");

    guint air =   pv_map_add_block (map, "Air",     0,   0,   0);
    guint rock =  pv_map_add_block (map, "Rock",  136, 138, 133);
    guint dirt =  pv_map_add_block (map, "Dirt",  233, 185, 110);
    guint grass = pv_map_add_block (map, "Grass", 138, 226,  52);

    guint8 blocks[16 * 16 * 16];
    for (guint x = 0; x < 16; x++)
        for (guint y = 0; y < 16; y++)
            for (guint z = 0; z < 16; z++) {
                guint block_id;
                if (z < 4)
                    block_id = rock;
                else if (z < 7)
                    block_id = dirt;
                else if (z < 8)
                    block_id = grass;
                else
                    block_id = air;
                gsize index = ((z * 16) + y) * 16 + x;
                blocks[index] = block_id;
            }
    pv_map_add_area_raster8 (map,
                             0, 0, 0,
                             16, 16, 16,
                             blocks);

    return map;
}

void
pv_map_generator_default_class_init (PvMapGeneratorDefaultClass *klass)
{
    PvMapGeneratorClass *generator_class = PV_MAP_GENERATOR_CLASS (klass);

    generator_class->generate = pv_map_generator_default_generate;
}

void
pv_map_generator_default_init (PvMapGeneratorDefault *self)
{
}

PvMapGeneratorDefault *
pv_map_generator_default_new (void)
{
    return g_object_new (pv_map_generator_default_get_type (), NULL);
}
