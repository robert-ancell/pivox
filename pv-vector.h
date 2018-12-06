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

#pragma once

static inline void
vec3_make (GLfloat *v,
           GLfloat  x,
           GLfloat  y,
           GLfloat  z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

static inline void
vec3_copy (GLfloat *v,
           GLfloat *a)
{
    v[0] = a[0];
    v[1] = a[1];
    v[2] = a[2];
}

static inline void
vec3_normalize (GLfloat *v)
{
    GLfloat l = sqrtf (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (l == 0)
        return;
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}

static inline GLfloat
vec3_dot (GLfloat *a,
          GLfloat *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static inline void
vec3_add (GLfloat *v,
          GLfloat *a,
          GLfloat *b)
{
    v[0] = a[0] + b[0];
    v[1] = a[1] + b[1];
    v[2] = a[2] + b[2];
}

static inline void
vec3_mult (GLfloat *v,
           GLfloat *a,
           GLfloat b)
{
    v[0] = a[0] * b;
    v[1] = a[1] * b;
    v[2] = a[2] * b;
}

static inline void
vec3_cross (GLfloat *v,
            GLfloat *a,
            GLfloat *b)
{
    vec3_make (v,
               a[1]*b[2] - a[2]*b[1],
               a[2]*b[0] - a[0]*b[2],
               a[0]*b[1] - a[1]*b[0]);
}
