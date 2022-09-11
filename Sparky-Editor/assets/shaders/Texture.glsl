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
	vec4 texColor = f_Color;
	 
	 switch (int(f_TexIndex))
	 {
		case 0:  texColor *= texture(u_Textures[0],  f_TexCoord * f_TexScale); break;
		case 1:  texColor *= texture(u_Textures[1],  f_TexCoord * f_TexScale); break;
		case 2:  texColor *= texture(u_Textures[2],  f_TexCoord * f_TexScale); break;
		case 3:  texColor *= texture(u_Textures[3],  f_TexCoord * f_TexScale); break;
		case 4:  texColor *= texture(u_Textures[4],  f_TexCoord * f_TexScale); break;
		case 5:  texColor *= texture(u_Textures[5],  f_TexCoord * f_TexScale); break;
		case 6:  texColor *= texture(u_Textures[6],  f_TexCoord * f_TexScale); break;
		case 7:  texColor *= texture(u_Textures[7],  f_TexCoord * f_TexScale); break;
		case 8:  texColor *= texture(u_Textures[8],  f_TexCoord * f_TexScale); break;
		case 9:  texColor *= texture(u_Textures[9],  f_TexCoord * f_TexScale); break;
		case 10: texColor *= texture(u_Textures[10], f_TexCoord * f_TexScale); break;
		case 11: texColor *= texture(u_Textures[11], f_TexCoord * f_TexScale); break;
		case 12: texColor *= texture(u_Textures[12], f_TexCoord * f_TexScale); break;
		case 13: texColor *= texture(u_Textures[13], f_TexCoord * f_TexScale); break;
		case 14: texColor *= texture(u_Textures[14], f_TexCoord * f_TexScale); break;
		case 15: texColor *= texture(u_Textures[15], f_TexCoord * f_TexScale); break;
		case 16: texColor *= texture(u_Textures[16], f_TexCoord * f_TexScale); break;
		case 17: texColor *= texture(u_Textures[17], f_TexCoord * f_TexScale); break;
		case 18: texColor *= texture(u_Textures[18], f_TexCoord * f_TexScale); break;
		case 19: texColor *= texture(u_Textures[19], f_TexCoord * f_TexScale); break;
		case 20: texColor *= texture(u_Textures[20], f_TexCoord * f_TexScale); break;
		case 21: texColor *= texture(u_Textures[21], f_TexCoord * f_TexScale); break;
		case 22: texColor *= texture(u_Textures[22], f_TexCoord * f_TexScale); break;
		case 23: texColor *= texture(u_Textures[23], f_TexCoord * f_TexScale); break;
		case 24: texColor *= texture(u_Textures[24], f_TexCoord * f_TexScale); break;
		case 25: texColor *= texture(u_Textures[25], f_TexCoord * f_TexScale); break;
		case 26: texColor *= texture(u_Textures[26], f_TexCoord * f_TexScale); break;
		case 27: texColor *= texture(u_Textures[27], f_TexCoord * f_TexScale); break;
		case 28: texColor *= texture(u_Textures[28], f_TexCoord * f_TexScale); break;
		case 29: texColor *= texture(u_Textures[29], f_TexCoord * f_TexScale); break;
		case 30: texColor *= texture(u_Textures[30], f_TexCoord * f_TexScale); break;
		case 31: texColor *= texture(u_Textures[31], f_TexCoord * f_TexScale); break;
	 }

	gl_Color = texColor;
}