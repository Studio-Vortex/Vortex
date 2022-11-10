//-------------------------
// - Sparky Game Engine -
// Renderer Model Shader with Texturing and Lighting
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position; // Vertex position
layout (location = 1) in vec3  a_Normal;   // Vertex normal
layout (location = 2) in vec4  a_Tangent;  // Vertex tangent
layout (location = 3) in vec2  a_TexCoord; // Vertex texture coordinate
layout (location = 4) in vec2  a_TexScale; // Texture scale
layout (location = 5) in int   a_EntityID; // Vertex Entity ID

out VertexOut {
	vec3       Position;
	vec3       Normal;
	vec4       Tangent;
	vec2       TexCoord;
	vec2       TexScale;
	flat int   EntityID;

	mat3       TBN;
} vertexOut;

uniform highp mat4 u_Model;
uniform highp mat4 u_View;
uniform highp mat4 u_Projection;

void main()
{
	vertexOut.Position = a_Position;
	vertexOut.Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	vertexOut.Tangent = vec4(normalize(mat3(u_Model) * a_Tangent.xyz), a_Tangent.w);
	vertexOut.TexCoord = a_TexCoord;
	vertexOut.TexScale = a_TexScale;

	vertexOut.EntityID = a_EntityID;

	// Calculate the Tangent Bitangent Normal Matrix
	vec3 N = normalize(vertexOut.Normal);
	vec3 T = normalize(vertexOut.Tangent.xyz);
	T = (T - dot(T, N) * N);
	vec3 B = cross(normalize(vertexOut.Normal), vertexOut.Tangent.xyz) * vertexOut.Tangent.w;
	vertexOut.TBN = mat3(T, B, N);
	
	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

in VertexOut
{
	vec3       Position;
	vec3       Normal;
	vec4       Tangent;
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

	vec3 Albedo;
	sampler2D AlbedoMap;
	float Metallic;
	sampler2D MetallicMap;
	float Roughness;
	sampler2D RoughnessMap;
	sampler2D AOMap;
	
	bool HasDiffuseMap;
	bool HasSpecularMap;
	bool HasNormalMap;

	bool HasAlbedoMap;
	bool HasMetallicMap;
	bool HasRoughnessMap;
	bool HasAOMap;
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

	vec3 Albedo;
	float Metallic;
	float Roughness;
	float AO;

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

const float PI = 3.14159265359;
const float GAMMA = 2.2;

uniform sampler2D        u_Texture;
uniform Material         u_Material;
uniform DirectionalLight u_DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform PointLight       u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight        u_SpotLights[MAX_SPOT_LIGHTS];
uniform SceneProperties  u_SceneProperties;

vec3 CalculateDirectionalLight(DirectionalLight lightSource, Material material, FragmentProperties properties);
vec3 CalculatePointLight(PointLight lightSource, Material material, FragmentProperties properties);
vec3 CalculateSpotLight(SpotLight lightSource, Material material, FragmentProperties properties);

float CalculateAttenuation(vec3 lightPosition, vec3 fragPosition, float constant, float linear, float quadratic);

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

void main()
{
	vec2 textureScale = fragmentIn.TexCoord * fragmentIn.TexScale;

	vec4 finalColor = vec4(1.0);
	bool pbr = u_Material.HasAlbedoMap && u_Material.HasMetallicMap && u_Material.HasRoughnessMap && u_Material.HasAOMap;

	FragmentProperties properties;
	properties.TBN = (pbr) ? fragmentIn.TBN : transpose(fragmentIn.TBN);

	properties.Diffuse = ((u_Material.HasDiffuseMap) ? texture(u_Material.DiffuseMap, textureScale).rgb : vec3(1.0));
	properties.Specular = ((u_Material.HasSpecularMap) ? texture(u_Material.SpecularMap, textureScale).rgb : vec3(1.0));
	properties.Normal = ((u_Material.HasNormalMap) ? normalize(properties.TBN * (texture(u_Material.NormalMap, textureScale).rgb * 2.0 - 1.0)) : fragmentIn.Normal);

	properties.Albedo = ((u_Material.HasAlbedoMap) ? pow(texture(u_Material.AlbedoMap, textureScale).rgb, vec3(GAMMA)) : u_Material.Albedo);
	properties.Metallic = ((u_Material.HasMetallicMap) ? texture(u_Material.MetallicMap, textureScale).r : u_Material.Metallic);
	properties.Roughness = ((u_Material.HasRoughnessMap) ? texture(u_Material.RoughnessMap, textureScale).r : u_Material.Roughness);
	properties.AO = ((u_Material.HasAOMap) ? texture(u_Material.AOMap, textureScale).r : 1.0);

	properties.TangentFragPos = properties.TBN * fragmentIn.Position;
	properties.TangentViewPos = properties.TBN * u_SceneProperties.CameraPosition;

	if (!pbr)
	{
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
		finalColor = vec4(mapped, 1.0);
	}
	else
	{
		vec3 N = properties.Normal;
		vec3 V = normalize(u_SceneProperties.CameraPosition - fragmentIn.Position);

		// Calculate reflectance at normal incidence, i.e. how much the surface reflects when looking directly at it
		// if dia-electric (like plastic) use F0 of 0.04
		// if it's a metal, use the albedo color as F0 (metallic workflow)
		vec3 F0 = vec3(0.04);
		F0 = mix(F0, properties.Albedo, properties.Metallic);

		// Reflectance equation
		vec3 Lo = vec3(0.0);

		// Calculate per-light radiance
		for(int i = 0; i < u_SceneProperties.ActivePointLights; i++)
		{
			vec3 L = normalize(u_PointLights[i].Position - fragmentIn.Position);
			vec3 H = normalize(V + L);
			float attenuation = CalculateAttenuation(u_PointLights[i].Position, fragmentIn.Position, u_PointLights[i].Constant, u_PointLights[i].Linear, u_PointLights[i].Quadratic);
			vec3 radiance = u_PointLights[i].Color * attenuation;

			// Cook-Torrance BRDF
			float NDF = DistributionGGX(N, H, properties.Roughness);
			float G   = GeometrySmith(N, V, L, properties.Roughness);
			vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);

			vec3 numerator = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent division by 0
			vec3 specular = numerator / denominator;

			// kS is equal to Fresnel
			vec3 kS = F;
			// For energy conservation, the diffuse and specular light can't
			// be above 1.0 (unless the surface emits light); to preserve this
			// relationship the diffuse component (kD) should equal 1.0 - kS.
			vec3 kD = vec3(1.0) - kS;
			// Multiply kD by the inverse metalness such that only non-metals 
			// have diffuse lighting, or a linear blend if partly metal (pure metals
			// have no diffuse light).
			kD *= 1.0 - properties.Metallic;

			// Scale light by NdotL
			float NdotL = max(dot(N, L), 0.0);

			// Add to outgoing radiance Lo
			Lo += (kD * properties.Albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
		}

		vec3 ambient = vec3(0.03) * properties.Albedo * properties.AO;

		vec3 color = ambient + Lo;

		// HDR tonemapping
		color = color / (color + vec3(1.0));

		// Gamma correct
		color = pow(color, vec3(1.0 / GAMMA));

		finalColor = vec4(color, 1.0);
	}

	o_Color = finalColor;
	o_EntityID = fragmentIn.EntityID;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float CalculateDiffuse(vec3 normal, vec3 lightDir)
{
	return max(dot(normal, lightDir), 0.0);
}

float CalculateSpecular(vec3 normal, vec3 halfwayDir, float shininess)
{
	return pow(max(dot(normal, halfwayDir), 0.0), shininess);
}

float CalculateAttenuation(vec3 lightPosition, vec3 fragPosition, float constant, float linear, float quadratic)
{
	float distance = length(lightPosition - fragPosition);
	return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

vec3 CalculateDirectionalLight(DirectionalLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.Diffuse * properties.AO;

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

	float diff = CalculateDiffuse(normal, lightDir);
	vec3 diffuse = lightSource.Diffuse * diff * properties.Diffuse;
	
	// Specular shading
	vec3 viewDir;

	if (material.HasNormalMap)
		viewDir = properties.TBN * normalize(properties.TangentViewPos - properties.TangentFragPos);
	else
		viewDir = normalize(u_SceneProperties.CameraPosition - fragmentIn.Position);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = CalculateSpecular(normal, halfwayDir, material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.Specular;

	return lightSource.Color * (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.Diffuse * properties.AO;

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

	float diff = CalculateDiffuse(normal, lightDir);
	vec3 diffuse = lightSource.Diffuse * diff * properties.Diffuse;

	// Specular shading
	vec3 viewDir;

	if (material.HasNormalMap)
		viewDir = properties.TBN * normalize(properties.TangentViewPos - properties.TangentFragPos);
	else
		viewDir = normalize(u_SceneProperties.CameraPosition - fragmentIn.Position);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = CalculateSpecular(normal, halfwayDir, material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.Specular;

	// Attenuation
	vec3 fragmentPos = ((material.HasNormalMap) ? properties.TangentFragPos : fragmentIn.Position);
	float attenuation = CalculateAttenuation(lightPos, fragmentPos, lightSource.Constant, lightSource.Linear, lightSource.Quadratic);
	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;

	return lightSource.Color * (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.Diffuse * properties.AO;

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

	float diff = CalculateDiffuse(normal, lightDir);
	vec3 diffuse = lightSource.Diffuse * diff * properties.Diffuse;

	// Specular shading
	vec3 viewDir;

	if (material.HasNormalMap)
		viewDir = properties.TBN * normalize(properties.TangentViewPos - properties.TangentFragPos);
	else
		viewDir = normalize(u_SceneProperties.CameraPosition - fragmentIn.Position);

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = CalculateSpecular(normal, halfwayDir, material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.Specular;

	// Spotlight
	float theta = dot(lightDir, normalize(-lightSource.Direction));
	float epsilon = lightSource.CutOff - lightSource.OuterCutOff;
	float intensity = clamp((theta - lightSource.OuterCutOff) / epsilon, 0.0, 1.0);
	diffuse *= intensity;
	specular *= intensity;

	// Attenuation
	vec3 fragmentPos = ((material.HasNormalMap) ? properties.TangentFragPos : fragmentIn.Position);
	float attenuation = CalculateAttenuation(lightPos, fragmentPos, lightSource.Constant, lightSource.Linear, lightSource.Quadratic);
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	return lightSource.Color * (ambient + diffuse + specular);
}