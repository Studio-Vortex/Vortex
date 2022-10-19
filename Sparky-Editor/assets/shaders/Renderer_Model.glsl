//-------------------------
// - Sparky Game Engine -
// Renderer Cube Shader with Texturing
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec4  a_Position; // Vertex position
layout (location = 1) in vec4  a_Color;     // Vertex color
layout (location = 2) in vec3  a_Normal;   // Vertex normal
layout (location = 3) in vec2  a_TexCoord; // Vertex texture coordinate
layout (location = 4) in int   a_EntityID; // Vertex Entity ID

out vec4       f_Color;
out vec3       f_Normal;
out vec2       f_TexCoord;
out flat int   f_EntityID;

uniform mat4 u_ViewProjection;

void main()
{
	f_Color = a_Color;
	f_Normal = a_Normal;
	f_TexCoord = a_TexCoord;
	
	f_EntityID = a_EntityID;
	
	gl_Position = u_ViewProjection * a_Position;
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

in vec4       f_Color;
in vec3       f_Normal;
in vec2       f_TexCoord;
in flat int   f_EntityID;

void main()
{
	// Set output color
	o_Color = f_Color;

	o_EntityID = f_EntityID;
}