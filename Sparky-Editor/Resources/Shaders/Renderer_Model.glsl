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
} fragmentIn;

struct Material
{
	vec3 Ambient;
	sampler2D DiffuseMap;
	sampler2D SpecularMap;
	sampler2D NormalMap;
	float Shininess;
	
	bool IsTextured;
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

#define MAX_POINT_LIGHTS 2

uniform sampler2D        u_Texture;
uniform vec3             u_CameraPosition;
uniform Material         u_Material;
uniform DirectionalLight u_DirectionalLight;
uniform PointLight       u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight        u_SpotLight;

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
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.Diffuse;

	// Diffuse shading
	vec3 normal = properties.Normal;
	vec3 lightDir = properties.TBN * normalize(-lightSource.Direction);
	float diff = CalculateDiffuse(normal, lightDir);
	vec3 diffuse = lightSource.Diffuse * diff * properties.Diffuse;

	// Specular shading
	vec3 viewDir = properties.TBN * normalize(properties.TangentViewPos - fragmentIn.Position);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = CalculateSpecular(normal, halfwayDir, material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.Specular;

	return lightSource.Color * (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.Diffuse;

	vec3 tangentLightPos = properties.TBN * lightSource.Position;

	// Diffuse shading
	vec3 normal = properties.Normal;
	vec3 lightDir = properties.TBN * normalize(tangentLightPos - properties.TangentFragPos);
	float diff = CalculateDiffuse(normal, lightDir);
	vec3 diffuse = lightSource.Diffuse * diff * properties.Diffuse;

	// Specular shading
	vec3 viewDir = properties.TBN * normalize(properties.TangentViewPos - properties.TangentFragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = CalculateSpecular(normal, halfwayDir, material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.Specular;

	// Attenuation
	float attenuation = CalculateAttenuation(tangentLightPos, properties.TangentFragPos, lightSource.Constant, lightSource.Linear, lightSource.Quadratic);
	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;

	return lightSource.Color * (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.Diffuse;

	vec3 tangentLightPos = properties.TBN * lightSource.Position;

	// Diffuse shading
	vec3 normal = properties.Normal;
	vec3 lightDir = properties.TBN * normalize(tangentLightPos - properties.TangentFragPos);
	float diff = CalculateDiffuse(normal, lightDir);
	vec3 diffuse = lightSource.Diffuse * diff * properties.Diffuse;

	// Specular shading
	vec3 viewDir = properties.TBN * normalize(properties.TangentViewPos - properties.TangentFragPos);
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
	float attenuation = CalculateAttenuation(tangentLightPos, properties.TangentFragPos, lightSource.Constant, lightSource.Linear, lightSource.Quadratic);
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	return lightSource.Color * (ambient + diffuse + specular);
}

void main()
{
	vec2 textureScale = fragmentIn.TexCoord * fragmentIn.TexScale;

	vec3 diffuse = texture(u_Material.DiffuseMap, textureScale).rgb;
	vec3 specular = texture(u_Material.SpecularMap, textureScale).rgb;
	vec3 normal = texture(u_Material.NormalMap, textureScale).rgb;
	normal = normal * 2.0 - 1.0;

	FragmentProperties properties;
	properties.Diffuse = diffuse;
	properties.Specular = specular;

	vec3 N = normalize(fragmentIn.Normal);
	vec3 T = normalize(fragmentIn.Tangent.xyz);
	T = (T - dot(T, N) * N);
	vec3 B = cross(normalize(fragmentIn.Normal), fragmentIn.Tangent.xyz) * fragmentIn.Tangent.w;
	mat3 TBN = mat3(T, B, N);
	TBN = transpose(TBN);

	properties.TBN = TBN;
	properties.TangentFragPos = TBN * fragmentIn.Position;
	properties.TangentViewPos = TBN * u_CameraPosition;
	properties.Normal = normalize(TBN * normal);

	vec3 lightColor = vec3(0.0);

	// Phase 1: Directional lighting
	lightColor += CalculateDirectionalLight(u_DirectionalLight, u_Material, properties);

	// Phase 2: Point lights
	for (int i = 0; i  < MAX_POINT_LIGHTS; i++)
		lightColor += CalculatePointLight(u_PointLights[i], u_Material, properties);

	// Phase 3: Spot light
	lightColor += CalculateSpotLight(u_SpotLight, u_Material, properties);

	vec4 fragColor = texture(u_Texture, textureScale);

	// Discard the pixel/fragment if it has an alpha of zero
	if (fragColor.a == 0.0)
		discard;

	// Apply Gamma correction
	float gamma = 2.2;
	vec4 finalColor = vec4(pow(lightColor * fragColor.rgb, vec3(1.0 / gamma)), fragColor.a);

	// Set the output color
	o_Color = finalColor;
	o_EntityID = fragmentIn.EntityID;
}