/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <GLES2/gl2.h>

#include "pv-window.h"

struct _PvWindow
{
    GtkWindow   parent_instance;

    GtkGLArea  *gl_area;

    PvRenderer *renderer;
};

G_DEFINE_TYPE (PvWindow, pv_window, GTK_TYPE_WINDOW)

static void
realize_cb (PvWindow *self)
{
}

static void
unrealize_cb (PvWindow *self)
{
}

static gboolean
render_cb (PvWindow *self)
{
    glClearColor (0.5, 0.5, 0.5, 1.0);
    glClear (GL_COLOR_BUFFER_BIT);

    if (self->renderer != NULL)
        pv_renderer_render (self->renderer);

    glFlush ();

    return FALSE;
}

static void
pv_window_dispose (GObject *object)
{
    PvWindow *self = PV_WINDOW (object);

    g_clear_object (&self->renderer);

    G_OBJECT_CLASS (pv_window_parent_class)->dispose (object);
}

void
pv_window_class_init (PvWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = pv_window_dispose;

    gtk_widget_class_set_template_from_resource (widget_class, "/com/example/pivox/pv-window.ui");

    gtk_widget_class_bind_template_child (widget_class, PvWindow, gl_area);

    gtk_widget_class_bind_template_callback (widget_class, realize_cb);
    gtk_widget_class_bind_template_callback (widget_class, unrealize_cb);
    gtk_widget_class_bind_template_callback (widget_class, render_cb);
}

void
pv_window_init (PvWindow *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));
}

PvWindow *
pv_window_new (void)
{
    return g_object_new (pv_window_get_type (), NULL);
}

void
pv_window_set_renderer (PvWindow   *self,
                        PvRenderer *renderer)
{
    g_return_if_fail (PV_IS_WINDOW (self));
    g_return_if_fail (renderer == NULL || PV_IS_RENDERER (renderer));

    if (self->renderer == renderer)
        return;

    g_clear_object (&self->renderer);
    self->renderer = g_object_ref (renderer);
    gtk_widget_queue_draw (GTK_WIDGET (self->gl_area));
}
