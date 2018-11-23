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

typedef enum 
{
    PV_VOX_MATERIAL_TYPE_DIFFUSE  = 0,
    PV_VOX_MATERIAL_TYPE_METAL    = 1,
    PV_VOX_MATERIAL_TYPE_GLASS    = 2,
    PV_VOX_MATERIAL_TYPE_EMISSIVE = 3
} PvVoxMaterialType;

typedef struct
{
   guint8 r;
   guint8 g;
   guint8 b;
   guint8 a;

   PvVoxMaterialType type;

   gfloat weight;
   gfloat plastic;
   gfloat roughness;
   gfloat specular;
   gfloat ior;
   gfloat attenuation;
   gfloat power;
   gfloat glow;
   gboolean is_total_power;
} PvVoxMaterial;

PvVoxFile     *pv_vox_file_new             (GFile        *file);

gboolean       pv_vox_file_decode          (PvVoxFile    *file,
                                            GCancellable *cancellable,
                                            GError      **error);

void           pv_vox_file_get_size        (PvVoxFile    *file,
                                            guint32      *size_x,
                                            guint32      *size_y,
                                            guint32      *size_z);

guint32        pv_vox_file_get_voxel_count (PvVoxFile    *file);

void           pv_vox_file_get_voxel       (PvVoxFile    *file,
                                            guint32       index,
                                            guint8       *x,
                                            guint8       *y,
                                            guint8       *z,
                                            guint8       *color_index);

PvVoxMaterial *pv_vox_file_get_material    (PvVoxFile    *file,
                                            guint8        index);
