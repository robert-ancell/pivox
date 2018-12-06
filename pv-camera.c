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
#include "pv-matrix.h"
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
