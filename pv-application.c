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
#include "pv-map-file.h"
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

    g_autoptr(GFile) pivox_file = g_file_new_for_uri ("resource:///com/example/pivox/map.pivox");
    g_autoptr(PvMapFile) map_file = pv_map_file_new (pivox_file);
    if (!pv_map_file_decode (map_file, NULL, &error)) {
       g_printerr ("Failed to load map: %s\n", error->message);
       return;
    }
    g_printerr ("Map name: %s\n", pv_map_file_get_name (map_file));
    g_printerr ("Map size: %" G_GUINT64_FORMAT "x%" G_GUINT64_FORMAT "x%" G_GUINT64_FORMAT "\n", pv_map_file_get_width (map_file), pv_map_file_get_height (map_file), pv_map_file_get_depth (map_file));

    self->map = pv_map_new (pv_map_file_get_width (map_file), pv_map_file_get_height (map_file), pv_map_file_get_depth (map_file));

    gsize n_blocks = pv_map_file_get_block_count (map_file);
    g_autoptr(GPtrArray) block_types = g_ptr_array_new_with_free_func (g_object_unref);
    for (gsize i = 0; i < n_blocks; i++) {
        const gchar *name = pv_map_file_get_block_name (map_file, i);
        g_autoptr(PvBlockType) block_type = pv_block_type_new (name);

        guint8 red, green, blue;
        pv_map_file_get_block_color (map_file, i, &red, &green, &blue);
        gfloat color[3] = { red / 255.0f, green / 255.0f, blue / 255.0f };
        pv_block_type_set_color (block_type, color);

        pv_map_add_block_type (self->map, block_type);
        g_ptr_array_add (block_types, g_object_ref (block_type));

        g_printerr ("Block %zi: %s #%02x%02x%02x\n", i, pv_block_type_get_name (block_type), red, green, blue);
    }

    guint16 blocks[16 * 16 * 16];
    pv_map_file_get_blocks (map_file, 0, 0, 0, 16, 16, 16, blocks);
    for (guint64 x = 0; x < 16; x++)
        for (guint64 y = 0; y < 16; y++)
            for (guint64 z = 0; z < 16; z++) {
                guint64 index = ((z * 16) + y) * 16 + x;
                guint16 block_id = blocks[index];
                if (block_id != 0)
                    pv_map_set_block (self->map, x, y, z, g_ptr_array_index (block_types, block_id));
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
