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

#include "pv-map-generator.h"

G_DECLARE_FINAL_TYPE (PvMapGeneratorDefault, pv_map_generator_default, PV, MAP_GENERATOR_DEFAULT, PvMapGenerator)

PvMapGeneratorDefault *pv_map_generator_default_new (void);
