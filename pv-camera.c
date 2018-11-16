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
#include <math.h>

#include "pv-camera.h"

struct _PvCamera
{
    GObject parent_instance;

    gfloat  x;
    gfloat  y;
    gfloat  z;
    gfloat  target_x;
    gfloat  target_y;
    gfloat  target_z;
};

G_DEFINE_TYPE (PvCamera, pv_camera, G_TYPE_OBJECT)

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
pv_camera_class_init (PvCameraClass *klass)
{
}

void
pv_camera_init (PvCamera *self)
{
}

PvCamera *
pv_camera_new (void)
{
    return g_object_new (pv_camera_get_type (), NULL);
}

void
pv_camera_set_position (PvCamera *self,
                        gfloat    x,
                        gfloat    y,
                        gfloat    z)
{
    g_return_if_fail (PV_IS_CAMERA (self));
    self->x = x;
    self->y = y;
    self->z = z;
}

void
pv_camera_set_target (PvCamera *self,
                      gfloat    x,
                      gfloat    y,
                      gfloat    z)
{
    g_return_if_fail (PV_IS_CAMERA (self));
    self->target_x = x;
    self->target_y = y;
    self->target_z = z;
}

void
pv_camera_transform (PvCamera *self,
                     gint      width,
                     gint      height,
                     gint      uniform_location)
{
    g_return_if_fail (PV_IS_CAMERA (self));

    GLfloat proj[16];
    matrix_make_projection (M_PI / 3.0f, (GLfloat) width / height, 0.1f, 100.0f, proj);

    GLfloat trans[16];
    matrix_translate (self->x, self->y, self->z, trans);
    GLfloat rot[16];
    GLfloat up[3], dir[3];
    vector_make (0, 0, 1, up);
    vector_make (self->target_x - self->x,
                 self->target_y - self->y,
                 self->target_z - self->z,
                 dir);
    vector_normalize (dir);
    matrix_make_direction (dir, up, rot);
    GLfloat t[16];
    matrix_mult (proj, rot, t);
    GLfloat mvp[16];
    matrix_mult (t, trans, mvp);

    glUniformMatrix4fv (uniform_location, 1, GL_TRUE, mvp);
}
