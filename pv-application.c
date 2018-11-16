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
#include "pv-map.h"
#include "pv-window.h"

struct _PvApplication
{
    GtkApplication parent_instance;

    PvMap         *map;
    PvCamera      *camera;
};

G_DEFINE_TYPE (PvApplication, pv_application, GTK_TYPE_APPLICATION)

static void
pv_application_activate (GApplication *app)
{
    PvApplication *self = PV_APPLICATION (app);

    PvWindow *window = pv_window_new ();
    gtk_application_add_window (GTK_APPLICATION (self), GTK_WINDOW (window));
    gtk_widget_show (GTK_WIDGET (window));

    g_autoptr(PvRenderer) renderer = pv_renderer_new ();
    pv_renderer_set_map (renderer, self->map);
    pv_renderer_set_camera (renderer, self->camera);
    pv_window_set_renderer (window, renderer);
}

static void
pv_application_dispose (GObject *object)
{
    PvApplication *self = PV_APPLICATION (object);

    g_clear_object (&self->map);

    G_OBJECT_CLASS (pv_application_parent_class)->dispose (object);
}

void
pv_application_class_init (PvApplicationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

    object_class->dispose = pv_application_dispose;
    app_class->activate = pv_application_activate;
}

void
pv_application_init (PvApplication *self)
{
    self->map = pv_map_new ();
    self->camera = pv_camera_new ();
    pv_camera_set_position (self->camera, 0.0, 0.0, 3.0);
    pv_camera_set_target (self->camera, pv_map_get_width (self->map) / 2.0, pv_map_get_height (self->map) / 2.0, 0.0);
}

PvApplication *
pv_application_new (void)
{
    return g_object_new (pv_application_get_type (), NULL);
}
