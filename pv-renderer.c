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

    GLfloat camera_x;
    GLfloat camera_y;
    GLfloat camera_z;
    GLfloat target_x;
    GLfloat target_y;
    GLfloat target_z;

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
    GLfloat x0 = x * s;
    GLfloat y0 = y * s;
    GLfloat x1 = x0 + s;
    GLfloat y1 = y0 + s;
    GLfloat z0 = z * s;

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

void
pv_renderer_set_camera (PvRenderer *self,
                        gfloat      x,
                        gfloat      y,
                        gfloat      z,
                        gfloat      target_x,
                        gfloat      target_y,
                        gfloat      target_z)
{
    g_return_if_fail (PV_IS_RENDERER (self));
    self->camera_x = x;
    self->camera_y = y;
    self->camera_z = z;
    self->target_x = target_x;
    self->target_y = target_y;
    self->target_z = target_z;
}

static void
vector_make (GLfloat  x,
             GLfloat  y,
             GLfloat  z,
             GLfloat *result)
{
    result[0] = x;
    result[1] = y;
    result[2] = z;
}

static void
vector_normalize (GLfloat *v)
{
    GLfloat l = sqrtf (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (l == 0)
        return;
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}

static void
vector_cross (GLfloat *a,
              GLfloat *b,
              GLfloat *result)
{
    vector_make (a[1]*b[2] - a[2]*b[1],
                 a[2]*b[0] - a[0]*b[2],
                 a[0]*b[1] - a[1]*b[0],
                 result);
}

static void
matrix_make (GLfloat v00, GLfloat v10, GLfloat v20, GLfloat v30,
             GLfloat v01, GLfloat v11, GLfloat v21, GLfloat v31,
             GLfloat v02, GLfloat v12, GLfloat v22, GLfloat v32,
             GLfloat v03, GLfloat v13, GLfloat v23, GLfloat v33,
             GLfloat *result)
{
    result[ 0] = v00;
    result[ 1] = v10;
    result[ 2] = v20;
    result[ 3] = v30;
    result[ 4] = v01;
    result[ 5] = v11;
    result[ 6] = v21;
    result[ 7] = v31;
    result[ 8] = v02;
    result[ 9] = v12;
    result[10] = v22;
    result[11] = v32;
    result[12] = v03;
    result[13] = v13;
    result[14] = v23;
    result[15] = v33;
}

static void
matrix_make_projection (GLfloat  fov_y,
                        GLfloat  aspect,
                        GLfloat  z_near,
                        GLfloat  z_far,
                        GLfloat *result)
{
    GLfloat f = atanf (fov_y / 2.0f);
    GLfloat a = f / aspect;
    GLfloat b = (z_far + z_near) / (z_near - z_far);
    GLfloat c = (2.0f * z_far * z_near) / (z_near - z_far);
    matrix_make ( a,  0,  0,  0,
                  0,  f,  0,  0,
                  0,  0,  b,  c,
                  0,  0, -1,  0,
                 result);
}

static void
matrix_make_direction (GLfloat *dir,
                       GLfloat *up,
                       GLfloat *result)
{
    GLfloat s[3];
    vector_cross (dir, up, s);
    GLfloat u[3];
    vector_cross (dir, s, u);
    matrix_make (   s[0],    s[1],    s[2], 0,
                    u[0],    u[1],    u[2], 0,
                 -dir[0], -dir[1], -dir[2], 0,
                       0,       0,       0, 1,
                 result);
}

static void
matrix_mult (GLfloat *a, GLfloat *b, GLfloat *result)
{
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result[col * 4 + row] = 0;
            for (int i = 0; i < 4; i++)
                result[col * 4 + row] += a[col * 4 + i] * b[i * 4 + row];
        }
    }
}

static void
matrix_translate (GLfloat x, GLfloat y, GLfloat z, GLfloat *result)
{
    matrix_make (1, 0, 0, x,
                 0, 1, 0, y,
                 0, 0, 1, z,
                 0, 0, 0, 1,
                 result);
}

void
pv_renderer_render (PvRenderer *self,
                    guint       width,
                    guint       height)
{
    g_return_if_fail (PV_IS_RENDERER (self));

    setup (self);

    glUseProgram (self->program);

    GLfloat proj[16];
    matrix_make_projection (M_PI / 3.0f, (GLfloat) width / height, 0.1f, 100.0f, proj);

    GLfloat trans[16];
    matrix_translate (self->camera_x, self->camera_y, self->camera_z, trans);
    GLfloat rot[16];
    GLfloat up[3], dir[3];
    vector_make (0, 0, 1, up);
    vector_make (self->target_x - self->camera_x,
                 self->target_y - self->camera_y,
                 self->target_z - self->camera_z,
                 dir);
    vector_normalize (dir);
    matrix_make_direction (dir, up, rot);
    GLfloat t[16];
    matrix_mult (proj, rot, t);
    GLfloat mvp[16];
    matrix_mult (t, trans, mvp);

    glUniformMatrix4fv (self->mvp, 1, GL_TRUE, mvp);

    glBindVertexArray (self->vao);
    glDrawArrays (GL_TRIANGLES, 0, 6 * pv_map_get_width (self->map) * pv_map_get_height (self->map));
}
