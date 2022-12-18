//-------------------------
// - Vortex Game Engine Equirectangular To Cubemap Shader -
//-------------------------

#type vertex
#version 460 core

layout (location = 0) in vec3 a_Position;

out vec3 f_LocalPosition;

uniform mat4 u_ViewProjection;

void main()
{
    f_LocalPosition = a_Position;
    gl_Position = a_ViewProjection * vec4(a_LocalPosition, 1.0);
}


#type fragment
#version 460 core

layout (location = 0) out vec4 o_Color;

in vec3 f_LocalPosition;

uniform sampler2D u_EquirectangularMap;

const vec2 inverseAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 p)
{
    vec2 uv = vec2(atan(p.z, p.x), asin(v.y));
    uv *= inverseAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(f_LocalPosition));
    vec3 color = texture(u_EquirectangularMap, uv).rgb;

    o_Color = vec4(color, 1.0);
}