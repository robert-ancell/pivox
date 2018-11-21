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

#include "pv-vox-loader.h"

struct _PvVoxLoader
{
    GObject  parent_instance;

    GFile   *file;

    guint8  *data;
    gsize    data_length;
    gsize    offset;

    guint32  size_x;
    guint32  size_y;
    guint32  size_z;
    guint32  palette[256];

    PvMap   *map;
};

G_DEFINE_TYPE (PvVoxLoader, pv_vox_loader, G_TYPE_OBJECT)

static guint32 default_palette[256] =
{
   0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
   0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff, 0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
   0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
   0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
   0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc, 0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
   0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
   0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
   0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66, 0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
   0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
   0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
   0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633, 0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
   0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
   0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
   0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000, 0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
   0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
   0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111
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
read_uint (PvVoxLoader *self)
{
    if (self->offset + 4 > self->data_length)
        return 0;

    gsize o = self->offset;
    guint32 value = self->data[o+3] << 24 |
                    self->data[o+2] << 16 |
                    self->data[o+1] << 8 |
                    self->data[o+0];

    self->offset += 4;

    return value;
}

static void
decode_size_chunk (PvVoxLoader *self)
{
    self->size_x = read_uint (self);
    self->size_y = read_uint (self);
    self->size_z = read_uint (self);

    g_clear_object (&self->map);
    self->map = pv_map_new (self->size_x, self->size_y, self->size_z);
}

static void
decode_xyzi_chunk (PvVoxLoader *self)
{
    PvBlockType *ground = pv_map_get_block_type (self->map, "ground");

    guint32 num_voxels = read_uint (self);
    for (guint32 i = 0; i < num_voxels; i++) {
        guint32 voxel = read_uint (self);
        guint8 x = (voxel >>  0) & 0xFF;
        guint8 y = (voxel >>  8) & 0xFF;
        guint8 z = (voxel >> 16) & 0xFF;
        //guint8 c = (voxel >> 24) & 0xFF;
        pv_map_set_block (self->map, x, y, z, ground);
    }
}

static void
decode_rgba_chunk (PvVoxLoader *self)
{
    for (guint32 i = 0; i < 255; i++)
        self->palette[i + 1] = read_uint (self);
}

static void
pv_vox_loader_dispose (GObject *object)
{
    PvVoxLoader *self = PV_VOX_LOADER (object);

    g_clear_object (&self->file);
    g_clear_pointer (&self->data, g_free);
    g_clear_object (&self->map);

    G_OBJECT_CLASS (pv_vox_loader_parent_class)->dispose (object);
}

void
pv_vox_loader_class_init (PvVoxLoaderClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = pv_vox_loader_dispose;
}

void
pv_vox_loader_init (PvVoxLoader *self)
{
    for (int i = 0; i < 256; i++)
        self->palette[i] = default_palette[i];
}

PvVoxLoader *
pv_vox_loader_new (GFile *file)
{
    PvVoxLoader *self;

    self = g_object_new (pv_vox_loader_get_type (), NULL);

    self->file = g_object_ref (file);

    return self;
}

PvMap *
pv_vox_loader_decode (PvVoxLoader  *self,
                      GCancellable *cancellable,
                      GError      **error)
{
    g_return_val_if_fail (PV_IS_VOX_LOADER (self), NULL);

    if (!g_file_load_contents (self->file, cancellable, (gchar **)&self->data, &self->data_length, NULL, error))
        return NULL;

    guint32 id = read_uint (self);
    if (id != id_to_uint ("VOX ")) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "Not a MagicaVoxel file");
        return NULL;
    }
    guint32 version = read_uint (self);
    g_printerr ("version: %u\n", version);

    id = read_uint (self);
    if (id != id_to_uint ("MAIN")) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "No MAIN chunk found");
        return NULL;
    }
    read_uint (self);
    read_uint (self);

    while (self->offset < self->data_length) {
        id = read_uint (self);
        if (id == 0)
            break;
        guint32 chunk_length = read_uint (self);
        guint32 child_chunks_length = read_uint (self);
        gsize next_offset = self->offset + chunk_length + child_chunks_length;
        if (id == id_to_uint ("SIZE")) {
            decode_size_chunk (self);
        }
        else if (id == id_to_uint ("XYZI")) {
            decode_xyzi_chunk (self);
        }
        else if (id == id_to_uint ("RGBA")) {
            decode_rgba_chunk (self);
        }
        else
            g_debug ("Unknown MagicaVoxel chunk %s\n", uint_to_id (id));
        self->offset = next_offset;
    }

    if (self->map == NULL) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "No models found");
        return NULL;
    }

    return self->map;
}
