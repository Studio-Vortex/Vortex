//-------------------------
// - Vortex Game Engine Renderer Shadow Map Shader -
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3 a_Position;

uniform mat4 u_LightProjection;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_LightProjection * u_Model * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

void main()
{
    //gl_FragDepth = gl_FragCoord.z;
}