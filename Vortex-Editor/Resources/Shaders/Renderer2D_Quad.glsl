//-------------------------
// - Sparky Game Engine -
// Renderer2D Quad Shader with Texturing
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position; // Vertex position
layout (location = 1) in vec4  a_Color; // Vertex color
layout (location = 2) in vec2  a_TexCoord; // Vertex texture coordinate
layout (location = 3) in float a_TexIndex; // Texture index out of 32
layout (location = 4) in vec2  a_TexScale; // Texture scale
layout (location = 5) in int   a_EntityID; // Vertex Entity ID

out DATA
{
	vec4        Color;
	vec2        TexCoord;
	flat float  TexIndex;
	vec2        TexScale;
	flat int    EntityID;
} vertexOut;

uniform mat4 u_ViewProjection;

void main()
{
	vertexOut.Color = a_Color;
	vertexOut.TexCoord = a_TexCoord;
	vertexOut.TexIndex = a_TexIndex;
	vertexOut.TexScale = a_TexScale;
	
	vertexOut.EntityID = a_EntityID;
	
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;
layout (location = 2) out vec4 o_BrightColor;

in DATA
{
	vec4       Color;
	vec2       TexCoord;
	flat float TexIndex;
	vec2       TexScale;
	flat int   EntityID;

} fragmentIn;

struct LightSource
{
	vec3 Color;
	vec3 Position;
	float Intensity;
};

#define MAX_LIGHT_SOURCES 100

uniform sampler2D   u_Textures[32];
uniform LightSource u_LightSources[MAX_LIGHT_SOURCES];

void main()
{
	vec4 texColor = fragmentIn.Color;
	 
	switch (int(fragmentIn.TexIndex))
	{
		case  0: texColor *= texture(u_Textures[ 0], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case  1: texColor *= texture(u_Textures[ 1], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case  2: texColor *= texture(u_Textures[ 2], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case  3: texColor *= texture(u_Textures[ 3], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case  4: texColor *= texture(u_Textures[ 4], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case  5: texColor *= texture(u_Textures[ 5], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case  6: texColor *= texture(u_Textures[ 6], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case  7: texColor *= texture(u_Textures[ 7], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case  8: texColor *= texture(u_Textures[ 8], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case  9: texColor *= texture(u_Textures[ 9], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 10: texColor *= texture(u_Textures[10], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 11: texColor *= texture(u_Textures[11], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 12: texColor *= texture(u_Textures[12], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 13: texColor *= texture(u_Textures[13], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 14: texColor *= texture(u_Textures[14], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 15: texColor *= texture(u_Textures[15], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 16: texColor *= texture(u_Textures[16], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 17: texColor *= texture(u_Textures[17], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 18: texColor *= texture(u_Textures[18], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 19: texColor *= texture(u_Textures[19], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 20: texColor *= texture(u_Textures[20], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 21: texColor *= texture(u_Textures[21], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 22: texColor *= texture(u_Textures[22], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 23: texColor *= texture(u_Textures[23], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 24: texColor *= texture(u_Textures[24], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 25: texColor *= texture(u_Textures[25], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 26: texColor *= texture(u_Textures[26], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 27: texColor *= texture(u_Textures[27], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 28: texColor *= texture(u_Textures[28], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 29: texColor *= texture(u_Textures[29], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 30: texColor *= texture(u_Textures[30], fragmentIn.TexCoord * fragmentIn.TexScale); break;
		case 31: texColor *= texture(u_Textures[31], fragmentIn.TexCoord * fragmentIn.TexScale); break;
	}

	// Discard the pixel/fragment if it has an alpha of zero
	if (texColor.a == 0.0)
		discard;

	// Set output color
	o_Color = texColor;
	o_EntityID = fragmentIn.EntityID;
	o_BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}