//-------------------------
// - Vortex Game Engine Bloom Final Composite Shader -
//------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;

layout (location = 0) out vec2 f_TexCoord;

void main()
{
    f_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;

layout (location = 0) in vec2 f_TexCoord;

uniform sampler2D u_SceneTexture;
uniform sampler2D u_BloomTexture;
uniform bool u_Bloom;
uniform float u_Exposure;
uniform float u_Gamma;

void main()
{
    vec3 hdrColor = texture(u_SceneTexture, f_TexCoord).rgb;
    vec3 bloomColor = texture(u_BloomTexture, f_TexCoord).rgb;
    
    if (u_Bloom)
        hdrColor += bloomColor; // additive blending
    
    // Tone Mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * u_Exposure);
    // Gamma Correction
    result = pow(result, vec3(1.0 / u_Gamma));
    
    o_Color = vec4(result, 1.0);
}