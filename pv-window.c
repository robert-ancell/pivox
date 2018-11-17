/*
 * Copyright (C) 2018 Robert Ancell
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. See http://www.gnu.org/copyleft/gpl.html the full text of the
 * license.
 */

#include <epoxy/gl.h>

#include "pv-window.h"

struct _PvWindow
{
    GtkWindow   parent_instance;

    GtkGLArea  *gl_area;

    PvRenderer *renderer;
    GLfloat     move[3];
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
key_event_cb (PvWindow    *self,
              GdkEventKey *event)
{
    GLfloat old_move[3];
    old_move[0] = self->move[0];
    old_move[1] = self->move[1];
    old_move[2] = self->move[2];

    switch (event->keyval) {
    case GDK_KEY_w:
    case GDK_KEY_Up:
        if (event->type == GDK_KEY_PRESS)
            self->move[0] = 1;
        else
            self->move[0] = 0;
        break;
    case GDK_KEY_a:
    case GDK_KEY_Left:
        if (event->type == GDK_KEY_PRESS)
            self->move[1] = -1;
        else
            self->move[1] = 0;
        break;
    case GDK_KEY_s:
    case GDK_KEY_Down:
        if (event->type == GDK_KEY_PRESS)
            self->move[0] = -1;
        else
            self->move[0] = 0;
        break;
    case GDK_KEY_d:
    case GDK_KEY_Right:
        if (event->type == GDK_KEY_PRESS)
            self->move[1] = 1;
        else
            self->move[1] = 0;
        break;
    case GDK_KEY_space:
        if (event->type == GDK_KEY_PRESS)
            self->move[2] = 1;
        else
            self->move[2] = 0;
        break;
    case GDK_KEY_Control_L:
    case GDK_KEY_Control_R:
        if (event->type == GDK_KEY_PRESS)
            self->move[2] = -1;
        else
            self->move[2] = 0;
        break;
    case GDK_KEY_Escape:
        if (event->type == GDK_KEY_RELEASE)
            gtk_widget_destroy (GTK_WIDGET (self));
        break;
    }

    if (old_move[0] != self->move[0] || old_move[1] != self->move[1] || old_move[2] != self->move[2])
        g_printerr ("%.0f %.0f %.0f\n", self->move[0], self->move[1], self->move[2]);

    return FALSE;
}

static gboolean
render_cb (PvWindow *self)
{
    glClearColor (0.5, 0.5, 0.5, 1.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);

    GtkAllocation alloc;
    gtk_widget_get_allocation (GTK_WIDGET (self->gl_area), &alloc);

    if (self->renderer != NULL)
        pv_renderer_render (self->renderer, alloc.width, alloc.height);

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

    gtk_widget_class_bind_template_callback (widget_class, key_event_cb);
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
