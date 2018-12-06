#version 330

attribute vec3 position;
attribute vec3 color;
attribute float shade;

out vec3 Vertex;
out vec3 Color;

uniform vec3 Normal;
uniform mat4 ViewMatrix;
uniform mat4 ViewProjectionMatrix;

void main ()
{
   Color = color * shade;
   gl_Position = ViewProjectionMatrix * vec4 (position.xyz, 1.0);
};
