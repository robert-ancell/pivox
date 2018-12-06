#version 330

uniform float ShadeLevel;
in vec3 Color;

void main ()
{
  gl_FragColor = vec4 (Color, 1.0) * ShadeLevel;
};
