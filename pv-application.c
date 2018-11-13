/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include "pv-application.h"
#include "pv-window.h"

struct _PvApplication
{
    GtkApplication parent_instance;
};

G_DEFINE_TYPE (PvApplication, pv_application, GTK_TYPE_APPLICATION)

static void
pv_application_activate (GApplication *self)
{
    PvWindow *window = pv_window_new ();
    gtk_application_add_window (GTK_APPLICATION (self), GTK_WINDOW (window));
    gtk_widget_show (GTK_WIDGET (window));
}

void
pv_application_class_init (PvApplicationClass *klass)
{
    GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

    app_class->activate = pv_application_activate;
}

void
pv_application_init (PvApplication *self)
{
}

PvApplication *
pv_application_new (void)
{
    return g_object_new (pv_application_get_type (), NULL);
}
