//-------------------------
// - Sparky Game Engine -
// Renderer2D Text Shader
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position; // Vertex position
layout (location = 1) in vec4  a_Color; // Vertex color
layout (location = 2) in vec2  a_TexCoord; // Vertex texture coordinate
layout (location = 3) in float a_TexIndex; // Texture index out of 32
layout (location = 4) in int   a_EntityID; // Vertex Entity ID

out vec4       f_Color;
out vec2       f_TexCoord;
out flat float f_TexIndex;
out flat int   f_EntityID;

uniform mat4 u_ViewProjection;

void main()
{
	f_Color = a_Color;
	f_TexCoord = a_TexCoord;
	f_TexIndex = a_TexIndex;
	
	f_EntityID = a_EntityID;
	
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

in vec4       f_Color;
in vec2       f_TexCoord;
in flat float f_TexIndex;
in flat int   f_EntityID;

uniform sampler2D u_FontAtlases[32];

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float ScreenPxRange()
{
	float pxRange = 2.0;
    vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_FontAtlases[int(f_TexIndex)], 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(f_TexCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

void main()
{
	vec4 bgColor = vec4(f_Color.rgb, 0.0);
	vec4 fgColor = f_Color;

	// MSDF texture has no mips (only LOD 0), but in the future it might
	// be nice to do some sort of fading/smoothing when camera is far
	vec3 msd = texture(u_FontAtlases[int(f_TexIndex)], f_TexCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = ScreenPxRange() * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    
	// Set output color
	o_Color = mix(bgColor, fgColor, opacity);

	o_EntityID = f_EntityID;
}