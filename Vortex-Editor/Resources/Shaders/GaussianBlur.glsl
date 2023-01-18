//-------------------------
// - Vortex Game Engine Gaussian Blur Shader -
//-------------------------

#type vertex
#version 460 core

void main()
{

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