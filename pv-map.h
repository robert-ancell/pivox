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

G_DECLARE_FINAL_TYPE (PvMap, pv_map, PV, MAP, GObject)

PvMap     *pv_map_new              (void);

gboolean       pv_map_load             (PvMap         *map,
                                        GInputStream  *stream,
                                        GCancellable  *cancellable,
                                        GError       **error);

gboolean       pv_map_save             (PvMap         *map,
                                        GOutputStream *stream,
                                        GCancellable  *cancellable,
                                        GError       **error);

void           pv_map_set_width        (PvMap         *map,
                                        guint64        width);

guint64        pv_map_get_width        (PvMap         *map);

void           pv_map_set_height       (PvMap         *map,
                                        guint64        height);

guint64        pv_map_get_height       (PvMap         *map);

void           pv_map_set_depth        (PvMap         *map,
                                        guint64        depth);

guint64        pv_map_get_depth        (PvMap         *map);

void           pv_map_set_name         (PvMap         *map,
                                        const gchar   *name);

const gchar   *pv_map_get_name         (PvMap         *map);

void           pv_map_set_description  (PvMap         *map,
                                        const gchar   *description);

const gchar   *pv_map_get_description  (PvMap         *map);

void           pv_map_set_author       (PvMap         *map,
                                        const gchar   *author);

const gchar   *pv_map_get_author       (PvMap         *map);

void           pv_map_set_author_email (PvMap         *map,
                                        const gchar   *author_email);

const gchar   *pv_map_get_author_email (PvMap         *map);

guint          pv_map_add_block        (PvMap         *map,
                                        const gchar   *name,
                                        guint8         red,
                                        guint8         green,
                                        guint8         blue);

gsize          pv_map_get_block_count  (PvMap         *map);

const gchar   *pv_map_get_block_name   (PvMap         *map,
                                        guint16        block_id);

void           pv_map_get_block_color  (PvMap         *map,
                                        guint16        block_id,
                                        guint8        *red,
                                        guint8        *green,
                                        guint8        *blue);

void           pv_map_add_area_raster8 (PvMap         *self,
                                        guint64        x,
                                        guint64        y,
                                        guint64        z,
                                        guint64        width,
                                        guint64        height,
                                        guint64        depth,
                                        guint8        *blocks);

void           pv_map_get_blocks       (PvMap         *map,
                                        guint64        x,
                                        guint64        y,
                                        guint64        z,
                                        guint64        width,
                                        guint64        height,
                                        guint64        depth,
                                        guint16       *blocks);
