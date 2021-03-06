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

#include <gtk/gtk.h>

#include "pv-renderer.h"

G_DECLARE_FINAL_TYPE (PvWindow, pv_window, PV, WINDOW, GtkWindow)

PvWindow *pv_window_new          (void);

void      pv_window_set_renderer (PvWindow   *window,
                                  PvRenderer *renderer);
