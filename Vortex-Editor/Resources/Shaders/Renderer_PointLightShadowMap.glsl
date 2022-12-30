//-------------------------
// - Vortex Game Engine Renderer Point Light Shadow Map Shader -
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
    if (u_HasAnimations)
    {
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
    else
    {
        gl_Position = u_Model * vec4(a_Position, 1.0);
    }
}


#type geometry
#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 u_ShadowTransforms[6];

layout (location = 0) out vec4 o_FragPos;

void main()
{
	for (int face = 0; face < 6; face++)
	{
		gl_Layer = face; // built-in variable that specifies which face to render

		for (int i = 0; i < 3; i++) // for each triangle strip
		{
			o_FragPos = gl_in[i].gl_Position;
			gl_Position = u_ShadowTransforms[face] * o_FragPos;
			EmitVertex();
		}

		EndPrimitive();
	}
}


#type fragment
#version 460 core

layout (location = 0) in vec4 o_FragPos;

uniform vec3 u_LightPositon;
uniform float u_FarPlane;

void main()
{
	// distance between fragment and light source
	float lightDistance = length(o_FragPos.xyz - u_LightPositon);

	// map to 0 -> 1 range
	lightDistance = lightDistance / u_FarPlane;

	// write to depth value
	gl_FragDepth = lightDistance;
}