//-------------------------
// - Sparky Game Engine -
// Renderer Model Shader with Texturing and Lighting
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position; // Vertex position
layout (location = 1) in vec3  a_Normal;   // Vertex normal
layout (location = 2) in vec2  a_TexCoord; // Vertex texture coordinate
layout (location = 3) in vec2  a_TexScale; // Texture scale
layout (location = 4) in int   a_EntityID; // Vertex Entity ID

out vec3       f_Position;
out vec3       f_Normal;
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
	f_TexCoord = a_TexCoord;
	f_TexScale = a_TexScale;
	
	f_EntityID = a_EntityID;
	
	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

in vec3       f_Position;
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

uniform sampler2D        u_Texture;
uniform vec3             u_CameraPosition;
uniform Material         u_Material;
uniform DirectionalLight u_DirectionalLight;
uniform PointLight       u_PointLight;
uniform SpotLight        u_SpotLight;
uniform int              u_LightType;

vec4 CalculateDirectionalLight(DirectionalLight lightSource, Material material)
{
	// Ambient
	vec3 ambient = lightSource.Ambient * material.Ambient * vec3(texture(material.Diffuse, f_TexCoord * f_TexScale));

	// Diffuse
	vec3 norm = normalize(f_Normal);
	vec3 lightDir = normalize(-lightSource.Direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = lightSource.Diffuse * diff * vec3(texture(material.Diffuse, f_TexCoord * f_TexScale).rgb);

	// Specular
	vec3 viewDir = normalize(u_CameraPosition - f_Position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);
	vec3 specular = lightSource.Specular * spec * vec3(texture(material.Specular, f_TexCoord * f_TexScale).rgb);

	vec3 lightResult = ambient + diffuse + specular;
	vec4 fragColor = texture(u_Texture, f_TexCoord * f_TexScale);

	// Discard the pixel/fragment if it has an alpha of zero
	if (fragColor.a == 0.0)
		discard;

	return vec4(lightSource.Color * lightResult * fragColor.rbg, fragColor.a);
}

vec4 CalculatePointLight(PointLight lightSource, Material material)
{
	// Ambient
	vec3 ambient = lightSource.Ambient * material.Ambient * vec3(texture(material.Diffuse, f_TexCoord * f_TexScale));

	// Diffuse
	vec3 norm = normalize(f_Normal);
	vec3 lightDir = normalize(lightSource.Position - f_Position);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = lightSource.Diffuse * diff * vec3(texture(material.Diffuse, f_TexCoord * f_TexScale).rgb);

	// Specular
	vec3 viewDir = normalize(u_CameraPosition - f_Position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);
	vec3 specular = lightSource.Specular * spec * vec3(texture(material.Specular, f_TexCoord * f_TexScale).rgb);

	vec3 lightResult = ambient + diffuse + specular;
	vec4 fragColor = texture(u_Texture, f_TexCoord * f_TexScale);

	// Discard the pixel/fragment if it has an alpha of zero
	if (fragColor.a == 0.0)
		discard;

	return vec4(lightSource.Color * lightResult * fragColor.rbg, fragColor.a);
}

vec4 CalculateSpotLight(SpotLight lightSource, Material material)
{
	return vec4(1.0);
}

void main()
{
	// Diffuse
	vec3 norm = normalize(f_Normal);

	vec3 lightDir; // Calculate the light direction based on the light type
	if (u_LightType == 0) // Directional
		o_Color = CalculateDirectionalLight(u_DirectionalLight, u_Material);
	else if (u_LightType == 1) // Point
		o_Color = CalculatePointLight(u_PointLight, u_Material);
	else if (u_LightType == 2) // spot
		o_Color = CalculateSpotLight(u_SpotLight, u_Material);

	o_EntityID = f_EntityID;
}