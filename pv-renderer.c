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
#include "pv-vector.h"

struct _PvRenderer
{
    GObject   parent_instance;

    gchar    *gl_renderer;

    PvMap    *map;

    PvCamera *camera;

    GLuint    program;
    GLuint    vao;
    GLfloat   north;
    GLfloat   south;
    GLfloat   east;
    GLfloat   west;
    GLfloat   top;
    GLfloat   bottom;
    GLint     north_offset;
    GLint     north_size;
    GLint     south_offset;
    GLint     south_size;
    GLint     east_offset;
    GLint     east_size;
    GLint     west_offset;
    GLint     west_size;
    GLint     top_offset;
    GLint     top_size;
    GLint     bottom_offset;
    GLint     bottom_size;
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

static void
add_float (GArray *vertices,
           GLfloat value)
{
    g_array_append_val (vertices, value);
}

static void
add_vec3 (GArray  *vertices,
          GLfloat *value)
{
    add_float (vertices, value[0]);
    add_float (vertices, value[1]);
    add_float (vertices, value[2]);
}

static void
add_uint (GArray *vertices,
          GLuint value)
{
    g_array_append_val (vertices, value);
}

static guint16
get_block (gsize width, gsize height, gsize depth, guint16 *blocks, guint x, guint y, guint z)
{
    if (x >= width || y >= height || z >= depth)
        return 0;
    guint64 index = ((z * 16) + y) * 16 + x;
    return blocks[index];
}

static GLfloat
ambient_shade (gsize    width,
               gsize    height,
               gsize    depth,
               guint16 *blocks,
               GLfloat *pos)
{
    int n_around = 0;
    if (get_block (width, height, depth, blocks, pos[0], pos[1], pos[2]) != 0)
       n_around++;
    if (get_block (width, height, depth, blocks, pos[0] - 1, pos[1], pos[2]) != 0)
       n_around++;
    if (get_block (width, height, depth, blocks, pos[0] - 1, pos[1] - 1, pos[2]) != 0)
       n_around++;
    if (get_block (width, height, depth, blocks, pos[0], pos[1] - 1, pos[2]) != 0)
       n_around++;
    if (get_block (width, height, depth, blocks, pos[0], pos[1], pos[2] - 1) != 0)
       n_around++;
    if (get_block (width, height, depth, blocks, pos[0] - 1, pos[1], pos[2] - 1) != 0)
       n_around++;
    if (get_block (width, height, depth, blocks, pos[0] - 1, pos[1] - 1, pos[2] - 1) != 0)
       n_around++;
    if (get_block (width, height, depth, blocks, pos[0], pos[1] - 1, pos[2] - 1) != 0)
       n_around++;

    // FIXME: Work out concaveness properly
    return n_around > 4 ? 0.5f : 1.0f;
}

static void
add_square (gsize    width,
            gsize    height,
            gsize    depth,
            guint16 *blocks,
            GArray  *vertices,
            GArray  *triangles,
            GLfloat *a,
            GLfloat *v0,
            GLfloat *v1,
            GLfloat *face_color)
{
    guint start = vertices->len / 6;

    add_vec3 (vertices, a);
    GLfloat color[3];
    vec3_mult (color, face_color, ambient_shade (width, height, depth, blocks, a));
    add_vec3 (vertices, color);

    GLfloat b[3];
    vec3_add (b, a, v0);
    add_vec3 (vertices, b);
    vec3_mult (color, face_color, ambient_shade (width, height, depth, blocks, b));
    add_vec3 (vertices, color);

    GLfloat c[3];
    vec3_add (c, b, v1);
    add_vec3 (vertices, c);
    vec3_mult (color, face_color, ambient_shade (width, height, depth, blocks, c));
    add_vec3 (vertices, color);

    GLfloat d[3];
    vec3_add (d, a, v1);
    add_vec3 (vertices, d);
    vec3_mult (color, face_color, ambient_shade (width, height, depth, blocks, d));
    add_vec3 (vertices, color);

    add_uint (triangles, start + 0);
    add_uint (triangles, start + 1);
    add_uint (triangles, start + 2);
    add_uint (triangles, start + 0);
    add_uint (triangles, start + 2);
    add_uint (triangles, start + 3);
}

static void
setup (PvRenderer *self)
{
    if (self->vao != 0)
        return;

    glGenVertexArrays (1, &self->vao);
    glBindVertexArray (self->vao);

    gsize width = pv_map_get_width (self->map);
    gsize height = pv_map_get_height (self->map);
    gsize depth = pv_map_get_depth (self->map);

    g_autofree guint16 *blocks = g_malloc (sizeof (guint16) * width * height * depth);
    pv_map_get_blocks (self->map, 0, 0, 0, width, height, depth, blocks);

    gsize block_count = pv_map_get_block_count (self->map);
    g_autofree gfloat *colors = g_malloc (sizeof (gfloat) * block_count * 3);
    gsize offset = 0;
    for (gsize block_id = 0; block_id < block_count; block_id++) {
        guint8 red, green, blue;
        pv_map_get_block_color (self->map, block_id, &red, &green, &blue);
        colors[offset + 0] = red / 255.0;
        colors[offset + 1] = green / 255.0;
        colors[offset + 2] = blue / 255.0;
        offset += 3;
    }

    GLfloat north[3] = {  1,  0,  0 };
    GLfloat south[3] = { -1,  0,  0 };
    GLfloat east[3]  = {  0,  1,  0 };
    GLfloat west[3]  = {  0, -1,  0 };
    GLfloat up[3]    = {  0,  0,  1 };
    GLfloat down[3]  = {  0,  0, -1 };

    /* Calculate triangles looking on each side.
     * Order from nearest to furtherest (so later triangles get rejected in the depth buffer) */
    g_autoptr(GArray) vertices = g_array_new (FALSE, FALSE, sizeof (GLfloat));
    g_autoptr(GArray) triangles = g_array_new (FALSE, FALSE, sizeof (GLuint));
    self->north_offset = 0;
    self->north_size = 0;
    for (int x = 0; x < width; x++) {
        for (int y = height; y >= 0; y--) {
            for (int z = 0; z < depth; z++) {
                guint16 block_id = get_block (width, height, depth, blocks, x, y, z);
                if (block_id == 0)
                    continue;
                if (get_block (width, height, depth, blocks, x, y + 1, z) != 0)
                    continue;

                GLfloat top_pos[3] = { x + 1, y + 1, z + 1 };
                add_square (width, height, depth, blocks, vertices, triangles, top_pos, south, down, colors + block_id * 3);
                self->north_size += 2;
                if (y + 1 > self->north)
                    self->north = y + 1;
            }
        }
    }
    self->south_offset = self->north_offset + self->north_size;
    self->south_size = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < depth; z++) {
                guint16 block_id = get_block (width, height, depth, blocks, x, y, z);
                if (block_id == 0)
                    continue;
                if (get_block (width, height, depth, blocks, x, y - 1, z) != 0)
                    continue;

                GLfloat base_pos[3] = { x, y, z };
                add_square (width, height, depth, blocks, vertices, triangles, base_pos, up, north, colors + block_id * 3);
                self->south_size += 2;
                if (y < self->south)
                    self->south = y;
            }
        }
    }
    self->east_offset = self->south_offset + self->south_size;
    self->east_size = 0;
    for (int x = width; x >= 0; x--) {
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < depth; z++) {
                guint16 block_id = get_block (width, height, depth, blocks, x, y, z);
                if (block_id == 0)
                    continue;
                if (get_block (width, height, depth, blocks, x + 1, y, z) != 0)
                    continue;

                GLfloat top_pos[3] = { x + 1, y + 1, z + 1 };
                add_square (width, height, depth, blocks, vertices, triangles, top_pos, down, west, colors + block_id * 3);
                self->east_size += 2;
                if (x > self->east)
                    self->east = x;
            }
        }
    }
    self->west_offset = self->east_offset + self->east_size;
    self->west_size = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < depth; z++) {
                guint16 block_id = get_block (width, height, depth, blocks, x, y, z);
                if (block_id == 0)
                    continue;
                if (get_block (width, height, depth, blocks, x - 1, y, z) != 0)
                    continue;

                GLfloat base_pos[3] = { x, y, z };
                add_square (width, height, depth, blocks, vertices, triangles, base_pos, east, up, colors + block_id * 3);
                self->west_size += 2;
                if (x < self->west)
                    self->west = x;
            }
        }
    }
    self->top_offset = self->west_offset + self->west_size;
    self->top_size = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int z = depth; z >= 0; z--) {
                guint16 block_id = get_block (width, height, depth, blocks, x, y, z);
                if (block_id == 0)
                    continue;
                if (get_block (width, height, depth, blocks, x, y, z + 1) != 0)
                    continue;

                GLfloat top_pos[3] = { x + 1, y + 1, z + 1 };
                add_square (width, height, depth, blocks, vertices, triangles, top_pos, west, south, colors + block_id * 3);
                self->top_size += 2;
                if (z + 1 > self->top)
                    self->top = z + 1;
            }
        }
    }
    self->bottom_offset = self->top_offset + self->top_size;
    self->bottom_size = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < depth; z++) {
                guint16 block_id = get_block (width, height, depth, blocks, x, y, z);
                if (block_id == 0)
                    continue;
                if (get_block (width, height, depth, blocks, x, y, z - 1) != 0)
                    continue;

                GLfloat base_pos[3] = { x, y, z };
                add_square (width, height, depth, blocks, vertices, triangles, base_pos, north, east, colors + block_id * 3);
                self->bottom_size += 2;
                if (z < self->bottom)
                    self->bottom = z;
            }
        }
    }

    GLuint vertex_buffer;
    glGenBuffers (1, &vertex_buffer);
    glBindBuffer (GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData (GL_ARRAY_BUFFER, vertices->len * sizeof (GLfloat), vertices->data, GL_STATIC_DRAW);

    GLuint triangle_buffer;
    glGenBuffers (1, &triangle_buffer);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, triangle_buffer);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, triangles->len * sizeof (GLuint), triangles->data, GL_STATIC_DRAW);

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
    glVertexAttribPointer (position_attr, 3, GL_FLOAT, GL_FALSE, 24, (void *)0);
    GLint color_attr = glGetAttribLocation (self->program, "color");
    glEnableVertexAttribArray (color_attr);
    glVertexAttribPointer (color_attr, 3, GL_FLOAT, GL_FALSE, 24, (void*)12);

    if (self->gl_renderer == NULL) {
        self->gl_renderer = g_strdup ((gchar *) glGetString (GL_RENDERER));
        g_printerr ("Renderer: %s\n", self->gl_renderer);
    }
}

static void
pv_renderer_dispose (GObject *object)
{
    PvRenderer *self = PV_RENDERER (object);

    g_clear_pointer (&self->gl_renderer, g_free);
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

    GLint v_location = glGetUniformLocation (self->program, "ViewMatrix");
    GLint vp_location = glGetUniformLocation (self->program, "ViewProjectionMatrix");
    pv_camera_transform (self->camera, width, height, v_location, vp_location);

    GLint normal_location = glGetUniformLocation (self->program, "Normal");
    GLint shade_location = glGetUniformLocation (self->program, "Shade");

    glBindVertexArray (self->vao);

    GLint n_triangles = 0;
    GLfloat x, y, z;
    pv_camera_get_position (self->camera, &x, &y, &z);

    GLfloat light_direction[3] = { 1, 1, -1 };

    if (x > self->west) {
        GLfloat normal[3] = { -1, 0, 0 };
        glUniform3f (normal_location, normal[0], normal[1], normal[2]);
        glUniform1f (shade_location, MAX (vec3_dot (light_direction, normal), 0.4));
        glDrawElements (GL_TRIANGLES, self->east_size * 3, GL_UNSIGNED_INT, (const GLvoid *) (self->east_offset * 3 * 4));
        n_triangles += self->east_size;
    }
    if (x < self->east) {
        GLfloat normal[3] = { 1, 0, 0 };
        glUniform3f (normal_location, normal[0], normal[1], normal[2]);
        glUniform1f (shade_location, MAX (vec3_dot (light_direction, normal), 0.4));
        glDrawElements (GL_TRIANGLES, self->west_size * 3, GL_UNSIGNED_INT, (const GLvoid *) (self->west_offset * 3 * 4));
        n_triangles += self->west_size;
    }

    if (y > self->south) {
        GLfloat normal[3] = { 0, -1, 0 };
        glUniform3f (normal_location, normal[0], normal[1], normal[2]);
        glUniform1f (shade_location, MAX (vec3_dot (light_direction, normal), 0.4));
        glDrawElements (GL_TRIANGLES, self->north_size * 3, GL_UNSIGNED_INT, (const GLvoid *) (self->north_offset * 3 * 4));
        n_triangles += self->north_size;
    }
    if (y < self->north) {
        GLfloat normal[3] = { 0, 1, 0 };
        glUniform3f (normal_location, normal[0], normal[1], normal[2]);
        glUniform1f (shade_location, MAX (vec3_dot (light_direction, normal), 0.4));
        glDrawElements (GL_TRIANGLES, self->south_size * 3, GL_UNSIGNED_INT, (const GLvoid *) (self->south_offset * 3 * 4));
        n_triangles += self->south_size;
    }

    if (z > self->bottom) {
        GLfloat normal[3] = { 0, 0, -1 };
        glUniform3f (normal_location, normal[0], normal[1], normal[2]);
        glUniform1f (shade_location, MAX (vec3_dot (light_direction, normal), 0.4));
        glDrawElements (GL_TRIANGLES, self->top_size * 3, GL_UNSIGNED_INT, (const GLvoid *) (self->top_offset * 3 * 4));
        n_triangles += self->top_size;
    }
    if (z < self->top) {
        GLfloat normal[3] = { 0, 0, 1 };
        glUniform3f (normal_location, normal[0], normal[1], normal[2]);
        glUniform1f (shade_location, MAX (vec3_dot (light_direction, normal), 0.4));
        glDrawElements (GL_TRIANGLES, self->bottom_size * 3, GL_UNSIGNED_INT, (const GLvoid *) (self->bottom_offset * 3 * 4));
        n_triangles += self->bottom_size;
    }

    g_printerr ("Rendered %d triangles\n", n_triangles);
}

const gchar *
pv_renderer_get_renderer (PvRenderer *self)
{
    g_return_val_if_fail (PV_IS_RENDERER (self), NULL);
    return self->gl_renderer;
}
