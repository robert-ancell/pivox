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
#include "pv-vector.h"

struct _PvCamera
{
    GObject  parent_instance;

    GLfloat  pos[3];
    GLfloat  target[3];
    gboolean target_absolute;
};

G_DEFINE_TYPE (PvCamera, pv_camera, G_TYPE_OBJECT)

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

/*static void
matrix_transpose (GLfloat *m, GLfloat *a)
{
    matrix_make (m,
                 a[0], a[4], a[ 8], a[12],
                 a[1], a[5], a[ 9], a[13],
                 a[2], a[6], a[10], a[14],
                 a[3], a[7], a[11], a[15]);
}*/

/*static gboolean
matrix_invert (GLfloat *m, GLfloat *a)
{
    GLfloat inv[16], det;
    int i;

    inv[0] = a[5]  * a[10] * a[15] -
             a[5]  * a[11] * a[14] -
             a[9]  * a[6]  * a[15] +
             a[9]  * a[7]  * a[14] +
             a[13] * a[6]  * a[11] -
             a[13] * a[7]  * a[10];

    inv[4] = -a[4]  * a[10] * a[15] +
              a[4]  * a[11] * a[14] +
              a[8]  * a[6]  * a[15] -
              a[8]  * a[7]  * a[14] -
              a[12] * a[6]  * a[11] +
              a[12] * a[7]  * a[10];

    inv[8] = a[4]  * a[9] * a[15] -
             a[4]  * a[11] * a[13] -
             a[8]  * a[5] * a[15] +
             a[8]  * a[7] * a[13] +
             a[12] * a[5] * a[11] -
             a[12] * a[7] * a[9];

    inv[12] = -a[4]  * a[9] * a[14] +
               a[4]  * a[10] * a[13] +
               a[8]  * a[5] * a[14] -
               a[8]  * a[6] * a[13] -
               a[12] * a[5] * a[10] +
               a[12] * a[6] * a[9];

    inv[1] = -a[1]  * a[10] * a[15] +
              a[1]  * a[11] * a[14] +
              a[9]  * a[2] * a[15] -
              a[9]  * a[3] * a[14] -
              a[13] * a[2] * a[11] +
              a[13] * a[3] * a[10];

    inv[5] = a[0]  * a[10] * a[15] -
             a[0]  * a[11] * a[14] -
             a[8]  * a[2] * a[15] +
             a[8]  * a[3] * a[14] +
             a[12] * a[2] * a[11] -
             a[12] * a[3] * a[10];

    inv[9] = -a[0]  * a[9] * a[15] +
              a[0]  * a[11] * a[13] +
              a[8]  * a[1] * a[15] -
              a[8]  * a[3] * a[13] -
              a[12] * a[1] * a[11] +
              a[12] * a[3] * a[9];

    inv[13] = a[0]  * a[9] * a[14] -
              a[0]  * a[10] * a[13] -
              a[8]  * a[1] * a[14] +
              a[8]  * a[2] * a[13] +
              a[12] * a[1] * a[10] -
              a[12] * a[2] * a[9];

    inv[2] = a[1]  * a[6] * a[15] -
             a[1]  * a[7] * a[14] -
             a[5]  * a[2] * a[15] +
             a[5]  * a[3] * a[14] +
             a[13] * a[2] * a[7] -
             a[13] * a[3] * a[6];

    inv[6] = -a[0]  * a[6] * a[15] +
              a[0]  * a[7] * a[14] +
              a[4]  * a[2] * a[15] -
              a[4]  * a[3] * a[14] -
              a[12] * a[2] * a[7] +
              a[12] * a[3] * a[6];

    inv[10] = a[0]  * a[5] * a[15] -
              a[0]  * a[7] * a[13] -
              a[4]  * a[1] * a[15] +
              a[4]  * a[3] * a[13] +
              a[12] * a[1] * a[7] -
              a[12] * a[3] * a[5];

    inv[14] = -a[0]  * a[5] * a[14] +
               a[0]  * a[6] * a[13] +
               a[4]  * a[1] * a[14] -
               a[4]  * a[2] * a[13] -
               a[12] * a[1] * a[6] +
               a[12] * a[2] * a[5];

    inv[3] = -a[1] * a[6] * a[11] +
              a[1] * a[7] * a[10] +
              a[5] * a[2] * a[11] -
              a[5] * a[3] * a[10] -
              a[9] * a[2] * a[7] +
              a[9] * a[3] * a[6];

    inv[7] = a[0] * a[6] * a[11] -
             a[0] * a[7] * a[10] -
             a[4] * a[2] * a[11] +
             a[4] * a[3] * a[10] +
             a[8] * a[2] * a[7] -
             a[8] * a[3] * a[6];

    inv[11] = -a[0] * a[5] * a[11] +
               a[0] * a[7] * a[9] +
               a[4] * a[1] * a[11] -
               a[4] * a[3] * a[9] -
               a[8] * a[1] * a[7] +
               a[8] * a[3] * a[5];

    inv[15] = a[0] * a[5] * a[10] -
              a[0] * a[6] * a[9] -
              a[4] * a[1] * a[10] +
              a[4] * a[2] * a[9] +
              a[8] * a[1] * a[6] -
              a[8] * a[2] * a[5];

    det = a[0] * inv[0] + a[1] * inv[4] + a[2] * inv[8] + a[3] * inv[12];

    if (det == 0)
        return FALSE;

    det = 1.0 / det;
    for (i = 0; i < 16; i++)
        m[i] = inv[i] * det;

    return TRUE;
}*/

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
    vector_cross (u, s, dir);
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

/*static void
matrix_mult_v (GLfloat *v, GLfloat *m, GLfloat *a)
{
    for (int col = 0; col < 4; col++) {
        v[col] = 0;
        for (int row = 0; row < 4; row++)
            v[col] += m[col * 4 + row] * a[row];
    }
}*/

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

static void
get_direction (PvCamera *self,
               GLfloat  *dir)
{
    if (self->target_absolute) {
        vector_make (dir,
                     self->target[0] - self->pos[0],
                     self->target[1] - self->pos[1],
                     self->target[2] - self->pos[2]);
        vector_normalize (dir);
    }
    else {
        vector_copy (dir, self->target);
    }
}

void
pv_camera_class_init (PvCameraClass *klass)
{
}

void
pv_camera_init (PvCamera *self)
{
   vector_make (self->target, 1, 0, 0);
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
    vector_make (self->pos, x, y, z);
}

void
pv_camera_get_position (PvCamera *self,
                        gfloat   *x,
                        gfloat   *y,
                        gfloat   *z)
{
    g_return_if_fail (PV_IS_CAMERA (self));
    if (x != NULL)
        *x = self->pos[0];
    if (y != NULL)
        *y = self->pos[1];
    if (z != NULL)
        *z = self->pos[2];
}

void
pv_camera_set_direction (PvCamera *self,
                         gfloat    x,
                         gfloat    y,
                         gfloat    z)
{
    g_return_if_fail (PV_IS_CAMERA (self));
    vector_make (self->target, x, y, z);
    self->target_absolute = FALSE;
}

void
pv_camera_get_direction (PvCamera *self,
                         gfloat   *x,
                         gfloat   *y,
                         gfloat   *z)
{
    g_return_if_fail (PV_IS_CAMERA (self));
    GLfloat dir[3];
    get_direction (self, dir);
    if (x != NULL)
        *x = dir[0];
    if (y != NULL)
        *y = dir[1];
    if (z != NULL)
        *z = dir[2];
}

void
pv_camera_set_target (PvCamera *self,
                      gfloat    x,
                      gfloat    y,
                      gfloat    z)
{
    g_return_if_fail (PV_IS_CAMERA (self));
    vector_make (self->target, x, y, z);
    self->target_absolute = TRUE;
}

void
pv_camera_transform (PvCamera *self,
                     gint      width,
                     gint      height,
                     gint      v_location,
                     gint      vp_location)
{
    g_return_if_fail (PV_IS_CAMERA (self));

    GLfloat trans[16];
    matrix_make_translate (trans, -self->pos[0], -self->pos[1], -self->pos[2]);

    GLfloat rot[16];
    GLfloat up[3];
    vector_make (up, 0, 0, 1);
    GLfloat dir[3];
    get_direction (self, dir);
    matrix_make_direction (rot, dir, up);

    GLfloat v[16];
    matrix_mult (v, rot, trans);
    glUniformMatrix4fv (v_location, 1, GL_TRUE, v);

    GLfloat proj[16];
    matrix_make_projection (proj, M_PI / 3.0f, (GLfloat) width / height, 0.1f, 100.0f);

    GLfloat vp[16];
    matrix_mult (vp, proj, v);
    glUniformMatrix4fv (vp_location, 1, GL_TRUE, vp);
}
