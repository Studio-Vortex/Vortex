//-------------------------
// - Vortex Game Engine Renderer Stencil Shader -
//-------------------------

#type vertex

layout (location = 0) in vec3 a_Position;

void main()
{
    
}


#type fragment

layout (location = 0) out vec4 o_Color;

uniform vec3 u_StencilColor;

void main()
{
    o_Color = vec4(u_StencilColor, 1.0);
}