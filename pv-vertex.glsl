#version 330

attribute vec3 position;
attribute vec3 color;

out vec3 Color;

uniform mat4 MVP;

void main ()
{
   gl_Position = MVP * vec4 (position.xyz, 1.0);
   Color = color;
};
