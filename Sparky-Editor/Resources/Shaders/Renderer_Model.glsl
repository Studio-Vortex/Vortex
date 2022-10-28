//-------------------------
// - Sparky Game Engine -
// Renderer Model Shader with Texturing
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position; // Vertex position
layout (location = 1) in vec3  a_Normal;   // Vertex normal
layout (location = 2) in vec4  a_Color;    // Vertex color
layout (location = 3) in vec2  a_TexCoord; // Vertex texture coordinate
layout (location = 4) in float a_TexScale; // Texture scale
layout (location = 5) in int   a_EntityID; // Vertex Entity ID

out vec3       f_Normal;
out vec4       f_Color;
out vec2       f_TexCoord;
out float      f_TexScale;
out flat int   f_EntityID;

uniform highp mat4 u_ViewProjection;

void main()
{
	f_Normal = a_Normal;
	f_Color = a_Color;
	f_TexCoord = a_TexCoord;
	f_TexScale = a_TexScale;
	
	f_EntityID = a_EntityID;
	
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

in vec3       f_Normal;
in vec4       f_Color;
in vec2       f_TexCoord;
in float      f_TexScale;
in flat int   f_EntityID;

uniform sampler2D u_Texture;

void main()
{
	vec4 texColor = f_Color * texture(u_Texture, f_TexCoord * f_TexScale);

	// Discard the pixel/fragment if it has an alpha of zero
	if (texColor.a == 0.0)
		discard;

	// Set output color
	o_Color = texColor;

	o_EntityID = f_EntityID;
}