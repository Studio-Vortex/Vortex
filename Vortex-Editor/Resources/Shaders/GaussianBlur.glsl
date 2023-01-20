//-------------------------
// - Vortex Game Engine Gaussian Blur Shader -
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position; // Vertex position
layout (location = 1) in vec2  a_TexCoord; // Vertex texture coordinate

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

uniform sampler2D u_Texture;

uniform bool u_Horizontal;
uniform float u_Weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 texOffset = 1.0 / textureSize(u_Texture, 0);
    vec3 result = texture(u_Texture, f_TexCoord).rgb * u_Weights[0];

    if (u_Horizontal)
    {
        for (int i = 1; i < 5; i++)
        {
            result += texture(u_Texture, f_TexCoord + vec2(texOffset.x * i, 0.0)).rgb * u_Weights[i];
            result += texture(u_Texture, f_TexCoord - vec2(texOffset.x * i, 0.0)).rgb * u_Weights[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; i++)
        {
            result += texture(u_Texture, f_TexCoord + vec2(0.0, texOffset.y * i)).rgb * u_Weights[i];
            result += texture(u_Texture, f_TexCoord - vec2(0.0, texOffset.y * i)).rgb * u_Weights[i];
        }
    }

    o_Color = vec4(result, 1.0);
}