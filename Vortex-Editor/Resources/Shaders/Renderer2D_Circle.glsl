//-------------------------
// - Sparky Game Engine -
// Renderer2D Circle Shader
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3  a_WorldPosition; // Vertex position in world space
layout (location = 1) in vec3  a_LocalPosition; // Vertex position in local space
layout (location = 2) in vec4  a_Color; // Vertex color
layout (location = 3) in float a_Thickness; // Circle thickness
layout (location = 4) in float a_Fade; // Circle fade out
layout (location = 5) in int   a_EntityID; // Vertex entity ID

out DATA
{
	vec3     LocalPosition;
	vec4     Color;
	float    Thickness;  
	float    Fade;
	flat int EntityID;
} vertexOut;

uniform mat4 u_ViewProjection;

void main()
{
	vertexOut.LocalPosition = a_LocalPosition;
	vertexOut.Color = a_Color;
	vertexOut.Thickness = a_Thickness;
	vertexOut.Fade = a_Fade;

	vertexOut.EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;
layout (location = 2) out vec4 o_BrightColor;

in DATA
{
	vec3     LocalPosition;
	vec4     Color;
	float    Thickness;  
	float    Fade;
	flat int EntityID;
} fragmentIn;

struct LightSource
{
	vec3 Color;
	vec3 Position;
	float Intensity;
};

#define MAX_LIGHT_SOURCES 100

uniform LightSource u_LightSources[MAX_LIGHT_SOURCES];

void main()
{
    // Calculate distance and fill circle with color
    float distance = 1.0 - length(fragmentIn.LocalPosition);
    float circle = smoothstep(0.0, fragmentIn.Fade, distance);
    circle *= smoothstep(fragmentIn.Thickness + fragmentIn.Fade, fragmentIn.Thickness, distance);

    // Discard the pixel/fragment if there is nothing to render
    if (circle == 0.0)
        discard;

    // Set output color
    o_Color = fragmentIn.Color * circle;
	o_EntityID = fragmentIn.EntityID;
	o_BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}