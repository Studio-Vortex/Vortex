// Basic Texture Shader

#type vertex
#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in float a_TexIndex;
layout (location = 4) in float a_TexScale;

out vec4 f_Color;
out vec2 f_TexCoord;
out float f_TexIndex;
out float f_TexScale;

uniform mat4 u_ViewProjection;

void main()
{
	f_Color = a_Color;
	f_TexCoord = a_TexCoord;
	f_TexIndex = a_TexIndex;
	f_TexScale = a_TexScale;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 gl_Color;

in vec4 f_Color;
in vec2 f_TexCoord;
in float f_TexIndex;
in float f_TexScale;

uniform sampler2D u_Textures[32];

void main()
{
	gl_Color = texture(u_Textures[int(f_TexIndex)], f_TexCoord * f_TexScale) * f_Color;
}