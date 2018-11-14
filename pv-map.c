/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "pv-map.h"

struct _PvMap
{
    GObject   parent_instance;

    guint     width;
    guint     height;
    guint     depth;
    GPtrArray *block_types;
    guint8    *blocks;
};

G_DEFINE_TYPE (PvMap, pv_map, G_TYPE_OBJECT)

static PvBlockType *
id_to_block_type (PvMap *self,
                  guint8 id)
{
    if (id >= self->block_types->len)
        return NULL;

    return g_ptr_array_index (self->block_types, id);
}

static void
pv_map_dispose (GObject *object)
{
    PvMap *self = PV_MAP (object);

    g_clear_pointer (&self->blocks, g_free);
    g_clear_pointer (&self->block_types, g_ptr_array_unref);

    G_OBJECT_CLASS (pv_map_parent_class)->dispose (object);
}

void
pv_map_class_init (PvMapClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = pv_map_dispose;
}

void
pv_map_init (PvMap *self)
{
    self->block_types = g_ptr_array_new_with_free_func (g_object_unref);

    self->width = 8;
    self->height = 8;
    self->depth = 1;
    self->blocks = g_malloc0_n (self->width * self->height * self->depth, 1);

    g_ptr_array_add (self->block_types, NULL); /* air */
    g_autoptr(PvBlockType) ground_type = pv_block_type_new ("ground");
    g_ptr_array_add (self->block_types, g_object_ref (ground_type));
    for (guint x = 0; x < self->width; x++) {
        for (guint y = 0; y < self->height; y++) {
            if ((x + y) % 2 == 1)
                self->blocks[y * self->width + x] = 1;
        }
    }
}

PvMap *
pv_map_new (void)
{
    return g_object_new (pv_map_get_type (), NULL);
}

guint
pv_map_get_width (PvMap *self)
{
    g_return_val_if_fail (PV_IS_MAP (self), 0);
    return self->width;
}

guint
pv_map_get_height (PvMap *self)
{
    g_return_val_if_fail (PV_IS_MAP (self), 0);
    return self->height;
}

guint
pv_map_get_depth (PvMap *self)
{
    g_return_val_if_fail (PV_IS_MAP (self), 0);
    return self->depth;
}

PvBlockType *
pv_map_get_block (PvMap *self,
                  guint  x,
                  guint  y,
                  guint  z)
{
    g_return_val_if_fail (PV_IS_MAP (self), NULL);

    if (x >= self->width || y >= self->height || z >= self->depth)
        return NULL;

    guint id = self->blocks[(z * self->height + y) * self->width + x];

    return id_to_block_type (self, id);
}
