// Basic Texture Shader

#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 f_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	f_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 gl_Color;

in vec2 f_TexCoord;

uniform vec4 u_TintColor;
uniform int u_TileScale;
uniform sampler2D u_Texture;

void main()
{
	gl_Color = texture(u_Texture, f_TexCoord * u_TileScale) * u_TintColor;
}