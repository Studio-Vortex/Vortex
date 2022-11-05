//-------------------------
// - Sparky Game Engine -
// Renderer Skybox Shader with Texturing
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

uniform samplerCube u_Skybox;

void main()
{
	vec4 skybox = texture(u_Skybox, f_TexCoord);

	// Apply Gamma correction
	float gamma = 2.2;
	vec4 finalColor = vec4(pow(skybox.rgb, vec3(1.0 / gamma)), skybox.a);

	o_Color = finalColor;
	o_EntityID = -1;
}