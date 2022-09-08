// Basic Shader that colors a vertex based on its position

#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

out vec3 f_Position;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	f_Position = a_Position;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 gl_Color;

in vec3 f_Position;

void main()
{
	gl_Color = vec4(f_Position * 0.5 + 0.5, 1.0);
}