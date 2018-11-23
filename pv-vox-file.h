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

G_DECLARE_FINAL_TYPE (PvVoxFile, pv_vox_file, PV, VOX_FILE, GObject)

PvVoxFile *pv_vox_file_new             (GFile        *file);

gboolean   pv_vox_file_decode          (PvVoxFile    *file,
                                        GCancellable *cancellable,
                                        GError      **error);

void       pv_vox_file_get_size        (PvVoxFile    *file,
                                        guint32      *size_x,
                                        guint32      *size_y,
                                        guint32      *size_z);

guint32    pv_vox_file_get_voxel_count (PvVoxFile    *file);

void       pv_vox_file_get_voxel       (PvVoxFile    *file,
                                        guint32       index,
                                        guint8       *x,
                                        guint8       *y,
                                        guint8       *z,
                                        guint8       *color_index);

void       pv_vox_file_get_color       (PvVoxFile    *file,
                                        guint8        index,
                                        guint8       *r,
                                        guint8       *g,
                                        guint8       *b,
                                        guint8       *a);
