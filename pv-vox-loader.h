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

#include <gio/gio.h>

#include "pv-map.h"

G_DECLARE_FINAL_TYPE (PvVoxLoader, pv_vox_loader, PV, VOX_LOADER, GObject)

PvVoxLoader *pv_vox_loader_new    (GFile        *file);

PvMap       *pv_vox_loader_decode (PvVoxLoader  *loader,
                                   GCancellable *cancellable,
                                   GError      **error);
