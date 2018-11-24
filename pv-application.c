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
#include "pv-vox-file.h"
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

    g_autoptr(GFile) file = g_file_new_for_uri ("resource:///com/example/pivox/map.vox");
    g_autoptr(PvVoxFile) vox_file = pv_vox_file_new (file);
    if (!pv_vox_file_decode (vox_file, NULL, &error)) {
       g_printerr ("Failed to load map: %s", error->message);
       return;
    }

    guint32 size_x, size_y, size_z;
    pv_vox_file_get_size (vox_file, &size_x, &size_y, &size_z);
    self->map = pv_map_new (size_x, size_y, size_z);
    g_autoptr(GPtrArray) block_types = g_ptr_array_new_with_free_func (g_object_unref);
    for (int i = 1; i < 256; i++) {
        g_autofree gchar *name = g_strdup_printf ("%d", i);
        g_autoptr(PvBlockType) block_type = pv_block_type_new (name);
        PvVoxMaterial *material = pv_vox_file_get_material (vox_file, i);
        pv_block_type_set_color (block_type, material->r / 255.0f, material->g / 255.0f, material->b / 255.0f);
        pv_map_add_block_type (self->map, block_type);
        g_ptr_array_add (block_types, g_object_ref (block_type));
    }
    guint32 model_count = pv_vox_file_get_model_count (vox_file);
    for (guint32 model_index = 0; model_index < model_count; model_index++) {
        guint32 voxel_count = pv_vox_file_get_voxel_count (vox_file, model_index);
        for (guint32 i = 0; i < voxel_count; i++) {
            guint8 x, y, z, color_index;
            pv_vox_file_get_voxel (vox_file, model_index, i, &x, &y, &z, &color_index);
            pv_map_set_block (self->map, x, y, z, g_ptr_array_index (block_types, color_index));
        }
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
    pv_camera_set_position (self->camera, 0.0, 0.0, 3.0);
    pv_camera_set_target (self->camera, pv_map_get_width (self->map) / 2.0, pv_map_get_height (self->map) / 2.0, 0.0);
}

PvApplication *
pv_application_new (void)
{
    return g_object_new (pv_application_get_type (), NULL);
}
