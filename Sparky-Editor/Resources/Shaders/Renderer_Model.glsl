//-------------------------
// - Sparky Game Engine -
// Renderer Model Shader with Texturing and Lighting
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position; // Vertex position
layout (location = 1) in vec3  a_Normal;   // Vertex normal
layout (location = 2) in vec4  a_Color;    // Vertex color
layout (location = 3) in vec2  a_TexCoord; // Vertex texture coordinate
layout (location = 4) in vec2  a_TexScale; // Texture scale
layout (location = 5) in int   a_EntityID; // Vertex Entity ID

out vec3       f_Position;
out vec3       f_Normal;
out vec4       f_Color;
out vec2       f_TexCoord;
out vec2       f_TexScale;
out flat int   f_EntityID;

uniform highp mat4 u_Model;
uniform highp mat4 u_View;
uniform highp mat4 u_Projection;

void main()
{
	f_Position = vec3(u_Model * vec4(a_Position, 1.0));
	f_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	f_Color = a_Color;
	f_TexCoord = a_TexCoord;
	f_TexScale = a_TexScale;
	
	f_EntityID = a_EntityID;
	
	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

struct Material
{
	sampler2D Diffuse;
	sampler2D Specular;
	float Shininess;
};

struct PointLight
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	vec3 Color;
	vec3 Position;
};

struct DirectionalLight
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	vec3 Color;
	vec3 Direction;
};

struct SpotLight
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	vec3 Color;
	vec3 Position;
	vec3 Point;
};

in vec3       f_Position;
in vec3       f_Normal;
in vec4       f_Color;
in vec2       f_TexCoord;
in vec2       f_TexScale;
in flat int   f_EntityID;

uniform sampler2D        u_Texture;
uniform vec3             u_CameraPosition;
uniform Material         u_Material;
uniform PointLight       u_PointLight;
uniform DirectionalLight u_DirectionalLight;

void main()
{
	// Ambient
	vec3 ambient = u_PointLight.Ambient * vec3(texture(u_Material.Diffuse, f_TexCoord));

	// Diffuse
	vec3 norm = normalize(f_Normal);
	vec3 lightDir = normalize(u_PointLight.Position - f_Position);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = u_PointLight.Diffuse * diff * vec3(texture(u_Material.Diffuse, f_TexCoord));

	// Specular
	vec3 viewDir = normalize(u_CameraPosition - f_Position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.Shininess);
	vec3 specular = u_PointLight.Specular * spec * vec3(texture(u_Material.Specular, f_TexCoord));

	vec3 lightResult = ambient + diffuse + specular;
	vec4 fragColor = f_Color * texture(u_Texture, f_TexCoord * f_TexScale);

	// Discard the pixel/fragment if it has an alpha of zero
	if (fragColor.a == 0.0)
		discard;

	vec4 finalColor = vec4(lightResult * u_PointLight.Color * fragColor.rbg, fragColor.a);

	o_Color = finalColor;
	o_EntityID = f_EntityID;
}