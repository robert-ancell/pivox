/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "pv-block-type.h"

struct _PvBlockType
{
    GObject parent_instance;

    gchar  *name;
    gdouble r;
    gdouble g;
    gdouble b;
};

G_DEFINE_TYPE (PvBlockType, pv_block_type, G_TYPE_OBJECT)

static void
pv_block_type_dispose (GObject *object)
{
    PvBlockType *self = PV_BLOCK_TYPE (object);

    g_clear_pointer (&self->name, g_free);

    G_OBJECT_CLASS (pv_block_type_parent_class)->dispose (object);
}

void
pv_block_type_class_init (PvBlockTypeClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = pv_block_type_dispose;
}

void
pv_block_type_init (PvBlockType *self)
{
}

PvBlockType *
pv_block_type_new (const gchar *name)
{
    PvBlockType *self = g_object_new (pv_block_type_get_type (), NULL);

    self->name = g_strdup (name);

    return self;
}

void
pv_block_type_set_color (PvBlockType *self,
                         gdouble      r,
                         gdouble      g,
                         gdouble      b)
{
    g_return_if_fail (PV_IS_BLOCK_TYPE (self));

    self->r = r;
    self->g = g;
    self->b = b;
}
