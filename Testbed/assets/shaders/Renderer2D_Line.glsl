//-------------------------
// - Sparky Game Engine -
// Renderer2D Line Shader
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position;
layout (location = 1) in vec4  a_Color;
layout (location = 2) in int   a_EntityID;

out vec4     f_Color;
out flat int f_EntityID;

uniform mat4 u_ViewProjection;

void main()
{
	f_Color = a_Color;

	f_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

in vec4     f_Color;
in flat int f_EntityID;

void main()
{
    // Set output color
    o_Color = f_Color;

	o_EntityID = f_EntityID;
}