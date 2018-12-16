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

#include "pv-map.h"

G_DECLARE_DERIVABLE_TYPE (PvMapGenerator, pv_map_generator, PV, MAP_GENERATOR, GObject)

struct _PvMapGeneratorClass
{
    GObjectClass parent_class;

    PvMap *(*generate)(PvMapGenerator *self);
};

PvMap *pv_map_generator_generate (PvMapGenerator *generator);
