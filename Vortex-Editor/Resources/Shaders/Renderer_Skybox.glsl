//-------------------------
// - Sparky Game Engine Renderer Skybox Shader -
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3 a_Position;

out vec3 f_TexCoord;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
	f_TexCoord = a_Position;

	vec4 position = u_Projection * u_View * vec4(a_Position, 1.0);
	gl_Position = position.xyww;
}

#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

in vec3 f_TexCoord;

uniform samplerCube u_EnvironmentMap;

const float GAMMA = 2.2;

void main()
{
	vec3 envColor = textureLod(u_EnvironmentMap, f_TexCoord, 0.0).rgb;

	// HDR Tonemap and Gamma Correction
	envColor = envColor / (envColor + vec3(1.0));
	envColor = pow(envColor, vec3(1.0 / GAMMA));

	o_Color = vec4(envColor, 1.0);
	o_EntityID = -1;
}