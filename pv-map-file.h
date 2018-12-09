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

PvMapFile     *pv_map_file_new              (GFile        *file);

gboolean       pv_map_file_decode           (PvMapFile    *file,
                                             GCancellable *cancellable,
                                             GError      **error);

guint64        pv_map_file_get_width        (PvMapFile    *file);

guint64        pv_map_file_get_height       (PvMapFile    *file);

guint64        pv_map_file_get_depth        (PvMapFile    *file);

const gchar   *pv_map_file_get_name         (PvMapFile    *file);

const gchar   *pv_map_file_get_description  (PvMapFile    *file);

const gchar   *pv_map_file_get_author       (PvMapFile    *file);

const gchar   *pv_map_file_get_author_email (PvMapFile    *file);
