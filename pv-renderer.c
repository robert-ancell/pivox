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
#include <math.h>

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

static guint
add_square (GLfloat *vertices,
            guint    x,
            guint    y,
            guint    z)
{
    GLfloat s = 1.0f;
    GLfloat x0 = x * s - 4.0f;
    GLfloat y0 = y * s - 4.0f;
    GLfloat x1 = x0 + s;
    GLfloat y1 = y0 + s;
    GLfloat z0 = z * s - 4.0f;

    vertices[ 0] = x0;
    vertices[ 1] = y0;
    vertices[ 2] = z0;

    vertices[ 3] = x1;
    vertices[ 4] = y0;
    vertices[ 5] = z0;

    vertices[ 6] = x1;
    vertices[ 7] = y1;
    vertices[ 8] = z0;

    vertices[ 9] = x0;
    vertices[10] = y0;
    vertices[11] = z0;

    vertices[12] = x1;
    vertices[13] = y1;
    vertices[14] = z0;

    vertices[15] = x0;
    vertices[16] = y1;
    vertices[17] = z0;

    return 18;
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

    guint n_vertices = 0;
    for (guint x = 0; x < pv_map_get_width (self->map); x++) {
        for (guint y = 0; y < pv_map_get_height (self->map); y++) {
            PvBlockType *type = pv_map_get_block (self->map, x, y, 0);
            if (type != NULL)
                n_vertices += 18;
        }
    }
    GLfloat *vertices = g_malloc_n (n_vertices, sizeof (GLfloat));
    guint offset = 0;
    for (guint x = 0; x < pv_map_get_width (self->map); x++) {
        for (guint y = 0; y < pv_map_get_height (self->map); y++) {
            PvBlockType *type = pv_map_get_block (self->map, x, y, 0);
            if (type != NULL)
                offset += add_square (&vertices[offset], x, y, 0);
        }
    }
    glBufferData (GL_ARRAY_BUFFER, n_vertices * sizeof (GLfloat), vertices, GL_STATIC_DRAW);

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
    glVertexAttribPointer (position_attr, 3, GL_FLOAT, GL_FALSE, 0, 0);

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

static void
make_projection_matrix (GLfloat  fov_y,
                        GLfloat  aspect,
                        GLfloat  z_near,
                        GLfloat  z_far,
                        GLfloat *m)
{
    GLfloat f = atanf (fov_y / 2.0f);
    m[ 0] = f / aspect;
    m[ 1] = 0;
    m[ 2] = 0;
    m[ 3] = 0;
    m[ 4] = 0;
    m[ 5] = f;
    m[ 6] = 0;
    m[ 7] = 0;
    m[ 8] = 0;
    m[ 9] = 0;
    m[10] = (z_far + z_near) / (z_near - z_far);
    m[11] = (2.0f * z_far * z_near) / (z_near - z_far);
    m[12] = 0;
    m[13] = 0;
    m[14] = -1.0f;
    m[15] = 0;
}

void
pv_renderer_render (PvRenderer *self)
{
    g_return_if_fail (PV_IS_RENDERER (self));

    setup (self);

    glUseProgram (self->program);

    GLfloat mvp[16];
    make_projection_matrix (M_PI / 3.0f, 1.0f, 0.1f, 100.0f, mvp);
    glUniformMatrix4fv (self->mvp, 1, GL_TRUE, mvp);

    glBindVertexArray (self->vao);
    glDrawArrays (GL_TRIANGLES, 0, 6 * pv_map_get_width (self->map) * pv_map_get_height (self->map));
}
