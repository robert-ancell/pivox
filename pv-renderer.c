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

#include "pv-renderer.h"

struct _PvRenderer
{
    GObject parent_instance;

    PvMap  *map;

    GLuint  vertex_shader;
    GLuint  fragment_shader;
    GLuint  program;
    GLuint  buffer;
};

G_DEFINE_TYPE (PvRenderer, pv_renderer, G_TYPE_OBJECT)

const GLchar* vertex_shader_code =
    "attribute vec4 position;\n"
    "void main ()\n"
    "{\n"
    "   gl_Position = vec4 (position.xyz, 1.0);\n"
    "}\n";
const GLchar* fragment_shader_code =
    "void main ()\n"
    "{\n"
    "  gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0);\n"
    "}\n";

static void
setup (PvRenderer *self)
{
    if (self->buffer != 0)
        return;

    glGenBuffers (1, &self->buffer);
    glBindBuffer (GL_ARRAY_BUFFER, self->buffer);
    GLfloat vertices[] = { 0.0f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f };
    glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);

    self->vertex_shader = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (self->vertex_shader, 1, &vertex_shader_code, NULL);
    glCompileShader (self->vertex_shader);

    self->fragment_shader = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (self->fragment_shader, 1, &fragment_shader_code, NULL);
    glCompileShader (self->fragment_shader);

    self->program = glCreateProgram ();
    glAttachShader (self->program, self->vertex_shader);
    glAttachShader (self->program, self->fragment_shader);
    glLinkProgram (self->program);
    glUseProgram (self->program);

    GLint position_attr = glGetAttribLocation (self->program, "position");
    glEnableVertexAttribArray (position_attr);
    glVertexAttribPointer (position_attr, 2, GL_FLOAT, GL_FALSE, 0, 0);

    const gchar *renderer = (gchar *) glGetString (GL_RENDERER);
    g_printerr ("renderer: %s\n", renderer);
}

static void
pv_renderer_dispose (GObject *object)
{
    PvRenderer *self = PV_RENDERER (object);

    g_clear_object (&self->map);

    G_OBJECT_CLASS (pv_renderer_parent_class)->dispose (object);
}

void
pv_renderer_class_init (PvRendererClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = pv_renderer_dispose;
}

void
pv_renderer_init (PvRenderer *self)
{
}

PvRenderer *
pv_renderer_new (void)
{
    return g_object_new (pv_renderer_get_type (), NULL);
}

void
pv_renderer_set_map (PvRenderer *self,
                     PvMap      *map)
{
    g_return_if_fail (PV_IS_RENDERER (self));
    g_return_if_fail (map == NULL || PV_IS_MAP (map));

    if (self->map == map)
        return;

    g_clear_object (&self->map);
    self->map = g_object_ref (map);
}

void
pv_renderer_render (PvRenderer *self)
{
    g_return_if_fail (PV_IS_RENDERER (self));

    setup (self);

    glDrawArrays (GL_TRIANGLES, 0, 3);
}
