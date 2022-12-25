//-------------------------
// - Vortex Game Engine Renderer Stencil Shader -
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3 a_Position;

void main()
{
    gl_Position = vec4(0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;

uniform vec3 u_StencilColor;

void main()
{
    o_Color = vec4(u_StencilColor, 1.0);
}