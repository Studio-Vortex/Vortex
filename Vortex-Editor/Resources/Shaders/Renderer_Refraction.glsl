//-------------------------
// - Sparky Game Engine -
// Renderer Refraction Shader
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position; // Vertex position
layout (location = 1) in vec3  a_Normal;   // Vertex normal
layout (location = 2) in vec4  a_Tangent;  // Vertex tangent
layout (location = 3) in vec2  a_TexCoord; // Vertex texture coordinate
layout (location = 4) in vec2  a_TexScale; // Texture scale
layout (location = 5) in int   a_EntityID; // Vertex Entity ID

out vec3       f_Position;
out vec3       f_Normal;
out vec2       f_TexCoord;
out vec2       f_TexScale;
out flat int   f_EntityID;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	f_Position = vec3(u_Model * vec4(a_Position, 1.0));
	f_Normal = a_Normal;
	f_TexCoord = a_TexCoord;
	f_TexScale = a_TexScale;
	
	f_EntityID = a_EntityID;
	
	gl_Position = u_ViewProjection * vec4(f_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

in vec3	      f_Position;
in vec3       f_Normal;
in vec2       f_TexCoord;
in vec2       f_TexScale;
in flat int   f_EntityID;

struct Material
{
	vec3 Ambient;
	sampler2D Diffuse;
	sampler2D Specular;
	float Shininess;
};

uniform samplerCube u_Skybox;
uniform vec3        u_CameraPosition;
uniform float       u_RefractiveIndex;
uniform Material    u_Material;

void main()
{
	float ratio = 1.0 / u_RefractiveIndex;
	vec3 I = normalize(f_Position - u_CameraPosition);
	vec3 R = refract(I, normalize(f_Normal), ratio);

	// Apply Gamma correction
	float gamma = 2.2;
	vec4 finalColor = vec4(pow(texture(u_Skybox, R).rgb * u_Material.Ambient.rgb, vec3(1.0 / gamma)), 1.0);

	o_Color = finalColor;
	o_EntityID = f_EntityID;
}