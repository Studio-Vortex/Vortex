//-------------------------
// - Sparky Game Engine Renderer Basic Lighting Shader -
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position; // Vertex position
layout (location = 1) in vec3  a_Normal;   // Vertex normal
layout (location = 2) in vec4  a_Tangent;  // Vertex tangent
layout (location = 3) in vec2  a_TexCoord; // Vertex texture coordinate
layout (location = 4) in vec2  a_TexScale; // Texture scale
layout (location = 5) in int   a_EntityID; // Vertex Entity ID

out DATA
{
	vec3       Position;
	vec3       Normal;
	vec3       Tangent;
	vec2       TexCoord;
	vec2       TexScale;
	flat int   EntityID;

	mat3       TBN;
} vertexOut;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

void main()
{
	vertexOut.Position = a_Position;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);

	mat3 model = mat3(u_Model);
	vertexOut.Normal = model * a_Normal;
	vertexOut.Tangent = vec3(normalize(mat3(u_Model) * a_Tangent.xyz));
	vertexOut.TexCoord = a_TexCoord;
	vertexOut.TexScale = a_TexScale;

	vertexOut.EntityID = a_EntityID;

	// Calculate the Tangent Bitangent Normal Matrix
	vec3 N = normalize(vertexOut.Normal);
	vec3 T = normalize(vertexOut.Tangent.xyz);
	T = (T - dot(T, N) * N);
	vec3 B = -normalize(cross(vertexOut.Normal, vertexOut.Tangent.xyz) * a_Tangent.w);
	vertexOut.TBN = mat3(T, B, N);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

in DATA
{
	vec3       Position;
	vec3       Normal;
	vec3       Tangent;
	vec2       TexCoord;
	vec2       TexScale;
	flat int   EntityID;

	mat3       TBN;
} fragmentIn;

struct Material
{
	vec3 Ambient;
	sampler2D DiffuseMap;
	sampler2D SpecularMap;
	sampler2D NormalMap;
	float Shininess;

	bool HasDiffuseMap;
	bool HasSpecularMap;
	bool HasNormalMap;
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
	vec3 Diffuse;
	vec3 Specular;
	vec3 Normal;

	mat3 TBN;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
};

struct SceneProperties
{
	int ActiveDirectionalLights;
	int ActivePointLights;
	int ActiveSpotLights;

	vec3 CameraPosition;
	float Exposure;
};

#define MAX_DIRECTIONAL_LIGHTS 25
#define MAX_POINT_LIGHTS 25
#define MAX_SPOT_LIGHTS 25

uniform Material         u_Material;
uniform DirectionalLight u_DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform PointLight       u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight        u_SpotLights[MAX_SPOT_LIGHTS];
uniform SceneProperties  u_SceneProperties;

const float GAMMA = 2.2;

vec3 CalculateDirectionalLight(DirectionalLight lightSource, Material material, FragmentProperties properties);
vec3 CalculatePointLight(PointLight lightSource, Material material, FragmentProperties properties);
vec3 CalculateSpotLight(SpotLight lightSource, Material material, FragmentProperties properties);

float Attenuate(vec3 lightPosition, vec3 fragPosition, float constant, float linear, float quadratic);

void main()
{
	vec2 textureScale = fragmentIn.TexCoord * fragmentIn.TexScale;

	FragmentProperties properties;
	properties.TBN = transpose(fragmentIn.TBN);

	properties.Diffuse = ((u_Material.HasDiffuseMap) ? texture(u_Material.DiffuseMap, textureScale).rgb : vec3(1.0));
	properties.Specular = ((u_Material.HasSpecularMap) ? texture(u_Material.SpecularMap, textureScale).rgb : vec3(1.0));
	properties.Normal = ((u_Material.HasNormalMap) ? normalize(properties.TBN * (texture(u_Material.NormalMap, textureScale).rgb * 2.0 - 1.0)) : fragmentIn.Normal);

	properties.TangentFragPos = properties.TBN * fragmentIn.Position;
	properties.TangentViewPos = properties.TBN * u_SceneProperties.CameraPosition;

	vec3 lightColor = vec3(0.0);

	// Phase 1: Directional lighting
	int activeDirectionalLights = u_SceneProperties.ActiveDirectionalLights;
	if (activeDirectionalLights > 0)
	{
		for (int i = 0; i < activeDirectionalLights; i++)
			lightColor += CalculateDirectionalLight(u_DirectionalLights[i], u_Material, properties);
	}

	// Phase 2: Point lights
	int activePointLights = u_SceneProperties.ActivePointLights;
	if (activePointLights > 0)
	{
		for (int i = 0; i < activePointLights; i++)
			lightColor += CalculatePointLight(u_PointLights[i], u_Material, properties);
	}

	// Phase 3: Spot light
	int activeSpotLights = u_SceneProperties.ActiveSpotLights;
	if (activeSpotLights > 0)
	{
		for (int i = 0; i < activeSpotLights; i++)
			lightColor += CalculateSpotLight(u_SpotLights[i], u_Material, properties);
	}

	// Apply Gamma Correction and Exposure Tone Mapping
	float exposure = u_SceneProperties.Exposure;
	vec3 mapped = vec3(1.0) - exp(-(lightColor) * exposure);
	mapped = pow(mapped, vec3(1.0 / GAMMA));

	// Set the output color
	o_Color = vec4(mapped, 1.0);
	o_EntityID = fragmentIn.EntityID;
}

float Diffuse(vec3 normal, vec3 lightDir)
{
	return max(dot(normal, lightDir), 0.0);
}

float Specular(vec3 normal, vec3 halfwayDir, float shininess)
{
	return pow(max(dot(normal, halfwayDir), 0.0), shininess);
}

float Attenuate(vec3 lightPosition, vec3 fragPosition, float constant, float linear, float quadratic)
{
	float distance = length(lightPosition - fragPosition);
	return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

vec3 CalculateDirectionalLight(DirectionalLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.Diffuse;

	// Diffuse shading
	vec3 normal;
	vec3 lightDir;

	if (material.HasNormalMap)
	{
		normal = properties.Normal; // Tangent space normal
		lightDir = properties.TBN * normalize(-lightSource.Direction);
	}
	else
	{
		normal = normalize(fragmentIn.Normal);
		lightDir = normalize(-lightSource.Direction);
	}

	float diff = Diffuse(normal, lightDir);
	vec3 diffuse = lightSource.Diffuse * diff * properties.Diffuse;
	
	// Specular shading
	vec3 viewDir;

	if (material.HasNormalMap)
		viewDir = properties.TBN * normalize(properties.TangentViewPos - properties.TangentFragPos);
	else
		viewDir = normalize(u_SceneProperties.CameraPosition - fragmentIn.Position);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = Specular(normal, halfwayDir, material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.Specular;

	return lightSource.Color * (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.Diffuse;

	// Diffuse shading
	vec3 normal;
	vec3 lightDir;
	vec3 lightPos;

	if (material.HasNormalMap)
	{
		vec3 tangentLightPos = properties.TBN * lightSource.Position;
		normal = properties.Normal;
		lightDir = properties.TBN * normalize(tangentLightPos - properties.TangentFragPos);
		lightPos = tangentLightPos;
	}
	else
	{
		normal = normalize(fragmentIn.Normal);
		lightDir = normalize(lightSource.Position - fragmentIn.Position);
		lightPos = lightSource.Position;
	}

	float diff = Diffuse(normal, lightDir);
	vec3 diffuse = lightSource.Diffuse * diff * properties.Diffuse;

	// Specular shading
	vec3 viewDir;

	if (material.HasNormalMap)
		viewDir = properties.TBN * normalize(properties.TangentViewPos - properties.TangentFragPos);
	else
		viewDir = normalize(u_SceneProperties.CameraPosition - fragmentIn.Position);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = Specular(normal, halfwayDir, material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.Specular;

	// Attenuation
	vec3 fragmentPos = ((material.HasNormalMap) ? properties.TangentFragPos : fragmentIn.Position);
	float attenuation = Attenuate(lightPos, fragmentPos, lightSource.Constant, lightSource.Linear, lightSource.Quadratic);
	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;

	return lightSource.Color * (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.Diffuse;

	// Diffuse shading
	vec3 normal;
	vec3 lightDir;
	vec3 lightPos;

	if (material.HasNormalMap)
	{
		vec3 tangentLightPos = properties.TBN * lightSource.Position;
		normal = properties.Normal;
		lightDir = properties.TBN * normalize(tangentLightPos - properties.TangentFragPos);
		lightPos = tangentLightPos;
	}
	else
	{
		normal = normalize(fragmentIn.Normal);
		lightDir = normalize(lightSource.Position - fragmentIn.Position);
		lightPos = lightSource.Position;
	}

	float diff = Diffuse(normal, lightDir);
	vec3 diffuse = lightSource.Diffuse * diff * properties.Diffuse;

	// Specular shading
	vec3 viewDir;

	if (material.HasNormalMap)
		viewDir = properties.TBN * normalize(properties.TangentViewPos - properties.TangentFragPos);
	else
		viewDir = normalize(u_SceneProperties.CameraPosition - fragmentIn.Position);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = Specular(normal, halfwayDir, material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.Specular;

	// Spotlight
	float theta = dot(lightDir, normalize(-lightSource.Direction));
	float epsilon = lightSource.CutOff - lightSource.OuterCutOff;
	float intensity = clamp((theta - lightSource.OuterCutOff) / epsilon, 0.0, 1.0);
	diffuse *= intensity;
	specular *= intensity;

	// Attenuation
	vec3 fragmentPos = ((material.HasNormalMap) ? properties.TangentFragPos : fragmentIn.Position);
	float attenuation = Attenuate(lightPos, fragmentPos, lightSource.Constant, lightSource.Linear, lightSource.Quadratic);
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	return lightSource.Color * (ambient + diffuse + specular);
}