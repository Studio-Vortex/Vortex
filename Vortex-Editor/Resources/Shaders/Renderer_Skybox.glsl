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
uniform float u_Gamma;
uniform float u_Exposure;
uniform float u_Multiplier;

void main()
{
	vec3 envColor = textureLod(u_EnvironmentMap, f_TexCoord, 0.0).rgb;

	// Exposure Tonemap and Gamma Correction
	vec3 mapped = vec3(1.0) - exp(-envColor * u_Exposure);
	mapped = pow(mapped, vec3(1.0 / u_Gamma));

	o_Color = vec4(mapped * u_Multiplier, 1.0);
	o_EntityID = -1;
}