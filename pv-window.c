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
    gdouble     pointer_x;
    gdouble     pointer_y;
};

G_DEFINE_TYPE (PvWindow, pv_window, GTK_TYPE_WINDOW)

static void
grab_pointer (PvWindow *self)
{
    GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (self->gl_area));
    GdkSeat *seat = gdk_display_get_default_seat (display);
    g_autoptr(GdkCursor) cursor = gdk_cursor_new_for_display (display, GDK_BLANK_CURSOR);
    GdkGrabStatus result = gdk_seat_grab (seat,
                                          gtk_widget_get_window (GTK_WIDGET (self->gl_area)),
                                          GDK_SEAT_CAPABILITY_POINTER,
                                          TRUE,
                                          cursor,
                                          NULL,
                                          NULL, NULL);
    if (result != GDK_GRAB_SUCCESS)
       g_warning ("Failed to grab pointer");
}

static void
warp_pointer (PvWindow *self)
{
    GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (self->gl_area));
    GdkSeat *seat = gdk_display_get_default_seat (display);
    GdkWindow *window = gtk_widget_get_window (GTK_WIDGET (self->gl_area));
    gint x, y;
    gdk_window_get_position (window, &x, &y);
    gdk_device_warp (gdk_seat_get_pointer (seat), gtk_widget_get_screen (GTK_WIDGET (self->gl_area)),
                     x + gdk_window_get_width (window) / 2,
                     y + gdk_window_get_height (window) / 2);
}

static void
realize_cb (PvWindow *self)
{
    GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (self->gl_area));
    g_autoptr(GdkCursor) cursor = gdk_cursor_new_for_display (display, GDK_BLANK_CURSOR);
    gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (self->gl_area)), cursor);

    warp_pointer (self);
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
    case GDK_KEY_g:
        grab_pointer (self);
        break;
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

    if (old_move[0] != self->move[0] || old_move[1] != self->move[1] || old_move[2] != self->move[2]) {
        PvCamera *camera = pv_renderer_get_camera (self->renderer);
        gfloat x, y, z;
        pv_camera_get_position (camera, &x, &y, &z);
        pv_camera_set_position (camera, x + self->move[0], y + self->move[1], z + self->move[2]);
        gtk_widget_queue_draw (GTK_WIDGET (self->gl_area));
    }

    return FALSE;
}

static gboolean
motion_notify_event_cb (PvWindow       *self,
                        GdkEventMotion *event)
{
    if (event->x == self->pointer_x || event->y == self->pointer_y)
        return FALSE;

    g_printerr ("%f %f\n", event->x - self->pointer_x, event->y - self->pointer_y);
    self->pointer_x = event->x;
    self->pointer_y = event->y;

    warp_pointer (self);

    return FALSE;
}

static gboolean
leave_notify_event_cb (PvWindow         *self,
                       GdkEventCrossing *event)
{
    warp_pointer (self);

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
    gtk_widget_class_bind_template_callback (widget_class, motion_notify_event_cb);
    gtk_widget_class_bind_template_callback (widget_class, leave_notify_event_cb);
    gtk_widget_class_bind_template_callback (widget_class, realize_cb);
    gtk_widget_class_bind_template_callback (widget_class, unrealize_cb);
    gtk_widget_class_bind_template_callback (widget_class, render_cb);
}

void
pv_window_init (PvWindow *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));

    gtk_widget_add_events (GTK_WIDGET (self->gl_area), GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
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
