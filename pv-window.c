/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "pv-window.h"

struct _PvWindow
{
    GtkWindow parent_instance;
};

G_DEFINE_TYPE (PvWindow, pv_window, GTK_TYPE_WINDOW)

void
pv_window_class_init (PvWindowClass *klass)
{
}

void
pv_window_init (PvWindow *self)
{
}

PvWindow *
pv_window_new (void)
{
    return g_object_new (pv_window_get_type (), NULL);
}
