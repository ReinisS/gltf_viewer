#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;
// ...

// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec4 a_normal;

// Vertex shader outputs
out vec4 v_color; // output vertex color

void main()
{
    mat4 mvp_matrix = u_projection * u_view * u_model;
    gl_Position = mvp_matrix * a_position;
    v_color = 0.5 * a_normal + 0.5; // maps the normal direction to an RGB color
}
