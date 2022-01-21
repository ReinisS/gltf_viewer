#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
// ...

// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;

// Vertex shader outputs
out vec4 v_color; // output vertex color

void main()
{
    float radius = 0.5;
    float sin_offset = sin(u_time);
    float cos_offset = cos(u_time);
    vec4 position_offset = vec4(sin_offset, cos_offset, 0.0, 0.0) * radius;
    gl_Position = a_position + position_offset;

    float r_offset = 0.0;
    float g_offset = 2.0;
    float b_offset = 4.0;
    float rate = 1.0;
    vec3 color_offset = (vec3(sin(u_time*rate + r_offset), sin(u_time*rate + g_offset), sin(u_time*rate + b_offset)) + vec3(1.0, 1.0, 1.0)) / 2;
    v_color = a_color * vec4(color_offset, 1.0);
}
