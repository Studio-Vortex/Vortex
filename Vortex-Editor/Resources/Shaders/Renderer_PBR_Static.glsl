//---------------------------------------------------------------
// - Vortex Game Engine PBR Static Shader -
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
//     Directional & Omnidirectional Shadow Mapping with PCF
//     Gamma Correction
//---------------------------------------------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_Position;
layout (location = 1) in vec4  a_Color;
layout (location = 2) in vec3  a_Normal;
layout (location = 3) in vec3  a_Tangent;
layout (location = 4) in vec3  a_BiTangent;
layout (location = 5) in vec2  a_TexCoord;
layout (location = 6) in vec2  a_TexScale;
layout (location = 7) in int   a_EntityID;

out DATA
{
	vec3       Position;
	vec4       Color;
	vec3       Normal;
	vec2       TexCoord;
	vec2       TexScale;
	flat int   EntityID;

	vec4       FragPosLight;

	mat3       TBN;
} vertexOut;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
uniform mat4 u_SkyLightProjection;

void main()
{
	vertexOut.Position = vec3(u_Model * vec4(a_Position, 1.0));
	vertexOut.Color = a_Color;

	mat3 model = mat3(u_Model);
	vertexOut.Normal = normalize(model * a_Normal);
	vertexOut.TexCoord = a_TexCoord;
	vertexOut.TexScale = a_TexScale;
	vertexOut.EntityID = a_EntityID;

	vertexOut.FragPosLight = u_SkyLightProjection * vec4(vertexOut.Position, 1.0);

	// Calculate TBN matrix
	vec3 T = normalize(model * a_Tangent);
	vec3 B = normalize(model * a_BiTangent);
	vec3 N = vertexOut.Normal;
	vertexOut.TBN = mat3(T, B, N);

	gl_Position = u_ViewProjection * vec4(vertexOut.Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;
layout (location = 2) out vec4 o_BrightColor;

in DATA
{
	vec3       Position;
	vec4       Color;
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
	float Emission;
	sampler2D EmissionMap;
	float ParallaxHeightScale;
	sampler2D POMap;
	sampler2D AOMap;

	float Opacity;

	bool HasAlbedoMap;
	bool HasNormalMap;
	bool HasMetallicMap;
	bool HasRoughnessMap;
	bool HasEmissionMap;
	bool HasPOMap;
	bool HasAOMap;
};

struct SkyLight
{
	vec3 Radiance;
	vec3 Direction;
	float Intensity;

	sampler2D ShadowMap;
	float ShadowBias;
	bool SoftShadows;
};

struct PointLight
{
	vec3 Radiance;
	vec3 Position;
	float Intensity;

	float ShadowBias;
	float FarPlane;
};

struct SpotLight
{
	vec3 Radiance;
	vec3 Position;
	vec3 Direction;
	float Intensity;

	float CutOff;
	float OuterCutOff;

	float ShadowBias;
};

struct FragmentProperties
{
	vec3 Albedo;
	vec4 Color;
	vec3 Normal;
	float Metallic;
	float Roughness;
	vec3 EmissionMap;
	float Emission;
	float AO;
	float Opacity;
};

struct SceneProperties
{
	bool HasSkyLight;
	int ActivePointLights;
	int ActiveSpotLights;

	samplerCube IrradianceMap;
	samplerCube PrefilterMap;
	sampler2D BRDFLut;

	vec3 CameraPosition;
	float Exposure;
	float Gamma;
	float SkyboxIntensity;
	vec3 BloomThreshold;
};

#define MAX_POINT_LIGHTS 50
#define MAX_SPOT_LIGHTS 50

uniform Material        u_Material;
uniform SkyLight        u_SkyLight;
uniform PointLight      u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight       u_SpotLights[MAX_SPOT_LIGHTS];
uniform SceneProperties u_SceneProperties;
uniform samplerCube     u_PointLightShadowMaps[MAX_POINT_LIGHTS];
uniform sampler2D       u_SpotLightShadowMaps[MAX_SPOT_LIGHTS];

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);
const float PI = 3.14159265359;
const float EPSILON = 0.0001;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float gaSchlickG1(float cosTheta, float k);
float gaSchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace, float NdotL, sampler2D shadowMap, float shadowBias, bool softShadows);
float CubemapShadowCalculation(vec3 fragPos, vec3 lightPos, samplerCube shadowMap, float shadowBias, float farPlane);

void main()
{
	vec3 viewDir = normalize(fragmentIn.TBN * u_SceneProperties.CameraPosition - fragmentIn.TBN * fragmentIn.Position);
	vec2 textureCoords = ((u_Material.HasPOMap) ? ParallaxOcclusionMapping(fragmentIn.TexCoord * fragmentIn.TexScale, viewDir) : fragmentIn.TexCoord * fragmentIn.TexScale);

	FragmentProperties properties;
	properties.Albedo = ((u_Material.HasAlbedoMap) ? pow(texture(u_Material.AlbedoMap, textureCoords).rgb, vec3(u_SceneProperties.Gamma)) * u_Material.Albedo : u_Material.Albedo);
	properties.Color = fragmentIn.Color;
	properties.Normal = ((u_Material.HasNormalMap) ? normalize(fragmentIn.TBN * (texture(u_Material.NormalMap, textureCoords).rgb * 2.0 - 1.0)) : normalize(fragmentIn.Normal));
	properties.Metallic = ((u_Material.HasMetallicMap) ? texture(u_Material.MetallicMap, textureCoords).r : u_Material.Metallic);
	properties.Roughness = ((u_Material.HasRoughnessMap) ? texture(u_Material.RoughnessMap, textureCoords).r : u_Material.Roughness);
	properties.EmissionMap = ((u_Material.HasEmissionMap) ? texture(u_Material.EmissionMap, textureCoords).rgb : vec3(0.0));
	properties.Emission = u_Material.Emission;
	properties.AO = ((u_Material.HasAOMap) ? texture(u_Material.AOMap, textureCoords).r : 1.0);
	properties.Opacity = u_Material.Opacity;

	// keep specular hightlight
	properties.Metallic = max(0.05, properties.Metallic);
	properties.Roughness = min(0.99, properties.Roughness);

	vec3 N = properties.Normal;
	vec3 V = normalize(u_SceneProperties.CameraPosition - fragmentIn.Position);
	vec3 R = reflect(-V, N);

	float NdotV = max(dot(N, V), 0.0);

	// Calculate reflectance at normal incidence, i.e. how much the surface reflects when looking directly at it
	// if dia-electric (like plastic) use F0 of 0.04
	// if it's a metal, use the albedo color as F0 (metallic workflow)
	vec3 F0 = Fdielectric;
	F0 = mix(F0, properties.Albedo, properties.Metallic);

	float totalShadow = 0.0;

	// Reflectance equation
	vec3 Lo = vec3(0.0);

	// Calculate sky-light radiance
	if (u_SceneProperties.HasSkyLight)
	{
		vec3 L = normalize(-u_SkyLight.Direction);
		vec3 H = normalize(V + L);

		vec3 radiance = u_SkyLight.Radiance * u_SkyLight.Intensity;

		// Cook-Torrance Bi-directional Reflectance Distribution Function
		float NDF = DistributionGGX(N, H, properties.Roughness);
		float G = GeometrySmith(N, V, L, properties.Roughness);
		float cosTheta = max(dot(H, V), 0.0);
		vec3 F = FresnelSchlick(cosTheta, F0);

		float NdotL = max(dot(N, L), 0.0);

		float shadow = ShadowCalculation(fragmentIn.FragPosLight, NdotL, u_SkyLight.ShadowMap, u_SkyLight.ShadowBias, u_SkyLight.SoftShadows);
		bool notInShadow = (1.0 - shadow) > 0.0;

		totalShadow += 1.0 - shadow;

		if (notInShadow)
		{
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
			// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
			Lo += (kD * properties.Albedo / PI + specular) * radiance * NdotL;
		}
	}

	for (int i = 0; i < u_SceneProperties.ActivePointLights; i++)
	{
		PointLight pointLight = u_PointLights[i];

		//float shadow = CubemapShadowCalculation(fragmentIn.Position, pointLight.Position, u_PointLightShadowMaps[i], pointLight.ShadowBias, pointLight.FarPlane);
		//bool inShadow = (1.0 - shadow) <= 0.0;

		//if (inShadow)
			//continue;

		vec3 L = normalize(pointLight.Position - fragmentIn.Position);
		vec3 H = normalize(V + L);
		float distance = length(pointLight.Position - fragmentIn.Position);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = pointLight.Radiance * attenuation * pointLight.Intensity;

		// Cook-Torrance Bi-directional Reflectance Distribution Function
		float NDF = DistributionGGX(N, H, properties.Roughness);
		float G = GeometrySmith(N, V, L, properties.Roughness);
		float cosTheta = max(dot(H, V), 0.0);
		vec3 F = FresnelSchlick(cosTheta, F0);

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
		float distance = length(spotLight.Position - fragmentIn.Position);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = spotLight.Radiance * attenuation * spotLight.Intensity;

		// Cook-Torrance Bi-Directional Reflectance Distribution Function
		float NDF = DistributionGGX(N, H, properties.Roughness);
		float G = GeometrySmith(N, V, L, properties.Roughness);
		float cosTheta = max(dot(H, V), 0.0f);
		vec3 F = FresnelSchlick(cosTheta, F0);

		float NdotL = max(dot(N, L), 0.0f);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * NdotV * NdotL + EPSILON; // prevent division by 0
		vec3 specular = numerator / denominator;

		// Spot Light
		float theta = dot(L, normalize(-spotLight.Direction));
		float epsilon = spotLight.CutOff - spotLight.OuterCutOff;
		float intensity = clamp((theta - spotLight.OuterCutOff) / epsilon, 0.0, 1.0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - properties.Roughness;

		Lo += intensity * (kD * properties.Albedo / PI + specular) * radiance * NdotL;
	}

	// Ambient lighting
	vec3 F = FresnelSchlickRoughness(NdotV, F0, properties.Roughness);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - properties.Metallic;

	vec3 irradiance = texture(u_SceneProperties.IrradianceMap, N).rgb;
	vec3 diffuse = irradiance * properties.Albedo;

	// sample both the pre-filter map and the BRDF lut and combine them together
	// as per the Split-Sum approximation to get the IBL specular part.
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(u_SceneProperties.PrefilterMap, R, properties.Roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(u_SceneProperties.BRDFLut, vec2(NdotV, properties.Roughness)).rg;
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y) * u_SceneProperties.SkyboxIntensity;

	vec3 ambient = (kD * diffuse + specular) * (properties.AO);
	vec3 color = ambient + Lo;

	// Exposure tonemapping
	vec3 mapped = vec3(1.0) - exp(-color * u_SceneProperties.Exposure);

	// Gamma correct
	mapped = pow(mapped, vec3(1.0 / u_SceneProperties.Gamma));

	float alpha = properties.Opacity;

	// Discard the fragment if it has an alpha of zero
	if (alpha == 0.0)
		discard;

	vec4 result = vec4(mapped, alpha) * properties.Color;
	
	if (u_Material.HasEmissionMap)
		result += vec4(properties.EmissionMap, 0.0);
	else
		result += vec4(vec3(properties.Emission), 0.0);

	o_Color = result;
	o_EntityID = fragmentIn.EntityID;

	// check whether fragment output is higher than threshold,
	// if so, use output as brighness color
	float brightness = dot(result.rgb, u_SceneProperties.BloomThreshold);

	if (brightness > 1.0)
		o_BrightColor = vec4(result.rgb, 1.0);
	else
		o_BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
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

float ShadowCalculation(vec4 fragPosLightSpace, float NdotL, sampler2D shadowMap, float shadowBias, bool softShadows)
{
	// perspective division
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// transform to 0 -> 1 range
	projCoords = projCoords * 0.5 + 0.5;

	// get closest depth value from light's perspective (using [0, 1] range fragPosLightSpace as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r;

	// get the current depth of the fragment from the light's perspective
	float currentDepth = projCoords.z;

	float bias = max(0.005 * (1.0 - NdotL), shadowBias);
	float shadow = 0.0;

	if (!softShadows)
	{
		shadow = currentDepth > closestDepth ? 1.0 : 0.0;
		if (projCoords.z > 1.0)
			shadow = 0.0;
		
		return shadow;
	}

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

float CubemapShadowCalculation(vec3 fragPos, vec3 lightPos, samplerCube shadowMap, float shadowBias, float farPlane)
{
	vec3 fragToLight = fragPos - lightPos;
	float closestDepth = texture(shadowMap, fragToLight).r;

	closestDepth *= farPlane;

	float currentDepth = length(fragToLight);

	float shadow = currentDepth - shadowBias > closestDepth ? 1.0 : 0.0;

	return shadow;
}