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
vector_make (GLfloat *v,
             GLfloat  x,
             GLfloat  y,
             GLfloat  z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
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
vector_cross (GLfloat *v,
              GLfloat *a,
              GLfloat *b)
{
    vector_make (v,
                 a[1]*b[2] - a[2]*b[1],
                 a[2]*b[0] - a[0]*b[2],
                 a[0]*b[1] - a[1]*b[0]);
}

static void
matrix_make (GLfloat *m,
             GLfloat v00, GLfloat v10, GLfloat v20, GLfloat v30,
             GLfloat v01, GLfloat v11, GLfloat v21, GLfloat v31,
             GLfloat v02, GLfloat v12, GLfloat v22, GLfloat v32,
             GLfloat v03, GLfloat v13, GLfloat v23, GLfloat v33)
{
    m[ 0] = v00;
    m[ 1] = v10;
    m[ 2] = v20;
    m[ 3] = v30;
    m[ 4] = v01;
    m[ 5] = v11;
    m[ 6] = v21;
    m[ 7] = v31;
    m[ 8] = v02;
    m[ 9] = v12;
    m[10] = v22;
    m[11] = v32;
    m[12] = v03;
    m[13] = v13;
    m[14] = v23;
    m[15] = v33;
}

static void
matrix_make_projection (GLfloat *m,
                        GLfloat  fov_y,
                        GLfloat  aspect,
                        GLfloat  z_near,
                        GLfloat  z_far)
{
    GLfloat f = atanf (fov_y / 2.0f);
    GLfloat a = f / aspect;
    GLfloat b = (z_far + z_near) / (z_near - z_far);
    GLfloat c = (2.0f * z_far * z_near) / (z_near - z_far);
    matrix_make ( m,
                  a,  0,  0,  0,
                  0,  f,  0,  0,
                  0,  0,  b,  c,
                  0,  0, -1,  0);
}

static void
matrix_make_direction (GLfloat *m,
                       GLfloat *dir,
                       GLfloat *up)
{
    GLfloat s[3];
    vector_cross (s, dir, up);
    GLfloat u[3];
    vector_cross (u, dir, s);
    matrix_make (m,
                    s[0],    s[1],    s[2], 0,
                    u[0],    u[1],    u[2], 0,
                 -dir[0], -dir[1], -dir[2], 0,
                       0,       0,       0, 1);
}

static void
matrix_mult (GLfloat *m, GLfloat *a, GLfloat *b)
{
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            m[col * 4 + row] = 0;
            for (int i = 0; i < 4; i++)
                m[col * 4 + row] += a[col * 4 + i] * b[i * 4 + row];
        }
    }
}

static void
matrix_make_translate (GLfloat *m,
                       GLfloat x, GLfloat y, GLfloat z)
{
    matrix_make (m,
                 1, 0, 0, x,
                 0, 1, 0, y,
                 0, 0, 1, z,
                 0, 0, 0, 1);
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
    matrix_make_projection (proj, M_PI / 3.0f, (GLfloat) width / height, 0.1f, 100.0f);

    GLfloat trans[16];
    matrix_make_translate (trans, self->x, self->y, self->z);
    GLfloat rot[16];
    GLfloat up[3], dir[3];
    vector_make (up, 0, 0, 1);
    vector_make (dir,
                 self->target_x - self->x,
                 self->target_y - self->y,
                 self->target_z - self->z);
    vector_normalize (dir);
    matrix_make_direction (rot, dir, up);
    GLfloat t[16];
    matrix_mult (t, proj, rot);
    GLfloat mvp[16];
    matrix_mult (mvp, t, trans);

    glUniformMatrix4fv (uniform_location, 1, GL_TRUE, mvp);
}
