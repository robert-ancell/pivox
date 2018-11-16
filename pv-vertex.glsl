#version 330

attribute vec3 position;

uniform mat4 MVP;

void main ()
{
   gl_Position = MVP * vec4 (position.xyz, 1.0);
};
