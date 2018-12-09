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

    guint32       version;
    guint8       *data;
    gsize         data_length;

    GPtrArray    *nodes;
    GPtrArray    *groups;
    GPtrArray    *shapes;
    GPtrArray    *layers;
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

static void
pv_vox_node_free (PvVoxNode *node)
{
    g_clear_pointer (&node->attributes, g_hash_table_unref);
    g_free (node);
}

static void
pv_vox_node_group_free (PvVoxNodeGroup *group)
{
    g_clear_pointer (&group->attributes, g_hash_table_unref);
    g_free (group->nodes);
    g_free (group);
}

static void
pv_vox_node_shape_free (PvVoxNodeShape *shape)
{
    g_clear_pointer (&shape->attributes, g_hash_table_unref);
    g_free (shape->models);
    for (guint32 i = 0; i < shape->models_length; i++)
        g_clear_pointer (&shape->model_attributes[i], g_hash_table_unref);
    g_free (shape->model_attributes);
    g_free (shape);
}

static void
pv_vox_layer_free (PvVoxLayer *layer)
{
    g_clear_pointer (&layer->attributes, g_hash_table_unref);
    g_free (layer);
}

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

static gboolean
read_uint (guint8  *data,
           gsize    data_length,
           gsize   *offset,
           guint32 *value,
           GError **error)
{
    if (*offset + 4 > data_length) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "Not enough data");
        return FALSE;
    }

    if (value != NULL)
        *value = data[*offset + 3] << 24 |
                 data[*offset + 2] << 16 |
                 data[*offset + 1] << 8 |
                 data[*offset + 0];
    *offset += 4;

    return TRUE;
}

static gfloat
read_float (guint8  *data,
            gsize    data_length,
            gsize   *offset,
            gfloat  *value,
            GError **error)
{
    guint32 uint_value;
    if (!read_uint (data, data_length, offset, &uint_value, error))
        return FALSE;
    if (value != NULL)
        *value = *((gfloat *) &uint_value);
    return TRUE;
}

static gchar *
read_string (guint8  *data,
             gsize    data_length,
             gsize   *offset,
             GError **error)
{
    guint32 length;
    if (!read_uint (data, data_length, offset, &length, error))
        return NULL;

    if (*offset + length > data_length) {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                     "Not enough data for %u octet string", length);
        return NULL;
    }

    gchar *value = g_malloc (length + 1);
    memcpy (value, data + *offset, length);
    value[length] = '\0';
    *offset += length;

    return value;
}

static gboolean
read_dict (guint8     *data,
           gsize       data_length,
           gsize      *offset,
           GHashTable *table,
           GError    **error)
{
    guint32 count;
    if (!read_uint (data, data_length, offset, &count, error))
        return FALSE;

    for (guint32 i = 0; i < count; i++) {
        gchar *name = read_string (data, data_length, offset, error);
        if (name == NULL)
            return FALSE;
        gchar *value = read_string (data, data_length, offset, error);
        if (name == NULL)
            return FALSE;
        g_hash_table_insert (table, name, value);
    }

    return TRUE;
}

static void
pv_vox_file_dispose (GObject *object)
{
    PvVoxFile *self = PV_VOX_FILE (object);

    g_clear_object (&self->file);
    g_clear_pointer (&self->data, g_free);
    g_clear_pointer (&self->nodes, g_ptr_array_unref);
    g_clear_pointer (&self->groups, g_ptr_array_unref);
    g_clear_pointer (&self->shapes, g_ptr_array_unref);
    g_clear_pointer (&self->layers, g_ptr_array_unref);
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
    self->nodes = g_ptr_array_new_with_free_func ((GDestroyNotify) pv_vox_node_free);
    self->groups = g_ptr_array_new_with_free_func ((GDestroyNotify) pv_vox_node_group_free);
    self->shapes = g_ptr_array_new_with_free_func ((GDestroyNotify) pv_vox_node_shape_free);
    self->layers = g_ptr_array_new_with_free_func ((GDestroyNotify) pv_vox_layer_free);
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
                   gsize     *offset,
                   GError   **error)
{
    // Contains no data
    return TRUE;
}

static gboolean
decode_pack_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   gsize     *offset,
                   GError   **error)
{
    guint32 n_models;
    if (!read_uint (chunk_start, chunk_length, offset, &n_models, error))
        return FALSE;

    return TRUE;
}

static gboolean
decode_size_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   gsize     *offset,
                   GError   **error)
{
    VoxModel *model = g_new0 (VoxModel, 1);
    if (!read_uint (chunk_start, chunk_length, offset, &model->size_x, error) ||
        !read_uint (chunk_start, chunk_length, offset, &model->size_y, error) ||
        !read_uint (chunk_start, chunk_length, offset, &model->size_z, error))
        return FALSE;
    g_ptr_array_add (self->models, model);

    return TRUE;
}

static gboolean
decode_xyzi_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   gsize     *offset,
                   GError   **error)
{
    guint32 voxels_length;
    if (!read_uint (chunk_start, chunk_length, offset, &voxels_length, error))
        return FALSE;
    if (*offset + voxels_length * 4 > chunk_length) {
        gsize max_voxels_length = (chunk_length - *offset) / 4;
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                     "%u voxels required but only space for %zi", voxels_length, max_voxels_length);
        return FALSE;
    }
    *offset += voxels_length * 4;

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
decode_nshp_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   gsize     *offset,
                   GError   **error)
{
    PvVoxNodeShape *shape = g_new0 (PvVoxNodeShape, 1);
    shape->attributes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    g_ptr_array_add (self->shapes, shape);

    if (!read_uint (chunk_start, chunk_length, offset, &shape->id, error) ||
        !read_dict (chunk_start, chunk_length, offset, shape->attributes, error) ||
        !read_uint (chunk_start, chunk_length, offset, &shape->models_length, error))
        return FALSE;

    shape->models = g_new (guint32, shape->models_length);
    shape->model_attributes = g_new (GHashTable *, shape->models_length);
    for (guint32 i = 0; i < shape->models_length; i++) {
        shape->model_attributes[i] = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
        if (!read_uint (chunk_start, chunk_length, offset, &shape->models[i], error) ||
            !read_dict (chunk_start, chunk_length, offset, shape->model_attributes[i], error))
           return FALSE;
    }

    return TRUE;
}

static gboolean
decode_ntrn_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   gsize     *offset,
                   GError   **error)
{
    PvVoxNode *node = g_new0 (PvVoxNode, 1);
    node->attributes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    g_ptr_array_add (self->nodes, node);

    if (!read_uint (chunk_start, chunk_length, offset, &node->id, error) ||
        !read_dict (chunk_start, chunk_length, offset, node->attributes, error) ||
        !read_uint (chunk_start, chunk_length, offset, &node->child_id, error))
        return FALSE;

    guint32 n_frames;
    if (!read_uint (chunk_start, chunk_length, offset, NULL, error) || // ??
        !read_uint (chunk_start, chunk_length, offset, &node->layer_id, error) ||
        !read_uint (chunk_start, chunk_length, offset, &n_frames, error))
        return FALSE;

    for (guint32 i = 0; i < n_frames; i++) {
        g_autoptr(GHashTable) attributes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
        if (!read_dict (chunk_start, chunk_length, offset, attributes, error))
            return FALSE;
    }

    return TRUE;
}

static gboolean
decode_ngrp_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   gsize     *offset,
                   GError   **error)
{
    PvVoxNodeGroup *group = g_new0 (PvVoxNodeGroup, 1);
    group->attributes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    g_ptr_array_add (self->groups, group);

    if (!read_uint (chunk_start, chunk_length, offset, &group->id, error) ||
        !read_dict (chunk_start, chunk_length, offset, group->attributes, error) ||
        !read_uint (chunk_start, chunk_length, offset, &group->nodes_length, error))
        return FALSE;

    group->nodes = g_new (guint32, group->nodes_length);
    for (guint32 i = 0; i < group->nodes_length; i++) {
        if (!read_uint (chunk_start, chunk_length, offset, &group->nodes[i], error))
            return FALSE;
    }

    return TRUE;
}

static gboolean
decode_layr_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   gsize     *offset,
                   GError   **error)
{
    PvVoxLayer *layer = g_new0 (PvVoxLayer, 1);
    layer->attributes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    g_ptr_array_add (self->layers, layer);

    if (!read_uint (chunk_start, chunk_length, offset, &layer->id, error) ||
        !read_dict (chunk_start, chunk_length, offset, layer->attributes, error) ||
        !read_uint (chunk_start, chunk_length, offset, NULL, error)) // ??
        return FALSE;

    return TRUE;
}

static gboolean
decode_robj_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   gsize     *offset,
                   GError   **error)
{
    // Table of object properties keyed - see 'type' key.
    g_autoptr(GHashTable) attributes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    if (!read_dict (chunk_start, chunk_length, offset, attributes, error))
        return FALSE;

    return TRUE;
}

static gboolean
decode_rgba_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   gsize     *offset,
                   GError   **error)
{
    if (chunk_length != 1024) {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                     "Expected %d octets for palette, got %zi", 1024, chunk_length);
        return FALSE;
    }

    for (int offset = 0, index = 1; offset < chunk_length && index < 256; offset += 4, index++) {
        self->materials[index].r = chunk_start[offset + 0];
        self->materials[index].g = chunk_start[offset + 1];
        self->materials[index].b = chunk_start[offset + 2];
        self->materials[index].a = chunk_start[offset + 3];
    }
    *offset += 1024;

    return TRUE;
}

static gboolean
decode_matt_chunk (PvVoxFile *self,
                   guint8    *chunk_start,
                   gsize      chunk_length,
                   gsize     *offset,
                   GError   **error)
{
    guint32 id;
    if (!read_uint (chunk_start, chunk_length, offset, &id, error))
        return FALSE;
    // FIXME: Check id between 1 and 255
    PvVoxMaterial null_material;
    PvVoxMaterial *material = id < 256 ? &self->materials[id] : &null_material;
    guint32 type;
    if (!read_uint (chunk_start, chunk_length, offset, &type, error))
        return FALSE;
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
    gfloat weight;
    if (!read_float (chunk_start, chunk_length, offset, &weight, error))
        return FALSE;
    g_hash_table_insert (material->properties, g_strdup ("_weight"), g_strdup_printf ("%f", weight));
    guint32 property_bits;
    if (!read_uint (chunk_start, chunk_length, offset, &property_bits, error))
        return FALSE;
    if (property_bits & 0x00000001) {
        gfloat plastic;
        if (!read_float (chunk_start, chunk_length, offset, &plastic, error))
            return FALSE;
        g_hash_table_insert (material->properties, g_strdup ("_plastic"), g_strdup_printf ("%f", plastic));
    }
    if (property_bits & 0x00000002) {
        gfloat rough;
        if (!read_float (chunk_start, chunk_length, offset, &rough, error))
            return FALSE;
        g_hash_table_insert (material->properties, g_strdup ("_rough"), g_strdup_printf ("%f", rough));
    }
    if (property_bits & 0x00000004) {
        gfloat spec;
        if (!read_float (chunk_start, chunk_length, offset, &spec, error))
            return FALSE;
        g_hash_table_insert (material->properties, g_strdup ("_spec"), g_strdup_printf ("%f", spec));
    }
    if (property_bits & 0x00000008) {
        gfloat ior;
        if (!read_float (chunk_start, chunk_length, offset, &ior, error))
            return FALSE;
        g_hash_table_insert (material->properties, g_strdup ("_ior"), g_strdup_printf ("%f", ior));
    }
    if (property_bits & 0x00000010) {
        gfloat att;
        if (!read_float (chunk_start, chunk_length, offset, &att, error))
            return FALSE;
        g_hash_table_insert (material->properties, g_strdup ("_att"), g_strdup_printf ("%f", att));
    }
    if (property_bits & 0x00000020) {
        gfloat power;
        if (!read_float (chunk_start, chunk_length, offset, &power, error))
            return FALSE;
        //material->power = power;
    }
    if (property_bits & 0x00000040) {
        gfloat glow;
        if (!read_float (chunk_start, chunk_length, offset, &glow, error))
            return FALSE;
        g_hash_table_insert (material->properties, g_strdup ("_glow"), g_strdup_printf ("%f", glow));
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
                   gsize     *offset,
                   GError   **error)
{
    guint32 id;
    if (!read_uint (chunk_start, chunk_length, offset, &id, error))
        return FALSE;

    PvVoxMaterial null_material;
    PvVoxMaterial *material = id < 256 ? &self->materials[id] : &null_material;

    if (!read_dict (chunk_start, chunk_length, offset, material->properties, error))
        return FALSE;

    return TRUE;
}

static gboolean
decode_chunks (PvVoxFile *self,
               guint8    *data,
               gsize      data_length,
               gsize     *offset,
               GError   **error)
{
    while (*offset < data_length) {
        guint8 *chunk_start = data + *offset;
        gsize n_remaining = data_length - *offset;
        gsize chunk_offset = 0;

        /* Read header */
        guint32 chunk_id, chunk_data_length, child_chunks_length;
        g_autoptr(GError) e = NULL;
        if (!read_uint (chunk_start, n_remaining, &chunk_offset, &chunk_id, &e) ||
            !read_uint (chunk_start, n_remaining, &chunk_offset, &chunk_data_length, &e) ||
            !read_uint (chunk_start, n_remaining, &chunk_offset, &child_chunks_length, &e)) {
            g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                         "Failed to read header: %s", e->message);
            return FALSE;
        }

        g_autofree gchar *id_string = uint_to_id (chunk_id);

        /* Check required space is available */
        gsize chunk_length = chunk_offset + chunk_data_length + child_chunks_length;
        if (chunk_length > n_remaining) {
            g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                         "Chunk %s requires %zi octets, but only %zi available", id_string, chunk_length, n_remaining);
            return FALSE;
        }

        guint8 *chunk_data_start = chunk_start + chunk_offset;
        guint8 *child_chunks_start = chunk_data_start + chunk_data_length;

        gboolean (*decode_func)(PvVoxFile *, guint8 *, gsize, gsize *, GError **) = NULL;
        if (chunk_id == id_to_uint ("MAIN"))
            decode_func = decode_main_chunk;
        else if (chunk_id == id_to_uint ("PACK"))
            decode_func = decode_pack_chunk;
        else if (chunk_id == id_to_uint ("SIZE"))
            decode_func = decode_size_chunk;
        else if (chunk_id == id_to_uint ("XYZI"))
            decode_func = decode_xyzi_chunk;
        else if (chunk_id == id_to_uint ("nSHP"))
            decode_func = decode_nshp_chunk;
        else if (chunk_id == id_to_uint ("nTRN"))
            decode_func = decode_ntrn_chunk;
        else if (chunk_id == id_to_uint ("nGRP"))
            decode_func = decode_ngrp_chunk;
        else if (chunk_id == id_to_uint ("LAYR"))
            decode_func = decode_layr_chunk;
        else if (chunk_id == id_to_uint ("rOBJ"))
            decode_func = decode_robj_chunk;
        else if (chunk_id == id_to_uint ("RGBA"))
            decode_func = decode_rgba_chunk;
        else if (chunk_id == id_to_uint ("MATT"))
            decode_func = decode_matt_chunk;
        else if (chunk_id == id_to_uint ("MATL"))
            decode_func = decode_matl_chunk;

        if (decode_func != NULL) {
            gsize chunk_data_offset = 0;

            //g_debug ("Decoding %s chunk with %u octets data", id_string, chunk_data_length);
            if (!decode_func (self, chunk_data_start, chunk_data_length, &chunk_data_offset, &e)) {
                g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                             "Failed decode %s chunk: %s", id_string, e->message);
                return FALSE;
            }

            if (chunk_data_offset < chunk_data_length)
                g_debug ("Ignoring %zi octets after %s chunk", chunk_data_length - chunk_data_offset, id_string);
        }
        else
            g_debug ("Ignoring unknown MagicaVoxel chunk %s with %u octets data", id_string, chunk_data_length);

        /* Decode child chunks */
        if (child_chunks_length > 0) {
            gsize child_chunks_offset = 0;
            if (!decode_chunks (self, child_chunks_start, child_chunks_length, &child_chunks_offset, error))
                return FALSE;
            if (child_chunks_offset < child_chunks_length)
                g_debug ("Ignoring %zi octets after %s child chunks", child_chunks_length - child_chunks_offset, id_string);
        }

        *offset += chunk_length;
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

    gsize offset = 0;

    guint32 id;
    g_autoptr(GError) local_error = NULL;
    if (!read_uint (self->data, self->data_length, &offset, &id, &local_error) ||
        id != id_to_uint ("VOX ") ||
        !read_uint (self->data, self->data_length, &offset, &self->version, &local_error)) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                             "Not a MagicaVoxel file");
        return FALSE;
    }
    if (self->version != 150) {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                     "Unknown MagicaVoxel file version %u", self->version);
        return FALSE;
    }

    if (!decode_chunks (self, self->data, self->data_length, &offset, error))
        return FALSE;
    if (offset < self->data_length)
        g_debug ("Ignoring %zi octets after MagicaVoxel file", self->data_length - offset);

    return TRUE;
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
pv_vox_file_get_layer_count (PvVoxFile *self)
{
    g_return_val_if_fail (PV_IS_VOX_FILE (self), 0);
    return self->layers->len;
}

PvVoxLayer *
pv_vox_file_get_layer (PvVoxFile *self, guint32 index)
{
    g_return_val_if_fail (PV_IS_VOX_FILE (self), 0);
    g_return_val_if_fail (index < self->layers->len, 0);

    PvVoxLayer *layer = g_ptr_array_index (self->layers, index);
    return layer;
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
