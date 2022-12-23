//-------------------------
// - Vortex Game Engine Physically Based Rendering Shader -
// - Includes
//     Albedo Mapping,
//     Normal Mapping,
//     Metallic Mapping,
//     Roughness Mapping,
//     Parallax Occlusion Mapping,
//     Emission,
//     Ambient Occlusion Mapping,
//     Image Based Lighting,
//     HDR & Exposure ToneMapping,
//     Directional Light Shadow Mapping and PCF
//     Gamma Correction
//-------------------------

// Still a work in progress, but will be seing many updates in the future

// NOTE: The attenation function used to calculate light intensity over distance,
// is NOT considered to be realistic in a PBR renderer.
// To be more physically correct, attenuation should be calculated based on the actual distance
// from the fragment to the light source. However this allows us to have extreme control over the lights in our scene

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position; // Vertex position
layout (location = 1) in vec3  a_Normal;   // Vertex normal
layout (location = 2) in vec3  a_Tangent;  // Vertex tangent
layout (location = 3) in vec3  a_BiTangent;  // Vertex bitangent
layout (location = 4) in vec2  a_TexCoord; // Vertex texture coordinate
layout (location = 5) in vec2  a_TexScale; // Texture scale
layout (location = 6) in int   a_EntityID; // Vertex Entity ID

out DATA
{
	vec3       Position;
	vec3       Normal;
	vec2       TexCoord;
	vec2       TexScale;
	flat int   EntityID;

	vec4       FragPosLight;

	mat3       TBN;
} vertexOut;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
uniform mat4 u_LightProjection;
uniform mat4 u_SpotLightProjection;

void main()
{
	vertexOut.Position = vec3(u_Model * vec4(a_Position, 1.0));
	gl_Position = u_ViewProjection * vec4(vertexOut.Position, 1.0);

	mat3 model = mat3(u_Model);
	vertexOut.Normal = normalize(model * a_Normal);
	vertexOut.TexCoord = a_TexCoord;
	vertexOut.TexScale = a_TexScale;
	vertexOut.EntityID = a_EntityID;

	vertexOut.FragPosLight = u_LightProjection * vec4(vertexOut.Position, 1.0);

	// Calculate TBN matrix
	vec3 T = normalize(model * a_Tangent);
	vec3 N = vertexOut.Normal;
	vec3 B = normalize(model * a_BiTangent);
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
	vec2       TexCoord;
	vec2       TexScale;
	flat int   EntityID;

	vec4       FragPosLight;

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
	vec3 Emission;
	sampler2D EmissionMap;
	float ParallaxHeightScale;
	sampler2D POMap;
	sampler2D AOMap;

	bool HasAlbedoMap;
	bool HasNormalMap;
	bool HasMetallicMap;
	bool HasRoughnessMap;
	bool HasEmissionMap;
	bool HasPOMap;
	bool HasAOMap;
};

struct DirectionalLight
{
	vec3 Radiance;
	vec3 Direction;
};

struct PointLight
{
	vec3 Radiance;
	vec3 Position;

	float Constant;
	float Linear;
	float Quadratic;
};

struct SpotLight
{
	vec3 Radiance;
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
	vec3 Albedo;
	vec3 Normal;
	float Metallic;
	float Roughness;
	vec3 Emission;
	float AO;
};

struct SceneProperties
{
	int ActiveDirectionalLights;
	int ActivePointLights;
	int ActiveSpotLights;

	samplerCube IrradianceMap;
	samplerCube PrefilterMap;
	sampler2D BRDFLut;

	vec3 CameraPosition;
	float Exposure;
	float Gamma;
};

#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_POINT_LIGHTS 100
#define MAX_SPOT_LIGHTS 50

struct SkylightShadowSettings
{
	sampler2D ShadowMap;
	float ShadowBias;
};

uniform Material         u_Material;
uniform DirectionalLight u_DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform PointLight       u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight        u_SpotLights[MAX_SPOT_LIGHTS];
uniform SceneProperties  u_SceneProperties;
uniform SkylightShadowSettings u_SkylightShadowSettings;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);
const float PI = 3.14159265359;
const float EPSILON = 0.0001;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float gaSchlickG1(float cosTheta, float k);
float gaSchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float rougness);
float Attenuate(vec3 lightPosition, vec3 fragPosition, float constant, float linear, float quadratic);
vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace, float NdotL, sampler2D shadowMap, float shadowBias);

void main()
{
	vec3 viewDir = normalize(fragmentIn.TBN * u_SceneProperties.CameraPosition - fragmentIn.TBN * fragmentIn.Position);
	vec2 textureCoords = ((u_Material.HasPOMap) ? ParallaxOcclusionMapping(fragmentIn.TexCoord * fragmentIn.TexScale, viewDir) : fragmentIn.TexCoord * fragmentIn.TexScale);

	FragmentProperties properties;
	properties.Albedo = ((u_Material.HasAlbedoMap) ? pow(texture(u_Material.AlbedoMap, textureCoords).rgb, vec3(u_SceneProperties.Gamma)) : u_Material.Albedo);
	properties.Normal = ((u_Material.HasNormalMap) ? normalize(fragmentIn.TBN * (texture(u_Material.NormalMap, textureCoords).rgb * 2.0 - 1.0)) : normalize(fragmentIn.Normal));
	properties.Metallic = ((u_Material.HasMetallicMap) ? texture(u_Material.MetallicMap, textureCoords).r : u_Material.Metallic);
	properties.Roughness = ((u_Material.HasRoughnessMap) ? texture(u_Material.RoughnessMap, textureCoords).r : u_Material.Roughness);
	properties.Emission = ((u_Material.HasEmissionMap) ? texture(u_Material.EmissionMap, textureCoords).rgb : u_Material.Emission);
	properties.AO = ((u_Material.HasAOMap) ? texture(u_Material.AOMap, textureCoords).r : 1.0);

	vec3 N = properties.Normal;
	vec3 V = normalize(u_SceneProperties.CameraPosition - fragmentIn.Position);
	vec3 R = reflect(-V, N);

	// Calculate reflectance at normal incidence, i.e. how much the surface reflects when looking directly at it
	// if dia-electric (like plastic) use F0 of 0.04
	// if it's a metal, use the albedo color as F0 (metallic workflow)
	vec3 F0 = Fdielectric;
	F0 = mix(F0, properties.Albedo, properties.Metallic);

	// Reflectance equation
	vec3 Lo = vec3(0.0);

	// Calculate per-light radiance
	for (int i = 0; i < u_SceneProperties.ActiveDirectionalLights; i++)
	{
		DirectionalLight dirLight = u_DirectionalLights[i];

		vec3 L = normalize(-dirLight.Direction);
		vec3 H = normalize(V + L);

		vec3 radiance = dirLight.Radiance;

		// Cook-Torrance Bi-directional Reflectance Distribution Function
		float NDF = DistributionGGX(N, H, properties.Roughness);
		float G = GeometrySmith(N, V, L, properties.Roughness);
		float cosTheta = max(dot(H, V), 0.0);
		vec3 F = FresnelSchlick(cosTheta, F0);

		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * NdotV * NdotL + EPSILON; // prevent division by 0
		vec3 specular = numerator / denominator;

		// kS is equal to Fresnel
		vec3 kS = F;
		// For energy conservation, the diffuse and specular light can't
		// be above 1.0 (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.0 - kS
		vec3 kD = vec3(1.0) - kS;
		// Multiply kD by the inverse metalness such that only non-metals 
		// have diffuse lighting, or a linear blend if partly metal (pure metals have no diffuse light)
		kD *= 1.0 - properties.Metallic;

		float shadow = ShadowCalculation(fragmentIn.FragPosLight, dot(N, L), u_SkylightShadowSettings.ShadowMap, u_SkylightShadowSettings.ShadowBias);

		// Add to outgoing radiance Lo
		Lo += (1.0 - shadow) * (kD * properties.Albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	}

	for(int i = 0; i < u_SceneProperties.ActivePointLights; i++)
	{
		PointLight pointLight = u_PointLights[i];

		vec3 L = normalize(pointLight.Position - fragmentIn.Position);
		vec3 H = normalize(V + L);
		float attenuation = Attenuate(pointLight.Position, fragmentIn.Position,
									  pointLight.Constant, pointLight.Linear, pointLight.Quadratic);

		vec3 radiance = pointLight.Radiance * attenuation;

		// Cook-Torrance Bi-directional Reflectance Distribution Function
		float NDF = DistributionGGX(N, H, properties.Roughness);
		float G = GeometrySmith(N, V, L, properties.Roughness);
		float cosTheta = max(dot(H, V), 0.0);
		vec3 F = FresnelSchlick(cosTheta, F0);

		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * NdotV * NdotL + EPSILON; // prevent division by 0
		vec3 specular = numerator / denominator;

		// kS is equal to Fresnel
		vec3 kS = F;
		// For energy conservation, the diffuse and specular light can't
		// be above 1.0 (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.0 - kS
		vec3 kD = vec3(1.0) - kS;
		// Multiply kD by the inverse metalness such that only non-metals 
		// have diffuse lighting, or a linear blend if partly metal (pure metals have no diffuse light)
		kD *= 1.0 - properties.Metallic;

		// Add to outgoing radiance Lo
		Lo += (kD * properties.Albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	}

	for (int i = 0; i < u_SceneProperties.ActiveSpotLights; i++)
	{
		SpotLight spotLight = u_SpotLights[i];

		vec3 L = normalize(spotLight.Position - fragmentIn.Position);
		vec3 H = normalize(V + L);
		float attenuation = Attenuate(spotLight.Position, fragmentIn.Position,
									  spotLight.Constant, spotLight.Linear, spotLight.Quadratic);

		vec3 radiance = spotLight.Radiance * attenuation;

		// Cook-Torrance Bi-Directional Reflectance Distribution Function
		float NDF = DistributionGGX(N, H, properties.Roughness);
		float G = GeometrySmith(N, V, L, properties.Roughness);
		float cosTheta = max(dot(H, V), 0.0f);
		vec3 F = FresnelSchlick(cosTheta, F0);

		float NdotV = max(dot(N, V), 0.0f);
		float NdotL = max(dot(N, L), 0.0f);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * NdotV * NdotL + EPSILON; // prevent division by 0
		vec3 specular = numerator / denominator;

		// Spot Light
		float theta = dot(L, normalize(-spotLight.Direction));
		float epsilon = spotLight.CutOff - spotLight.OuterCutOff;
		float intensity = clamp((theta - spotLight.OuterCutOff) / epsilon, 0.0, 1.0);

		vec3 kS = F * intensity;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - properties.Roughness * intensity;

		Lo += (kD * properties.Albedo / PI + specular) * radiance * NdotL;
	}

	// Ambient lighting
	vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, properties.Roughness);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - properties.Metallic;

	vec3 irradiance = texture(u_SceneProperties.IrradianceMap, N).rgb;
	vec3 diffuse = irradiance * properties.Albedo;

	// sample both the pre-filter map and the BRDF lut and combine them together
	// as per the Split-Sum approximation to get the IBL specular part.
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(u_SceneProperties.PrefilterMap, R, properties.Roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(u_SceneProperties.BRDFLut, vec2(max(dot(N, V), 0.0), properties.Roughness)).rg;
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

	vec3 ambient = (kD * diffuse + specular) * (properties.AO * 5);
	vec3 color = ambient + Lo;

	// Exposure tonemapping
	vec3 mapped = vec3(1.0) - exp(-color * u_SceneProperties.Exposure);

	// Gamma correct
	mapped = pow(mapped, vec3(1.0 / u_SceneProperties.Gamma));

	float alpha = ((u_Material.HasAlbedoMap) ? texture(u_Material.AlbedoMap, textureCoords).a : 1.0);

	// Discard the fragment if it has an alpha of zero
	if (alpha == 0.0)
		discard;

	o_Color = vec4(mapped, alpha) + vec4(properties.Emission, 0.0);
	o_EntityID = fragmentIn.EntityID;
}

// Towbridge-Reitz normal distribuion function
// Uses Epic/Disney's reparameterization of alpha = roughness^2
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float denom = (NdotH2 * (alphaSq - 1.0) + 1.0);
    denom = PI * denom * denom;
	float ggxDistribution = alphaSq / denom;
	
    return ggxDistribution;
}

// Single term for separable Schlick-GGX below
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method
float gaSchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = gaSchlickGGX(NdotV, roughness);
    float ggx1 = gaSchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

// Schlick's approximation of the Fresnel factor
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Schlick's approximation of the Fresnel factor with roughness remapping
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float Attenuate(vec3 lightPosition, vec3 fragPosition, float constant, float linear, float quadratic)
{
	float distance = length(lightPosition - fragPosition);
	return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir)
{
	const float minLayers = 8.0;
	const float maxLayers = 32.0;
	float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));  
	float layerDepth = 1.0 / numLayers;
	float currentLayerDepth = 0.0;

	vec2 p = viewDir.xy * u_Material.ParallaxHeightScale;
	vec2 deltaTexCoords = p / numLayers;

	vec2 currentTexCoords = texCoords;
	float currentDepthValue = texture(u_Material.POMap, currentTexCoords).r;

	while (currentLayerDepth < currentDepthValue)
	{
		currentTexCoords -= deltaTexCoords;
		currentDepthValue = texture(u_Material.POMap, currentTexCoords).r;
		currentLayerDepth += layerDepth;
	}

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth  = currentDepthValue - currentLayerDepth;
    float beforeDepth = texture(u_Material.POMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return currentTexCoords;
}

float ShadowCalculation(vec4 fragPosLightSpace, float NdotL, sampler2D shadowMap, float shadowBias)
{
	// perspective division
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// transform to 0 -> 1 range
	projCoords = projCoords * 0.5 + 0.5;

	// get closest depth value from light's perspective (using [0, 1] range fragPosLightSpace as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r;

	// get the current depth of the fragment from the light's perspective
	float currentDepth = projCoords.z;

	float bias = shadowBias;
	float shadow = 0.0;

	// Percentage Closer Filtering
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	float sampleCount = 18.0;

	for (float x = -1.0; x <= 1.0; x += 0.5)
	{
		for (float y = -1.0; y <= 1.0; y += 0.5)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	// take the average of all the samples
	shadow /= sampleCount;

	if (projCoords.z > 1.0)
		shadow = 0.0;

	return shadow;
}