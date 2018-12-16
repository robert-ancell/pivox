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
    PvMapGenerator *self = PV_MAP_GENERATOR (generator);
    return NULL;
}

static void
pv_map_generator_default_dispose (GObject *object)
{
    //PvMapGeneratorDefault *self = PV_MAP_GENERATOR_DEFAULT (object);

    G_OBJECT_CLASS (pv_map_generator_default_parent_class)->dispose (object);
}

void
pv_map_generator_default_class_init (PvMapGeneratorDefaultClass *klass)
{
    PvMapGeneratorClass *generator_class = PV_MAP_GENERATOR_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    generator_class->generate = pv_map_generator_default_generate;
    object_class->dispose = pv_map_generator_default_dispose;
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
