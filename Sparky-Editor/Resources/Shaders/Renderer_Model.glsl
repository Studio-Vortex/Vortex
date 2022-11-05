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
	f_Position = a_Position;
	f_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	f_TexCoord = a_TexCoord;
	f_TexScale = a_TexScale;
	
	f_EntityID = a_EntityID;
	
	gl_Position = u_Projection * u_View * vec4(f_Position, 1.0);
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

struct DirectionalLight
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	vec3 Color;
	vec3 Direction;
};

struct PointLight
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	vec3 Color;
	vec3 Position;

	float Constant;
	float Linear;
	float Quadratic;
};

struct SpotLight
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	vec3 Color;
	vec3 Position;
	vec3 Direction;

	float Constant;
	float Linear;
	float Quadratic;

	float CutOff;
	float OuterCutOff;
};

struct FragmentProperties
{
	vec3 DiffuseMap;
	vec3 SpecularMap;
};

#define MAX_POINT_LIGHTS 2

uniform sampler2D        u_Texture;
uniform vec3             u_CameraPosition;
uniform Material         u_Material;
uniform DirectionalLight u_DirectionalLight;
uniform PointLight       u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight        u_SpotLight;

vec3 CalculateDirectionalLight(DirectionalLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.DiffuseMap;

	// Diffuse shading
	vec3 normal = normalize(f_Normal);
	vec3 lightDir = normalize(-lightSource.Direction);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = lightSource.Diffuse * diff * properties.DiffuseMap;

	// Specular shading
	vec3 viewDir = normalize(u_CameraPosition - f_Position);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.SpecularMap;

	return lightSource.Color * (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.DiffuseMap;

	// Diffuse shading
	vec3 normal = normalize(f_Normal);
	vec3 lightDir = normalize(lightSource.Position - f_Position);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = lightSource.Diffuse * diff * properties.DiffuseMap;

	// Specular shading
	vec3 viewDir = normalize(u_CameraPosition - f_Position);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.SpecularMap;

	// Attenuation shading
	float distance = length(lightSource.Position - f_Position);
	float attenuation = 1.0 / (lightSource.Constant + lightSource.Linear * distance + lightSource.Quadratic * (distance * distance));
	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;

	return lightSource.Color * (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.DiffuseMap;

	// Diffuse shading
	vec3 normal = normalize(f_Normal);
	vec3 lightDir = normalize(lightSource.Position - f_Position);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = lightSource.Diffuse * diff * properties.DiffuseMap;

	// Specular shading
	vec3 viewDir = normalize(u_CameraPosition - f_Position);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.SpecularMap;

	// Spotlight
	float theta = dot(lightDir, normalize(-lightSource.Direction));
	float epsilon = lightSource.CutOff - lightSource.OuterCutOff;
	float intensity = clamp((theta - lightSource.OuterCutOff) / epsilon, 0.0, 1.0);
	diffuse *= intensity;
	specular *= intensity;

	// Attenuation shading
	float distance = length(lightSource.Position - f_Position);
	float attenuation = 1.0 / (lightSource.Constant + lightSource.Linear * distance + lightSource.Quadratic * (distance * distance));
	//ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;

	return lightSource.Color * (ambient + diffuse + specular);
}

void main()
{
	vec3 lightColor;
	vec3 diffuseMap = texture(u_Material.Diffuse, f_TexCoord * f_TexScale).rgb;
	vec3 specularMap = texture(u_Material.Specular, f_TexCoord * f_TexScale).rgb;

	FragmentProperties properties;
	properties.DiffuseMap = diffuseMap;
	properties.SpecularMap = specularMap;

	// Phase 1: Directional lighting
	lightColor += CalculateDirectionalLight(u_DirectionalLight, u_Material, properties);

	// Phase 2: Point lights
	for (int i = 0; i  < MAX_POINT_LIGHTS; i++)
		lightColor += CalculatePointLight(u_PointLights[i], u_Material, properties);

	// Phase 3: Spot light
	lightColor += CalculateSpotLight(u_SpotLight, u_Material, properties);

	vec4 fragColor = texture(u_Texture, f_TexCoord * f_TexScale);

	// Discard the pixel/fragment if it has an alpha of zero
	if (fragColor.a == 0.0)
		discard;

	// Apply Gamma correction
	float gamma = 2.2;
	vec4 finalColor = vec4(pow(lightColor * fragColor.rgb, vec3(1.0 / gamma)), fragColor.a);

	// Set the output color
	o_Color = finalColor;
	o_EntityID = f_EntityID;
}