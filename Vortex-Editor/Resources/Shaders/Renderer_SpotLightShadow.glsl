//-------------------------
// - Vortex Game Engine Renderer Spot Light Shadow Map Shader -
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in ivec4 a_BoneIDs;
layout (location = 2) in vec4 a_BoneWeights;

uniform mat4 u_Model;

#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4
uniform mat4 u_FinalBoneMatrices[MAX_BONES];
uniform bool u_HasAnimations;

void main()
{
    if (!u_HasAnimations)
    {
        gl_Position = u_Model * vec4(a_Position, 1.0);
        return;
    }

    vec4 totalPosition = vec4(0.0);

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (a_BoneIDs[i] == -1)
            continue;

        if (a_BoneIDs[i] >= MAX_BONES)
        {
            totalPosition = vec4(a_Position, 1.0);
            break;
        }

        vec4 localPosition = u_FinalBoneMatrices[a_BoneIDs[i]] * vec4(a_Position, 1.0);
        totalPosition += localPosition * a_BoneWeights[i];
    }

    gl_Position = u_Model * totalPosition;
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;

void main()
{

}