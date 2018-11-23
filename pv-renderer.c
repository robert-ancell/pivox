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
    GObject   parent_instance;

    PvMap    *map;

    PvCamera *camera;

    GLuint    program;
    GLuint    vao;
    GLint     x_offset;
    GLint     x_size;
    GLint     y_offset;
    GLint     y_size;
    GLint     z_offset;
    GLint     z_size;
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
    if (status == GL_FALSE) {
       GLint log_length;
       glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &log_length);
       gchar info[log_length];
       glGetShaderInfoLog (shader, 1024, NULL, info);
       g_printerr ("Failed to compile shader %s:\n", filename);
       g_printerr ("%s\n", info);
    }

    return shader;
}

static guint
add_square (GLfloat *vertices,
            GLfloat *pos,
            GLfloat *v0,
            GLfloat *v1)
{
    vertices[ 0] = pos[0];
    vertices[ 1] = pos[1];
    vertices[ 2] = pos[2];

    vertices[ 3] = pos[0] + v0[0];
    vertices[ 4] = pos[1] + v0[1];
    vertices[ 5] = pos[2] + v0[2];

    vertices[ 6] = pos[0] + v0[0] + v1[0];
    vertices[ 7] = pos[1] + v0[1] + v1[1];
    vertices[ 8] = pos[2] + v0[2] + v1[2];

    vertices[ 9] = pos[0];
    vertices[10] = pos[1];
    vertices[11] = pos[2];

    vertices[12] = vertices[6];
    vertices[13] = vertices[7];
    vertices[14] = vertices[8];

    vertices[15] = pos[0] + v1[0];
    vertices[16] = pos[1] + v1[1];
    vertices[17] = pos[2] + v1[2];

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
            for (guint z = 0; z < pv_map_get_depth (self->map); z++) {
                PvBlockType *type = pv_map_get_block (self->map, x, y, z);
                if (type != NULL)
                    n_vertices += 18 * 6;
            }
        }
    }

    GLfloat north[3] = {  1,  0,  0 };
    GLfloat south[3] = { -1,  0,  0 };
    GLfloat east[3]  = {  0,  1,  0 };
    GLfloat west[3]  = {  0, -1,  0 };
    GLfloat up[3]    = {  0,  0,  1 };
    GLfloat down[3]  = {  0,  0, -1 };

    GLfloat *vertices = g_malloc_n (n_vertices, sizeof (GLfloat));
    guint offset = 0;
    self->z_offset = offset;
    for (guint x = 0; x < pv_map_get_width (self->map); x++) {
        for (guint y = 0; y < pv_map_get_height (self->map); y++) {
            for (guint z = 0; z < pv_map_get_depth (self->map); z++) {
                PvBlockType *type = pv_map_get_block (self->map, x, y, z);
                if (type == NULL)
                    continue;

                GLfloat base_pos[3] = { x, y, z };
                GLfloat top_pos[3] = { x + 1, y + 1, z + 1};
                offset += add_square (&vertices[offset], base_pos, north, east);
                offset += add_square (&vertices[offset], top_pos, west, south);
            }
        }
    }
    self->z_size = offset - self->z_offset;
    self->x_offset = offset;
    for (guint x = 0; x < pv_map_get_width (self->map); x++) {
        for (guint y = 0; y < pv_map_get_height (self->map); y++) {
            for (guint z = 0; z < pv_map_get_depth (self->map); z++) {
                PvBlockType *type = pv_map_get_block (self->map, x, y, z);
                if (type == NULL)
                    continue;

                GLfloat base_pos[3] = { x, y, z };
                GLfloat top_pos[3] = { x + 1, y + 1, z + 1};
                offset += add_square (&vertices[offset], base_pos, east, up);
                offset += add_square (&vertices[offset], top_pos, down, west);
            }
        }
    }
    self->x_size = offset - self->x_offset;
    self->y_offset = offset;
    for (guint x = 0; x < pv_map_get_width (self->map); x++) {
        for (guint y = 0; y < pv_map_get_height (self->map); y++) {
            for (guint z = 0; z < pv_map_get_depth (self->map); z++) {
                PvBlockType *type = pv_map_get_block (self->map, x, y, z);
                if (type == NULL)
                    continue;

                GLfloat base_pos[3] = { x, y, z };
                GLfloat top_pos[3] = { x + 1, y + 1, z + 1};
                offset += add_square (&vertices[offset], base_pos, up, north);
                offset += add_square (&vertices[offset], top_pos, south, down);
            }
        }
    }
    self->y_size = offset - self->y_offset;

    glBufferData (GL_ARRAY_BUFFER, (self->x_size + self->y_size + self->z_size) * sizeof (GLfloat), vertices, GL_STATIC_DRAW);

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

    const gchar *renderer = (gchar *) glGetString (GL_RENDERER);
    g_printerr ("renderer: %s\n", renderer);
}

static void
pv_renderer_dispose (GObject *object)
{
    PvRenderer *self = PV_RENDERER (object);

    g_clear_object (&self->map);
    g_clear_object (&self->camera);

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
pv_renderer_set_camera (PvRenderer *self,
                        PvCamera   *camera)
{
    g_return_if_fail (PV_IS_RENDERER (self));
    g_return_if_fail (camera == NULL || PV_IS_CAMERA (camera));

    if (self->camera == camera)
        return;

    g_clear_object (&self->camera);
    self->camera = g_object_ref (camera);
}

PvCamera *
pv_renderer_get_camera (PvRenderer *self)
{
    g_return_val_if_fail (PV_IS_RENDERER (self), NULL);
    return self->camera;
}

void
pv_renderer_render (PvRenderer *self,
                    guint       width,
                    guint       height)
{
    g_return_if_fail (PV_IS_RENDERER (self));

    setup (self);

    glUseProgram (self->program);

    GLint mvp = glGetUniformLocation (self->program, "MVP");
    pv_camera_transform (self->camera, width, height, mvp);

    GLint color = glGetUniformLocation (self->program, "Color");
    glBindVertexArray (self->vao);
    glUniform3f (color, 1, 0, 0);
    glDrawArrays (GL_TRIANGLES, self->x_offset / 3, self->x_size / 3);
    glUniform3f (color, 0, 1, 0);
    glDrawArrays (GL_TRIANGLES, self->y_offset / 3, self->y_size / 3);
    glUniform3f (color, 0, 0, 1);
    glDrawArrays (GL_TRIANGLES, self->z_offset / 3, self->z_size / 3);
}
