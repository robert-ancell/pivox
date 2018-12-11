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
#include <json-glib/json-glib.h>

#include "pv-map-file.h"

struct _PvMapFile
{
    GObject       parent_instance;

    GFile        *file;

    JsonObject   *root;
    GPtrArray    *data_blocks;

    JsonArray    *blocks;

    JsonArray    *areas;
};

G_DEFINE_TYPE (PvMapFile, pv_map_file, G_TYPE_OBJECT)

static guint32
id_to_uint (const gchar *id)
{
    if (strlen (id) != 4)
        return 0;
    return id[3] << 24 | id[2] << 16 | id[1] << 8 | id[0];
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

gint64
get_int64_member (JsonObject *object, const gchar *member_name, gint64 default_value)
{
    if (!json_object_has_member (object, member_name))
        return default_value;
    JsonNode *node = json_object_get_member (object, member_name);
    if (!JSON_NODE_HOLDS_VALUE (node) || json_node_get_value_type (node) != G_TYPE_INT64)
        return default_value;
    return json_node_get_int (node);
}

guint64
get_uint64_member (JsonObject *object, const gchar *member_name, guint64 default_value)
{
    gint64 value = get_int64_member (object, member_name, -1);
    if (value < 0)
        return default_value;
    return value;
}

const gchar *
get_string_member (JsonObject *object, const gchar *member_name, const gchar *default_value)
{
    if (!json_object_has_member (object, member_name))
        return default_value;
    JsonNode *node = json_object_get_member (object, member_name);
    if (!JSON_NODE_HOLDS_VALUE (node) || json_node_get_value_type (node) != G_TYPE_STRING)
        return default_value;
    return json_node_get_string (node);
}

static void
pv_map_file_dispose (GObject *object)
{
    PvMapFile *self = PV_MAP_FILE (object);

    g_clear_object (&self->file);
    g_clear_pointer (&self->root, json_object_unref);
    g_clear_pointer (&self->data_blocks, g_ptr_array_unref);
    g_clear_pointer (&self->blocks, json_array_unref);
    g_clear_pointer (&self->areas, json_array_unref);

    G_OBJECT_CLASS (pv_map_file_parent_class)->dispose (object);
}

void
pv_map_file_class_init (PvMapFileClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = pv_map_file_dispose;
}

void
pv_map_file_init (PvMapFile *self)
{
    self->data_blocks = g_ptr_array_new_with_free_func ((GDestroyNotify) g_bytes_unref);
}

PvMapFile *
pv_map_file_new (GFile *file)
{
    PvMapFile *self;

    self = g_object_new (pv_map_file_get_type (), NULL);

    self->file = g_object_ref (file);

    return self;
}

gboolean
pv_map_file_decode (PvMapFile    *self,
                    GCancellable *cancellable,
                    GError      **error)
{
    g_return_val_if_fail (PV_IS_MAP_FILE (self), FALSE);

    g_autofree guint8 *data = NULL;
    gsize data_length;
    if (!g_file_load_contents (self->file, cancellable, (gchar **)&data, &data_length, NULL, error))
        return FALSE;

    gsize offset = 0;

    guint32 id;
    g_autoptr(GError) local_error = NULL;
    if (!read_uint (data, data_length, &offset, &id, &local_error) ||
        id != id_to_uint ("PiVx")) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                             "Not a Pivox map file");
        return FALSE;
    }

    int block_count = 0;
    while (offset < data_length) {
        guint32 block_length;
        if (!read_uint (data, data_length, &offset, &block_length, error))
            return FALSE;

        /* Terminate on zero length block */
        if (block_length == 0)
            break;

        if (offset + block_length > data_length) {
            g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                         "Unable to load Pivox map file: Not enough space for block %d", block_count);
            return FALSE;
        }

        if (block_count == 0) {
            g_autoptr(JsonParser) parser = json_parser_new ();
            g_autoptr(GError) local_error = NULL;
            if (!json_parser_load_from_data (parser, (const gchar *) (data + offset), block_length, &local_error)) {
                g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                             "Unable to load Pivox map file: Block 0 does not contain valid JSON data: %s", local_error->message);
                return FALSE;
            }
            JsonNode *root = json_parser_steal_root (parser);
            if (!JSON_NODE_HOLDS_OBJECT (root)) {
                g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                                     "Unable to load Pivox map file: Block 0 does not contain a JSON object");
                return FALSE;
            }
            self->root = json_node_dup_object (root);
        }
        else {
            GBytes *block = g_bytes_new (data + offset, block_length);
            g_ptr_array_add (self->data_blocks, block);
        }

        offset += block_length;
        block_count++;
    }

    if (block_count == 0) {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                     "Unable to load Pivox map file: Contains no data blocks");
        return FALSE;
    }

    if (json_object_has_member (self->root, "blocks"))
        self->blocks = json_array_ref (json_object_get_array_member (self->root, "blocks"));

    if (json_object_has_member (self->root, "areas"))
        self->areas = json_array_ref (json_object_get_array_member (self->root, "areas"));

    return TRUE;
}

guint64
pv_map_file_get_width (PvMapFile *self)
{
    g_return_val_if_fail (PV_IS_MAP_FILE (self), 0);
    return get_uint64_member (self->root, "width", 1);
}

guint64
pv_map_file_get_height (PvMapFile *self)
{
    g_return_val_if_fail (PV_IS_MAP_FILE (self), 0);
    return get_uint64_member (self->root, "height", 1);
}

guint64
pv_map_file_get_depth (PvMapFile *self)
{
    g_return_val_if_fail (PV_IS_MAP_FILE (self), 0);
    return get_uint64_member (self->root, "depth", 1);
}

const gchar *
pv_map_file_get_name (PvMapFile *self)
{
    g_return_val_if_fail (PV_IS_MAP_FILE (self), NULL);
    return get_string_member (self->root, "name", NULL);
}

const gchar *
pv_map_file_get_description (PvMapFile *self)
{
    g_return_val_if_fail (PV_IS_MAP_FILE (self), NULL);
    return get_string_member (self->root, "description", NULL);
}

const gchar *
pv_map_file_get_author (PvMapFile *self)
{
    g_return_val_if_fail (PV_IS_MAP_FILE (self), NULL);
    return get_string_member (self->root, "author", NULL);
}

const gchar *
pv_map_file_get_author_email (PvMapFile *self)
{
    g_return_val_if_fail (PV_IS_MAP_FILE (self), NULL);
    return get_string_member (self->root, "author_email", NULL);
}

gsize
pv_map_file_get_block_count (PvMapFile *self)
{
    g_return_val_if_fail (PV_IS_MAP_FILE (self), 0);
    return json_array_get_length (self->blocks);
}

const gchar *
pv_map_file_get_block_name (PvMapFile *self,
                            guint16    block_id)
{
    g_return_val_if_fail (PV_IS_MAP_FILE (self), NULL);
    g_return_val_if_fail (block_id < json_array_get_length (self->blocks), NULL);

    JsonObject *block = json_array_get_object_element (self->blocks, block_id);
    return get_string_member (block, "name", NULL);
}

static guint8
parse_hex (gchar c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else
        return 0;
}

static void
parse_rgb (const gchar *color,
           guint8      *red,
           guint8      *green,
           guint8      *blue)
{
    *red = *green = *blue = 0;

    if (color == NULL || color[0] != '#')
        return;

    if (color[1] == '\0' || color[2] == '\0')
        return;
    *red = parse_hex (color[1]) << 4 | parse_hex (color[2]);

    if (color[3] == '\0' || color[4] == '\0')
        return;
    *green = parse_hex (color[3]) << 4 | parse_hex (color[4]);

    if (color[5] == '\0' || color[6] == '\0')
        return;
    *blue = parse_hex (color[5]) << 4 | parse_hex (color[6]);
}

void
pv_map_file_get_block_color (PvMapFile *self,
                             guint16    block_id,
                             guint8    *red,
                             guint8    *green,
                             guint8    *blue)
{
    g_return_if_fail (PV_IS_MAP_FILE (self));
    g_return_if_fail (block_id < json_array_get_length (self->blocks));

    JsonObject *block = json_array_get_object_element (self->blocks, block_id);
    const gchar *color = get_string_member (block, "color", NULL);
    parse_rgb (color, red, green, blue);
}

void
pv_map_file_get_blocks (PvMapFile *self,
                        guint64    fill_x,
                        guint64    fill_y,
                        guint64    fill_z,
                        guint64    fill_width,
                        guint64    fill_height,
                        guint64    fill_depth,
                        guint16   *fill_blocks)
{
    g_return_if_fail (PV_IS_MAP_FILE (self));

    /* Start with default block */
    memset (fill_blocks, 0, sizeof (guint16) * fill_width * fill_height * fill_depth);

    if (self->areas == NULL)
        return;

    guint n_areas = json_array_get_length (self->areas);
    for (guint i = 0; i < n_areas; i++) {
        JsonObject *area = json_array_get_object_element (self->areas, i);

        guint64 area_x = get_uint64_member (area, "x", 0);
        guint64 area_width = get_uint64_member (area, "width", 0);
        if (area_x > fill_x + fill_width || area_x + area_width < fill_x)
            continue;

        guint64 area_y = get_uint64_member (area, "y", 0);
        guint64 area_height = get_uint64_member (area, "height", 0);
        if (area_y > fill_y + fill_height || area_y + area_height < fill_y)
            continue;

        guint64 area_z = get_uint64_member (area, "z", 0);
        guint64 area_depth = get_uint64_member (area, "depth", 0);
        if (area_z > fill_z + fill_depth || area_z + area_depth < fill_z)
            continue;

        /* Get overlapping area */
        guint64 x0 = MAX (fill_x, area_x);
        guint64 x1 = MIN (fill_x + fill_width, area_x + area_width);
        guint64 y0 = MAX (fill_y, area_y);
        guint64 y1 = MIN (fill_y + fill_height, area_y + area_height);
        guint64 z0 = MAX (fill_z, area_z);
        guint64 z1 = MIN (fill_z + fill_depth, area_z + area_depth);

        const gchar *type = json_object_get_string_member (area, "type");
        if (g_strcmp0 (type, "fill") == 0) {
            gint64 block = json_object_get_int_member (area, "block");
            g_assert (block < 65536);
            for (guint x = x0; x < x1; x++)
               for (guint y = y0; y < y1; y++)
                   for (guint z = z0; z < z1; z++)
                       fill_blocks[((z - fill_z) * fill_height + (y - fill_y)) * fill_width + (x - fill_x)] = block;
        }
        else if (g_strcmp0 (type, "raster8") == 0) {
            gint64 data_block_index = json_object_get_int_member (area, "data");
            g_assert (data_block_index >= 0);
            g_assert (data_block_index < self->data_blocks->len);
            GBytes *data_block = g_ptr_array_index (self->data_blocks, data_block_index);
            gsize blocks_length;
            const guint8 *blocks = g_bytes_get_data (data_block, &blocks_length);
            g_assert (blocks_length == area_width * area_height * area_depth);
            // FIXME "compression"
            for (guint x = x0; x < x1; x++)
               for (guint y = y0; y < y1; y++)
                   for (guint z = z0; z < z1; z++) {
                       guint8 block = blocks[(((z - area_z) * area_height) + (y - area_y)) * area_width + (x - area_x)];
                       fill_blocks[((z - fill_z) * fill_height + (y - fill_y)) * fill_width + (x - fill_x)] = block;
                   }
        }
        else if (g_strcmp0 (type, "coord8.8") == 0) {
            gint64 data_block_index = json_object_get_int_member (area, "data");
            g_assert (data_block_index >= 0);
            g_assert (data_block_index < self->data_blocks->len);
            GBytes *data_block = g_ptr_array_index (self->data_blocks, data_block_index);
            gsize data_length;
            const guint8 *data = g_bytes_get_data (data_block, &data_length);
            g_assert (data_length % 4 == 0);
            // FIXME "compression"
            gsize offset = 0;
            while (offset < data_length) {
                guint8 x = data[offset * 4 + 0] + area_x - fill_x;
                guint8 y = data[offset * 4 + 1] + area_y - fill_y;
                guint8 z = data[offset * 4 + 2] + area_z - fill_z;
                guint8 block = data[offset * 4 + 3];
                fill_blocks[(z * fill_height + y) * fill_width + x] = block;
                offset += 4;
            }
        }
        else
            g_warning ("Ignoring unknown area type '%s'", type);
    }
}
