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

G_DECLARE_FINAL_TYPE (PvMapFile, pv_map_file, PV, MAP_FILE, GObject)

PvMapFile     *pv_map_file_new              (void);

gboolean       pv_map_file_load             (PvMapFile    *file,
                                             GFile        *file_,
                                             GCancellable *cancellable,
                                             GError      **error);

guint64        pv_map_file_get_width        (PvMapFile    *file);

guint64        pv_map_file_get_height       (PvMapFile    *file);

guint64        pv_map_file_get_depth        (PvMapFile    *file);

const gchar   *pv_map_file_get_name         (PvMapFile    *file);

const gchar   *pv_map_file_get_description  (PvMapFile    *file);

const gchar   *pv_map_file_get_author       (PvMapFile    *file);

const gchar   *pv_map_file_get_author_email (PvMapFile    *file);

const gchar   *pv_map_file_get_block_name   (PvMapFile    *file,
                                             guint16       block_id);

gsize          pv_map_file_get_block_count  (PvMapFile    *file);

void           pv_map_file_get_block_color  (PvMapFile    *file,
                                             guint16       block_id,
                                             guint8       *red,
                                             guint8       *green,
                                             guint8       *blue);

void           pv_map_file_get_blocks       (PvMapFile    *file,
                                             guint64       x,
                                             guint64       y,
                                             guint64       z,
                                             guint64       width,
                                             guint64       height,
                                             guint64       depth,
                                             guint16      *blocks);
