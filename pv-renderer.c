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
#include <gio/gio.h>

#include "pv-renderer.h"

struct _PvRenderer
{
    GObject parent_instance;

    PvMap  *map;

    GLuint  program;
    GLuint  vao;
    GLint   mvp;
};

G_DEFINE_TYPE (PvRenderer, pv_renderer, G_TYPE_OBJECT)

static GLuint
load_shader (GLenum shader_type, const gchar *filename)
{
    g_autofree gchar *path = g_strdup_printf ("/com/example/pivox/%s", filename);
    g_autoptr(GBytes) shader_source = g_resources_lookup_data (path, G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
    const gchar *shader_text = g_bytes_get_data (shader_source, NULL);

    GLuint shader = glCreateShader (shader_type);
    glShaderSource (shader, 1, &shader_text, NULL);
    glCompileShader (shader);

    GLint status;
    glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
       g_printerr ("Failed to compile shader\n");

    return shader;
}

static void
setup (PvRenderer *self)
{
    if (self->vao != 0)
        return;

    glGenVertexArrays (1, &self->vao);
    glBindVertexArray (self->vao);

    GLuint buffer;
    glGenBuffers (1, &buffer);
    glBindBuffer (GL_ARRAY_BUFFER, buffer);
    GLfloat vertices[] = { -0.5f, -0.5f,
                           -0.5f,  0.5f,
                            0.5f,  0.5f,
                            0.5f, -0.5f };
    glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);

    GLuint vertex_shader = load_shader (GL_VERTEX_SHADER, "pv-vertex.glsl");
    GLuint fragment_shader = load_shader (GL_FRAGMENT_SHADER, "pv-fragment.glsl");

    self->program = glCreateProgram ();
    glAttachShader (self->program, vertex_shader);
    glAttachShader (self->program, fragment_shader);
    glLinkProgram (self->program);
    GLint status;
    glGetProgramiv (self->program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
       g_printerr ("Failed to link program\n");

    glDetachShader (self->program, vertex_shader);
    glDetachShader (self->program, fragment_shader);

    GLint position_attr = glGetAttribLocation (self->program, "position");
    glEnableVertexAttribArray (position_attr);
    glVertexAttribPointer (position_attr, 2, GL_FLOAT, GL_FALSE, 0, 0);

    self->mvp = glGetUniformLocation (self->program, "MVP");

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

    glUseProgram (self->program);
    GLfloat mvp[16] = { 1, 0, 0, 0,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1 };
    glUniformMatrix4fv (self->mvp, 1, GL_FALSE, mvp);
    glBindVertexArray (self->vao);
    glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
}
