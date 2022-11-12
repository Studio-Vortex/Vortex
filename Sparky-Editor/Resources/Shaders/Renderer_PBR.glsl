//-------------------------
// - Sparky Game Engine Renderer PBR Shader -
//-------------------------

// Still a work in progress, but will be seing many updates in the future

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
	vec3       WorldPosition;
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
	vertexOut.WorldPosition = vec3(u_Model * vec4(a_Position, 1.0));
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
	vec3       WorldPosition;
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
	vec3 Albedo;
	sampler2D AlbedoMap;
	sampler2D NormalMap;
	float Metallic;
	sampler2D MetallicMap;
	float Roughness;
	sampler2D RoughnessMap;
	sampler2D AOMap;
	
	bool HasAlbedoMap;
	bool HasNormalMap;
	bool HasMetallicMap;
	bool HasRoughnessMap;
	bool HasAOMap;
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

struct FragmentProperties
{
	vec3 Albedo;
	vec3 Normal;
	float Metallic;
	float Roughness;
	float AO;

	mat3 TBN;
};

struct SceneProperties
{
	int ActiveDirectionalLights;
	int ActivePointLights;
	int ActiveSpotLights;

	vec3 CameraPosition;
	float Exposure;
};

#define MAX_POINT_LIGHTS 25

uniform sampler2D        u_Texture;
uniform Material         u_Material;
uniform PointLight       u_PointLights[MAX_POINT_LIGHTS];
uniform SceneProperties  u_SceneProperties;

const float PI = 3.14159265359;
const float GAMMA = 2.2;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0, float rougness);

float Attenuate(vec3 lightPosition, vec3 fragPosition, float constant, float linear, float quadratic)
{
	float distance = length(lightPosition - fragPosition);
	return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

void main()
{
	vec2 textureScale = fragmentIn.TexCoord * fragmentIn.TexScale;

	FragmentProperties properties;
	properties.TBN = fragmentIn.TBN;

	properties.Albedo = ((u_Material.HasAlbedoMap) ? pow(texture(u_Material.AlbedoMap, textureScale).rgb, vec3(GAMMA)) : u_Material.Albedo);
	properties.Normal = ((u_Material.HasNormalMap) ? normalize(properties.TBN * (texture(u_Material.NormalMap, textureScale).rgb * 2.0 - 1.0)) : fragmentIn.Normal);
	properties.Metallic = ((u_Material.HasMetallicMap) ? texture(u_Material.MetallicMap, textureScale).r : u_Material.Metallic);
	properties.Roughness = ((u_Material.HasRoughnessMap) ? texture(u_Material.RoughnessMap, textureScale).r : u_Material.Roughness);
	properties.AO = ((u_Material.HasAOMap) ? texture(u_Material.AOMap, textureScale).r : 1.0);

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
		float attenuation = Attenuate(u_PointLights[i].Position, fragmentIn.Position, u_PointLights[i].Constant, u_PointLights[i].Linear, u_PointLights[i].Quadratic);
		vec3 radiance = u_PointLights[i].Color * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, properties.Roughness);
		float G   = GeometrySmith(N, V, L, properties.Roughness);
		vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0, properties.Roughness);

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

	o_Color = vec4(color, 1.0);
	o_EntityID = fragmentIn.EntityID;
}

// Towbridge-Reitz normal distribuion function
// Uses Disney's reparameterization of aplha = roughness^2
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = alphaSq;
    float denom = (NdotH2 * (alphaSq - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}