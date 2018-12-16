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
load_map (PvApplication *self)
{
    g_autoptr(GError) error = NULL;

    self->map = pv_map_new ();
    g_autoptr(GFile) file = g_file_new_for_uri ("resource:///com/example/pivox/map.pivox");
    g_autoptr(GFileInputStream) stream = g_file_read (file, NULL, &error);
    if (stream == NULL ||
        !pv_map_load (self->map, G_INPUT_STREAM (stream), NULL, &error)) {
        g_printerr ("Failed to load map: %s\n", error->message);
        return;
    }
    g_printerr ("Map name: %s\n", pv_map_get_name (self->map));
    g_printerr ("Map size: %" G_GUINT64_FORMAT "x%" G_GUINT64_FORMAT "x%" G_GUINT64_FORMAT "\n", pv_map_get_width (self->map), pv_map_get_height (self->map), pv_map_get_depth (self->map));

    gsize n_blocks = pv_map_get_block_count (self->map);
    for (gsize i = 0; i < n_blocks; i++) {
        const gchar *name = pv_map_get_block_name (self->map, i);
        guint8 red, green, blue;
        pv_map_get_block_color (self->map, i, &red, &green, &blue);
        g_printerr ("Block %zi: %s #%02x%02x%02x\n", i, name, red, green, blue);
    }
}

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
    load_map (self);

    self->camera = pv_camera_new ();
    pv_camera_set_position (self->camera, 0.0, 0.0, 7.0);
    pv_camera_set_target (self->camera, pv_map_get_width (self->map) / 2.0, pv_map_get_height (self->map) / 2.0, 0.0);
}

PvApplication *
pv_application_new (void)
{
    return g_object_new (pv_application_get_type (), NULL);
}
