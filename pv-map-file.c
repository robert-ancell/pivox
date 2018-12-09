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
    GPtrArray    *blocks;
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
    g_clear_pointer (&self->blocks, g_ptr_array_unref);

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
    self->blocks = g_ptr_array_new_with_free_func ((GDestroyNotify) g_bytes_unref);
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
            g_ptr_array_add (self->blocks, block);
        }

        offset += block_length;
        block_count++;
    }

    if (block_count == 0) {
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                     "Unable to load Pivox map file: Contains no blocks");
        return FALSE;
    }

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
