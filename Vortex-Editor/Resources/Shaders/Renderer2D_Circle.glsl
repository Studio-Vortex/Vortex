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

out vec3     f_LocalPosition;
out vec4     f_Color;
out float    f_Thickness;  
out float    f_Fade;
out flat int f_EntityID;

uniform mat4 u_ViewProjection;

void main()
{
	f_LocalPosition = a_LocalPosition;
	f_Color = a_Color;
	f_Thickness = a_Thickness;
	f_Fade = a_Fade;

	f_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out int o_EntityID;

in vec3     f_LocalPosition;
in vec4     f_Color;
in float    f_Thickness;  
in float    f_Fade;
in flat int f_EntityID;

void main()
{
    // Calculate distance and fill circle with color
    float distance = 1.0 - length(f_LocalPosition);
    float circle = smoothstep(0.0, f_Fade, distance);
    circle *= smoothstep(f_Thickness + f_Fade, f_Thickness, distance);

    // Discard the pixel/fragment if there is nothing to render
    if (circle == 0.0)
        discard;

	// Apply Gamma correction
	float gamma = 2.2;
	vec4 finalColor = vec4(pow(f_Color.rgb, vec3(1.0 / gamma)), f_Color.a * circle);

    // Set output color
    o_Color = f_Color * circle;
	o_EntityID = f_EntityID;
}