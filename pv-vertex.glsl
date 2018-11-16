#version 330

attribute vec4 position;

out vec3 Color;

uniform mat4 MVP;

void main ()
{
   Color = vec3 (1.0, 0.5, 0.5);
   gl_Position = MVP * vec4 (position.xyz, 1.0);
};
