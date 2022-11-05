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

	float CutOff;
	float OuterCutOff;
};

struct FragmentProperties
{
	vec4 FragColor;
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

vec4 CalculateDirectionalLight(DirectionalLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.DiffuseMap;

	// Diffuse shading
	vec3 norm = normalize(f_Normal);
	vec3 lightDir = normalize(-lightSource.Direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = lightSource.Diffuse * diff * properties.DiffuseMap;

	// Specular shading
	vec3 viewDir = normalize(u_CameraPosition - f_Position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.SpecularMap;

	vec3 lightResult = ambient + diffuse + specular;

	return vec4(lightSource.Color * lightResult * properties.FragColor.rbg, properties.FragColor.a);
}

vec4 CalculatePointLight(PointLight lightSource, Material material, FragmentProperties properties)
{
	// Ambient shading
	vec3 ambient = lightSource.Ambient * material.Ambient * properties.DiffuseMap;

	// Diffuse shading
	vec3 norm = normalize(f_Normal);
	vec3 lightDir = normalize(lightSource.Position - f_Position);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = lightSource.Diffuse * diff * properties.DiffuseMap;

	// Specular shading
	vec3 viewDir = normalize(u_CameraPosition - f_Position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);
	vec3 specular = lightSource.Specular * spec * properties.SpecularMap;

	// Attenuation shading
	float distance = length(lightSource.Position - f_Position);
	float attenuation = 1.0 / (lightSource.Constant + lightSource.Linear * distance + lightSource.Quadratic * (distance * distance));
	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;

	vec3 lightResult = ambient + diffuse + specular;

	return vec4(lightSource.Color * lightResult * properties.FragColor.rbg, properties.FragColor.a);
}

vec4 CalculateSpotLight(SpotLight lightSource, Material material, FragmentProperties properties)
{
	vec3 lightDir = normalize(lightSource.Position - f_Position);

	float theta = dot(lightDir, normalize(-lightSource.Direction));
	float epsilon = lightSource.CutOff - lightSource.OuterCutOff;
	float intensity = clamp((theta - lightSource.OuterCutOff) / epsilon, 0.0, 1.0);

	vec4 result;

	// remember that we're working with angles as cosines instead of degrees so a '>' is used
	if (theta > lightSource.CutOff)
	{
		// Ambient shading
		vec3 ambient = lightSource.Ambient * material.Ambient * properties.DiffuseMap;

		// Diffuse shading
		vec3 norm = normalize(f_Normal);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = lightSource.Diffuse * diff * properties.DiffuseMap;

		// Specular shading
		vec3 viewDir = normalize(u_CameraPosition - f_Position);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);
		vec3 specular = lightSource.Specular * spec * properties.SpecularMap;

		diffuse *= intensity;
		specular *= intensity;

		vec3 lightResult = ambient + diffuse + specular;

		result = vec4(lightSource.Color * lightResult * properties.FragColor.rbg, properties.FragColor.a);
	}
	else
		result = vec4(vec4(lightSource.Ambient, 1.0) * texture(material.Diffuse, f_TexCoord * f_TexScale)) * properties.FragColor;

	return result;
}

void main()
{
	vec4 color;

	vec4 fragColor = texture(u_Texture, f_TexCoord * f_TexScale);
	vec3 diffuseMap = texture(u_Material.Diffuse, f_TexCoord * f_TexScale).rgb;
	vec3 specularMap = texture(u_Material.Specular, f_TexCoord * f_TexScale).rgb;

	FragmentProperties properties;
	properties.FragColor = fragColor;
	properties.DiffuseMap = diffuseMap;
	properties.SpecularMap = specularMap;

	// Phase 1: Directional lighting
	color += CalculateDirectionalLight(u_DirectionalLight, u_Material, properties);

	// Phase 2: Point lights
	for (int i = 0; i  < MAX_POINT_LIGHTS; i++)
		color += CalculatePointLight(u_PointLights[i], u_Material, properties);

	// Phase 3: Spot light
	color += CalculateSpotLight(u_SpotLight, u_Material, properties);

	// Discard the pixel/fragment if it has an alpha of zero
	if (color.a == 0.0)
		discard;

	vec4 texCoords = vec4(f_TexCoord, 0.0, 1.0);
	vec4 normals = vec4(f_Normal, 1.0);

	// Set the output color
	o_Color = color;
	o_EntityID = f_EntityID;
}