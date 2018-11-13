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
    GtkWindow parent_instance;
};

G_DEFINE_TYPE (PvWindow, pv_window, GTK_TYPE_WINDOW)

static void
realize_cb (PvWindow *window)
{
}

static void
unrealize_cb (PvWindow *window)
{
}

static gboolean
render_cb (PvWindow *window)
{
    glClearColor (0.5, 0.5, 0.5, 1.0);
    glClear (GL_COLOR_BUFFER_BIT);

    glFlush ();

    return FALSE;
}

void
pv_window_class_init (PvWindowClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class, "/com/example/pivox/pv-window.ui");

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
