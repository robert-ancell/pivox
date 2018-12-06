#version 330

attribute vec3 position;
attribute vec3 color;

out vec3 Color;

uniform vec3 Normal; // FIXME: Not used
uniform mat4 ViewMatrix; // FIXME: Not used
uniform mat4 ViewProjectionMatrix;
uniform float Shade;

void main ()
{
   Color = color * Shade;
   gl_Position = ViewProjectionMatrix * vec4 (position.xyz, 1.0);
};
