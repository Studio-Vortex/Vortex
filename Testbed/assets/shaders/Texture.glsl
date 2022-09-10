// Basic Texture Shader

#type vertex
#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_TexCoord;

out vec4 f_Color;
out vec2 f_TexCoord;

uniform mat4 u_ViewProjection;

void main()
{
	f_Color = a_Color;
	f_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 gl_Color;

in vec4 f_Color;
in vec2 f_TexCoord;

uniform vec4 u_Color;
uniform int u_TexScale;
uniform sampler2D u_Texture;

void main()
{
	//gl_Color = texture(u_Texture, f_TexCoord * u_TexScale) * u_Color;
	gl_Color = f_Color;
}