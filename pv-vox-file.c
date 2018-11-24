/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <ctype.h>

#include "pv-vox-file.h"

struct _PvVoxFile
{
    GObject       parent_instance;

    GFile        *file;

    guint8       *data;
    gsize         data_length;

    GPtrArray    *models;

    PvVoxMaterial materials[256];
};

typedef struct
{
    guint32 size_x;
    guint32 size_y;
    guint32 size_z;
    guint8 *voxels;
    guint32 voxels_length;
} VoxModel;

G_DEFINE_TYPE (PvVoxFile, pv_vox_file, G_TYPE_OBJECT)

static guint8 default_palette[1024] =
{
    0x00, 0x00, 0x00, 0x00,  0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xcc, 0xff,  0xff, 0xff, 0x99, 0xff,
    0xff, 0xff, 0x66, 0xff,  0xff, 0xff, 0x33, 0xff,  0xff, 0xff, 0x00, 0xff,  0xff, 0xcc, 0xff, 0xff,
    0xff, 0xcc, 0xcc, 0xff,  0xff, 0xcc, 0x99, 0xff,  0xff, 0xcc, 0x66, 0xff,  0xff, 0xcc, 0x33, 0xff,
    0xff, 0xcc, 0x00, 0xff,  0xff, 0x99, 0xff, 0xff,  0xff, 0x99, 0xcc, 0xff,  0xff, 0x99, 0x99, 0xff,
    0xff, 0x99, 0x66, 0xff,  0xff, 0x99, 0x33, 0xff,  0xff, 0x99, 0x00, 0xff,  0xff, 0x66, 0xff, 0xff,
    0xff, 0x66, 0xcc, 0xff,  0xff, 0x66, 0x99, 0xff,  0xff, 0x66, 0x66, 0xff,  0xff, 0x66, 0x33, 0xff,
    0xff, 0x66, 0x00, 0xff,  0xff, 0x33, 0xff, 0xff,  0xff, 0x33, 0xcc, 0xff,  0xff, 0x33, 0x99, 0xff,
    0xff, 0x33, 0x66, 0xff,  0xff, 0x33, 0x33, 0xff,  0xff, 0x33, 0x00, 0xff,  0xff, 0x00, 0xff, 0xff,
    0xff, 0x00, 0xcc, 0xff,  0xff, 0x00, 0x99, 0xff,  0xff, 0x00, 0x66, 0xff,  0xff, 0x00, 0x33, 0xff,
    0xff, 0x00, 0x00, 0xff,  0xcc, 0xff, 0xff, 0xff,  0xcc, 0xff, 0xcc, 0xff,  0xcc, 0xff, 0x99, 0xff,
    0xcc, 0xff, 0x66, 0xff,  0xcc, 0xff, 0x33, 0xff,  0xcc, 0xff, 0x00, 0xff,  0xcc, 0xcc, 0xff, 0xff,
    0xcc, 0xcc, 0xcc, 0xff,  0xcc, 0xcc, 0x99, 0xff,  0xcc, 0xcc, 0x66, 0xff,  0xcc, 0xcc, 0x33, 0xff,
    0xcc, 0xcc, 0x00, 0xff,  0xcc, 0x99, 0xff, 0xff,  0xcc, 0x99, 0xcc, 0xff,  0xcc, 0x99, 0x99, 0xff,
    0xcc, 0x99, 0x66, 0xff,  0xcc, 0x99, 0x33, 0xff,  0xcc, 0x99, 0x00, 0xff,  0xcc, 0x66, 0xff, 0xff,
    0xcc, 0x66, 0xcc, 0xff,  0xcc, 0x66, 0x99, 0xff,  0xcc, 0x66, 0x66, 0xff,  0xcc, 0x66, 0x33, 0xff,
    0xcc, 0x66, 0x00, 0xff,  0xcc, 0x33, 0xff, 0xff,  0xcc, 0x33, 0xcc, 0xff,  0xcc, 0x33, 0x99, 0xff,
    0xcc, 0x33, 0x66, 0xff,  0xcc, 0x33, 0x33, 0xff,  0xcc, 0x33, 0x00, 0xff,  0xcc, 0x00, 0xff, 0xff,
    0xcc, 0x00, 0xcc, 0xff,  0xcc, 0x00, 0x99, 0xff,  0xcc, 0x00, 0x66, 0xff,  0xcc, 0x00, 0x33, 0xff,
    0xcc, 0x00, 0x00, 0xff,  0x99, 0xff, 0xff, 0xff,  0x99, 0xff, 0xcc, 0xff,  0x99, 0xff, 0x99, 0xff,
    0x99, 0xff, 0x66, 0xff,  0x99, 0xff, 0x33, 0xff,  0x99, 0xff, 0x00, 0xff,  0x99, 0xcc, 0xff, 0xff,
    0x99, 0xcc, 0xcc, 0xff,  0x99, 0xcc, 0x99, 0xff,  0x99, 0xcc, 0x66, 0xff,  0x99, 0xcc, 0x33, 0xff,
    0x99, 0xcc, 0x00, 0xff,  0x99, 0x99, 0xff, 0xff,  0x99, 0x99, 0xcc, 0xff,  0x99, 0x99, 0x99, 0xff,
    0x99, 0x99, 0x66, 0xff,  0x99, 0x99, 0x33, 0xff,  0x99, 0x99, 0x00, 0xff,  0x99, 0x66, 0xff, 0xff,
    0x99, 0x66, 0xcc, 0xff,  0x99, 0x66, 0x99, 0xff,  0x99, 0x66, 0x66, 0xff,  0x99, 0x66, 0x33, 0xff,
    0x99, 0x66, 0x00, 0xff,  0x99, 0x33, 0xff, 0xff,  0x99, 0x33, 0xcc, 0xff,  0x99, 0x33, 0x99, 0xff,
    0x99, 0x33, 0x66, 0xff,  0x99, 0x33, 0x33, 0xff,  0x99, 0x33, 0x00, 0xff,  0x99, 0x00, 0xff, 0xff,
    0x99, 0x00, 0xcc, 0xff,  0x99, 0x00, 0x99, 0xff,  0x99, 0x00, 0x66, 0xff,  0x99, 0x00, 0x33, 0xff,
    0x99, 0x00, 0x00, 0xff,  0x66, 0xff, 0xff, 0xff,  0x66, 0xff, 0xcc, 0xff,  0x66, 0xff, 0x99, 0xff,
    0x66, 0xff, 0x66, 0xff,  0x66, 0xff, 0x33, 0xff,  0x66, 0xff, 0x00, 0xff,  0x66, 0xcc, 0xff, 0xff,
    0x66, 0xcc, 0xcc, 0xff,  0x66, 0xcc, 0x99, 0xff,  0x66, 0xcc, 0x66, 0xff,  0x66, 0xcc, 0x33, 0xff,
    0x66, 0xcc, 0x00, 0xff,  0x66, 0x99, 0xff, 0xff,  0x66, 0x99, 0xcc, 0xff,  0x66, 0x99, 0x99, 0xff,
    0x66, 0x99, 0x66, 0xff,  0x66, 0x99, 0x33, 0xff,  0x66, 0x99, 0x00, 0xff,  0x66, 0x66, 0xff, 0xff,
    0x66, 0x66, 0xcc, 0xff,  0x66, 0x66, 0x99, 0xff,  0x66, 0x66, 0x66, 0xff,  0x66, 0x66, 0x33, 0xff,
    0x66, 0x66, 0x00, 0xff,  0x66, 0x33, 0xff, 0xff,  0x66, 0x33, 0xcc, 0xff,  0x66, 0x33, 0x99, 0xff,
    0x66, 0x33, 0x66, 0xff,  0x66, 0x33, 0x33, 0xff,  0x66, 0x33, 0x00, 0xff,  0x66, 0x00, 0xff, 0xff,
    0x66, 0x00, 0xcc, 0xff,  0x66, 0x00, 0x99, 0xff,  0x66, 0x00, 0x66, 0xff,  0x66, 0x00, 0x33, 0xff,
    0x66, 0x00, 0x00, 0xff,  0x33, 0xff, 0xff, 0xff,  0x33, 0xff, 0xcc, 0xff,  0x33, 0xff, 0x99, 0xff,
    0x33, 0xff, 0x66, 0xff,  0x33, 0xff, 0x33, 0xff,  0x33, 0xff, 0x00, 0xff,  0x33, 0xcc, 0xff, 0xff,
    0x33, 0xcc, 0xcc, 0xff,  0x33, 0xcc, 0x99, 0xff,  0x33, 0xcc, 0x66, 0xff,  0x33, 0xcc, 0x33, 0xff,
    0x33, 0xcc, 0x00, 0xff,  0x33, 0x99, 0xff, 0xff,  0x33, 0x99, 0xcc, 0xff,  0x33, 0x99, 0x99, 0xff,
    0x33, 0x99, 0x66, 0xff,  0x33, 0x99, 0x33, 0xff,  0x33, 0x99, 0x00, 0xff,  0x33, 0x66, 0xff, 0xff,
    0x33, 0x66, 0xcc, 0xff,  0x33, 0x66, 0x99, 0xff,  0x33, 0x66, 0x66, 0xff,  0x33, 0x66, 0x33, 0xff,
    0x33, 0x66, 0x00, 0xff,  0x33, 0x33, 0xff, 0xff,  0x33, 0x33, 0xcc, 0xff,  0x33, 0x33, 0x99, 0xff,
    0x33, 0x33, 0x66, 0xff,  0x33, 0x33, 0x33, 0xff,  0x33, 0x33, 0x00, 0xff,  0x33, 0x00, 0xff, 0xff,
    0x33, 0x00, 0xcc, 0xff,  0x33, 0x00, 0x99, 0xff,  0x33, 0x00, 0x66, 0xff,  0x33, 0x00, 0x33, 0xff,
    0x33, 0x00, 0x00, 0xff,  0x00, 0xff, 0xff, 0xff,  0x00, 0xff, 0xcc, 0xff,  0x00, 0xff, 0x99, 0xff,
    0x00, 0xff, 0x66, 0xff,  0x00, 0xff, 0x33, 0xff,  0x00, 0xff, 0x00, 0xff,  0x00, 0xcc, 0xff, 0xff,
    0x00, 0xcc, 0xcc, 0xff,  0x00, 0xcc, 0x99, 0xff,  0x00, 0xcc, 0x66, 0xff,  0x00, 0xcc, 0x33, 0xff,
    0x00, 0xcc, 0x00, 0xff,  0x00, 0x99, 0xff, 0xff,  0x00, 0x99, 0xcc, 0xff,  0x00, 0x99, 0x99, 0xff,
    0x00, 0x99, 0x66, 0xff,  0x00, 0x99, 0x33, 0xff,  0x00, 0x99, 0x00, 0xff,  0x00, 0x66, 0xff, 0xff,
    0x00, 0x66, 0xcc, 0xff,  0x00, 0x66, 0x99, 0xff,  0x00, 0x66, 0x66, 0xff,  0x00, 0x66, 0x33, 0xff,
    0x00, 0x66, 0x00, 0xff,  0x00, 0x33, 0xff, 0xff,  0x00, 0x33, 0xcc, 0xff,  0x00, 0x33, 0x99, 0xff,
    0x00, 0x33, 0x66, 0xff,  0x00, 0x33, 0x33, 0xff,  0x00, 0x33, 0x00, 0xff,  0x00, 0x00, 0xff, 0xff,
    0x00, 0x00, 0xcc, 0xff,  0x00, 0x00, 0x99, 0xff,  0x00, 0x00, 0x66, 0xff,  0x00, 0x00, 0x33, 0xff,
    0xee, 0x00, 0x00, 0xff,  0xdd, 0x00, 0x00, 0xff,  0xbb, 0x00, 0x00, 0xff,  0xaa, 0x00, 0x00, 0xff,
    0x88, 0x00, 0x00, 0xff,  0x77, 0x00, 0x00, 0xff,  0x55, 0x00, 0x00, 0xff,  0x44, 0x00, 0x00, 0xff,
    0x22, 0x00, 0x00, 0xff,  0x11, 0x00, 0x00, 0xff,  0x00, 0xee, 0x00, 0xff,  0x00, 0xdd, 0x00, 0xff,
    0x00, 0xbb, 0x00, 0xff,  0x00, 0xaa, 0x00, 0xff,  0x00, 0x88, 0x00, 0xff,  0x00, 0x77, 0x00, 0xff,
    0x00, 0x55, 0x00, 0xff,  0x00, 0x44, 0x00, 0xff,  0x00, 0x22, 0x00, 0xff,  0x00, 0x11, 0x00, 0xff,
    0x00, 0x00, 0xee, 0xff,  0x00, 0x00, 0xdd, 0xff,  0x00, 0x00, 0xbb, 0xff,  0x00, 0x00, 0xaa, 0xff,
    0x00, 0x00, 0x88, 0xff,  0x00, 0x00, 0x77, 0xff,  0x00, 0x00, 0x55, 0xff,  0x00, 0x00, 0x44, 0xff,
    0x00, 0x00, 0x22, 0xff,  0x00, 0x00, 0x11, 0xff,  0xee, 0xee, 0xee, 0xff,  0xdd, 0xdd, 0xdd, 0xff,
    0xbb, 0xbb, 0xbb, 0xff,  0xaa, 0xaa, 0xaa, 0xff,  0x88, 0x88, 0x88, 0xff,  0x77, 0x77, 0x77, 0xff,
    0x55, 0x55, 0x55, 0xff,  0x44, 0x44, 0x44, 0xff,  0x22, 0x22, 0x22, 0xff,  0x11, 0x11, 0x11, 0xff
};

static guint32
id_to_uint (const gchar *id)
{
    if (strlen (id) != 4)
        return 0;
    return id[3] << 24 | id[2] << 16 | id[1] << 8 | id[0];
}

static gchar
id_char (guint8 value)
{
    gchar c = value;
    return isprint (c) ? c : '?';
}

static gchar *
uint_to_id (guint32 value)
{
    gchar id[5] = {
        id_char ((value >>  0) & 0xFF),
        id_char ((value >>  8) & 0xFF),
        id_char ((value >> 16) & 0xFF),
        id_char ((value >> 24) & 0xFF),
        '\0',
    };
    return g_strdup (id);
}

static guint32
read_uint (guint8 *data,
           gsize   data_length,
           gsize   offset)
{
    if (offset + 4 > data_length)
        return 0;

    return data[offset + 3] << 24 |
           data[offset + 2] << 16 |
           data[offset + 1] << 8 |
           data[offset + 0];
}

static gfloat
read_float (guint8 *data,
            gsize   data_length,
            gsize   offset)
{
    guint32 value = read_uint (data, data_length, offset);
    return *((gfloat *) &value);
}

static gchar *
read_string (guint8 *data,
             gsize   data_length,
             gsize  *offset)
{
    if (*offset + 4 > data_length)
        return g_strdup ("");

    guint32 length = read_uint (data, data_length, *offset);
    *offset += 4;

    if (*offset + length > data_length) {
       *offset = data_length;
       return g_strdup ("");
    }

    gchar *value = g_malloc (length + 1);
    memcpy (value, data + *offset, length);
    value[length] = '\0';
    *offset += length;

    return value;
}

static void
pv_vox_file_dispose (GObject *object)
{
    PvVoxFile *self = PV_VOX_FILE (object);

    g_clear_object (&self->file);
    g_clear_pointer (&self->data, g_free);
    g_clear_pointer (&self->models, g_ptr_array_unref);
    for (int i = 0; i < 256; i++)
        g_clear_pointer (&self->materials[i].properties, g_hash_table_unref);

    G_OBJECT_CLASS (pv_vox_file_parent_class)->dispose (object);
}

void
pv_vox_file_class_init (PvVoxFileClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = pv_vox_file_dispose;
}

void
pv_vox_file_init (PvVoxFile *self)
{
    self->models = g_ptr_array_new_with_free_func (g_free);
    for (int i = 0; i < 256; i++) {
        guint8 *color = default_palette + i * 4;
        self->materials[i].r = color[0];
        self->materials[i].g = color[1];
        self->materials[i].b = color[2];
        self->materials[i].a = color[3];
        self->materials[i].properties = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    }
}

PvVoxFile *
pv_vox_file_new (GFile *file)
{
    PvVoxFile *self;

    self = g_object_new (pv_vox_file_get_type (), NULL);

    self->file = g_object_ref (file);

    return self;
}

static gboolean
decode_main_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   GError   **error)
{
    // Contains no data
    return TRUE;
}

static gboolean
decode_pack_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   GError   **error)
{
    if (chunk_length < 4)
        return TRUE;

    //guint32 n_models = read_uint (chunk_start, chunk_length, 0);

    return TRUE;
}

static gboolean
decode_size_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   GError   **error)
{
   if (chunk_length < 12)
       return TRUE;

    VoxModel *model = g_new0 (VoxModel, 1);
    model->size_x = read_uint (chunk_start, chunk_length, 0);
    model->size_y = read_uint (chunk_start, chunk_length, 4);
    model->size_z = read_uint (chunk_start, chunk_length, 8);
    g_ptr_array_add (self->models, model);

    return TRUE;
}

static gboolean
decode_xyzi_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   GError   **error)
{
    if (chunk_length < 4)
        return TRUE;

    guint32 voxels_length = read_uint (chunk_start, chunk_length, 0);
    if (4 + voxels_length * 4 > chunk_length) {
        guint32 max_voxels_length = (chunk_length / 4) - 4;
        g_warning ("XYZI block specified %u voxels but only space for %u", voxels_length, max_voxels_length);
        voxels_length = max_voxels_length;
    }

    VoxModel *model = self->models->len > 0 ? g_ptr_array_index (self->models, self->models->len - 1) : NULL;
    if (model == NULL) {
        g_warning ("Ignoring XYZI block without preceeding SIZE block");
    }
    else if (model->voxels != NULL) {
        g_warning ("Ignoring duplicate XYZI block");
    }
    else {
        model->voxels = chunk_start + 4;
        model->voxels_length = voxels_length;
    }

    return TRUE;
}

static gboolean
decode_rgba_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   GError   **error)
{
    g_printerr ("RGBA %zu\n", chunk_length);
    for (int offset = 0, index = 1; offset < chunk_length && index < 256; offset += 4, index++) {
        self->materials[index].r = chunk_start[offset + 0];
        self->materials[index].g = chunk_start[offset + 1];
        self->materials[index].b = chunk_start[offset + 2];
        self->materials[index].a = chunk_start[offset + 3];
    }

    return TRUE;
}

static gboolean
decode_matt_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   GError   **error)
{
    if (chunk_length < 16)
        return TRUE;

    guint32 id = read_uint (chunk_start, chunk_length, 0);
    // FIXME: Check id between 1 and 255
    PvVoxMaterial null_material;
    PvVoxMaterial *material = id < 256 ? &self->materials[id] : &null_material;
    guint32 type = read_uint (chunk_start, chunk_length, 4);
    switch (type) {
    case 0:
        g_hash_table_insert (material->properties, g_strdup ("_type"), g_strdup ("_diffuse"));
        break;
    case 1:
        g_hash_table_insert (material->properties, g_strdup ("_type"), g_strdup ("_metal"));
        break;
    case 2:
        g_hash_table_insert (material->properties, g_strdup ("_type"), g_strdup ("_glass"));
        break;
    case 3:
        g_hash_table_insert (material->properties, g_strdup ("_type"), g_strdup ("_emissive"));
        break;
    }
    gfloat weight = read_float (chunk_start, chunk_length, 8);
    g_hash_table_insert (material->properties, g_strdup ("_weight"), g_strdup_printf ("%f", weight));
    guint32 property_bits = read_uint (chunk_start, chunk_length, 12);
    gsize property_offset = 0;
    if (property_bits & 0x00000001) {
        gfloat plastic = read_float (chunk_start, chunk_length, property_offset);
        g_hash_table_insert (material->properties, g_strdup ("_plastic"), g_strdup_printf ("%f", plastic));
        property_offset += 4;
    }
    if (property_bits & 0x00000002) {
        gfloat rough = read_float (chunk_start, chunk_length, property_offset);
        g_hash_table_insert (material->properties, g_strdup ("_rough"), g_strdup_printf ("%f", rough));
        property_offset += 4;
    }
    if (property_bits & 0x00000004) {
        gfloat spec = read_float (chunk_start, chunk_length, property_offset);
        g_hash_table_insert (material->properties, g_strdup ("_spec"), g_strdup_printf ("%f", spec));
        property_offset += 4;
    }
    if (property_bits & 0x00000008) {
        gfloat ior = read_float (chunk_start, chunk_length, property_offset);
        g_hash_table_insert (material->properties, g_strdup ("_ior"), g_strdup_printf ("%f", ior));
        property_offset += 4;
    }
    if (property_bits & 0x00000010) {
        gfloat att = read_float (chunk_start, chunk_length, property_offset);
        g_hash_table_insert (material->properties, g_strdup ("_att"), g_strdup_printf ("%f", att));
        property_offset += 4;
    }
    if (property_bits & 0x00000020) {
        //material->power = read_float (chunk_start, chunk_length, property_offset);
        property_offset += 4;
    }
    if (property_bits & 0x00000040) {
        gfloat glow = read_float (chunk_start, chunk_length, property_offset);
        g_hash_table_insert (material->properties, g_strdup ("_glow"), g_strdup_printf ("%f", glow));
        property_offset += 4;
    }
    if (property_bits & 0x00000080) {
        //material->is_total_power = TRUE;
    }

    return TRUE;
}

static gboolean
decode_matl_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   GError   **error)
{
    if (chunk_length < 8)
        return TRUE;

    guint32 id = read_uint (chunk_start, chunk_length, 0);
    guint32 property_count = read_uint (chunk_start, chunk_length, 4);

    PvVoxMaterial null_material;
    PvVoxMaterial *material = id < 256 ? &self->materials[id] : &null_material;

    gsize offset = 8;
    for (guint32 i = 0; i < property_count; i++) {
        gchar *name = read_string (chunk_start, chunk_length, &offset);
        gchar *value = read_string (chunk_start, chunk_length, &offset);
        g_hash_table_insert (material->properties, name, value);
    }

    return TRUE;
}

static gboolean
decode_chunks (PvVoxFile *self,
               guint8    *data,
               gsize      data_length,
               GError   **error)
{
    gsize offset = 0;
    while (offset < data_length) {
        gsize n_remaining = data_length - offset;

        /* Check enough space for header */
        guint8 *chunk_header = data + offset;
        gsize chunk_header_length = 12;
        if (n_remaining < chunk_header_length) {
            g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "Not enough space for chunk header");
            return FALSE;
        }

        /* Read header, check enough space for data */
        guint32 chunk_id = read_uint (chunk_header, n_remaining, 0);
        guint32 chunk_length = read_uint (chunk_header, n_remaining, 4);
        guint32 child_chunks_length = read_uint (chunk_header, n_remaining, 8);

        g_autofree gchar *id_string = uint_to_id (chunk_id);

        gsize n_required = chunk_header_length + chunk_length + child_chunks_length;
        if (n_required > n_remaining) {
            g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                         "Chunk %s requires %zi octets, but only %zi available", id_string, n_required, n_remaining);
            return FALSE;
        }

        guint8 *chunk_start = data + offset + chunk_header_length;
        guint8 *child_chunks_start = data + offset + chunk_header_length + chunk_length;

        gboolean (*decode_func)(PvVoxFile *, guint8 *, gsize, GError **) = NULL;
        if (chunk_id == id_to_uint ("MAIN"))
            decode_func = decode_main_chunk;
        else if (chunk_id == id_to_uint ("PACK"))
            decode_func = decode_pack_chunk;
        else if (chunk_id == id_to_uint ("SIZE"))
            decode_func = decode_size_chunk;
        else if (chunk_id == id_to_uint ("XYZI"))
            decode_func = decode_xyzi_chunk;
        else if (chunk_id == id_to_uint ("RGBA"))
            decode_func = decode_rgba_chunk;
        else if (chunk_id == id_to_uint ("MATT"))
            decode_func = decode_matt_chunk;
        else if (chunk_id == id_to_uint ("MATL"))
            decode_func = decode_matl_chunk;
        else
            g_debug ("Ignoring unknown MagicaVoxel chunk %s", id_string);
        if (decode_func != NULL && !decode_func (self, chunk_start, chunk_length, error))
            return FALSE;

        /* Decode child chunks */
        if (child_chunks_length > 0)
            if (!decode_chunks (self, child_chunks_start, child_chunks_length, error))
                return FALSE;

        offset += chunk_header_length + chunk_length + child_chunks_length;
    }

    return TRUE;
}

gboolean
pv_vox_file_decode (PvVoxFile    *self,
                    GCancellable *cancellable,
                    GError      **error)
{
    g_return_val_if_fail (PV_IS_VOX_FILE (self), FALSE);

    if (!g_file_load_contents (self->file, cancellable, (gchar **)&self->data, &self->data_length, NULL, error))
        return FALSE;

    if (self->data_length < 8) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "Not a MagicaVoxel file");
        return FALSE;
    }

    guint32 id = read_uint (self->data, self->data_length, 0);
    if (id != id_to_uint ("VOX ")) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "Not a MagicaVoxel file");
        return FALSE;
    }
    guint32 version = read_uint (self->data, self->data_length, 4);
    g_printerr ("version: %u\n", version);

    return decode_chunks (self, self->data + 8, self->data_length - 8, error);
}

void
pv_vox_file_get_size (PvVoxFile *self,
                      guint32   *size_x,
                      guint32   *size_y,
                      guint32   *size_z)
{
    g_return_if_fail (PV_IS_VOX_FILE (self));
    g_return_if_fail (self->models->len > 0);

    VoxModel *model = g_ptr_array_index (self->models, 0);

    if (size_x != NULL)
        *size_x = model->size_x;
    if (size_y != NULL)
        *size_y = model->size_y;
    if (size_z != NULL)
        *size_z = model->size_z;
}

guint32
pv_vox_file_get_model_count (PvVoxFile *self)
{
    g_return_val_if_fail (PV_IS_VOX_FILE (self), 0);
    return self->models->len;
}

guint32
pv_vox_file_get_voxel_count (PvVoxFile *self,
                             guint32    model_index)
{
    g_return_val_if_fail (PV_IS_VOX_FILE (self), 0);
    g_return_val_if_fail (model_index < self->models->len, 0);

    VoxModel *model = g_ptr_array_index (self->models, model_index);
    return model->voxels_length;
}

void
pv_vox_file_get_voxel (PvVoxFile *self,
                       guint32    model_index,
                       guint32    voxel_index,
                       guint8    *x,
                       guint8    *y,
                       guint8    *z,
                       guint8    *color_index)
{
    g_return_if_fail (PV_IS_VOX_FILE (self));
    g_return_if_fail (model_index < self->models->len);

    VoxModel *model = g_ptr_array_index (self->models, model_index);
    g_return_if_fail (voxel_index < model->voxels_length);

    guint8 *voxel = model->voxels + voxel_index * 4;
    if (x != NULL)
        *x = voxel[0];
    if (y != NULL)
        *y = voxel[1];
    if (z != NULL)
        *z = voxel[2];
    if (color_index != NULL)
        *color_index = voxel[3];
}

PvVoxMaterial *
pv_vox_file_get_material (PvVoxFile *self,
                          guint8     index)
{
    g_return_val_if_fail (PV_IS_VOX_FILE (self), NULL);
    return &self->materials[index];
}
