//-------------------------
// - Vortex Game Engine Renderer Stencil Shader -
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
uniform float u_OutlineThickness;

void main()
{
    gl_Position = u_ViewProjection * u_Model * u_OutlineThickness * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;

uniform vec3 u_StencilColor;

void main()
{
    o_Color = vec4(u_StencilColor, 1.0);
}