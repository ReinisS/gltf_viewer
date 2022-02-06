#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;
uniform vec3 u_diffuseColor;    // The diffuse surface color of the model
uniform vec3 u_lightPosition;   // The position of your light source
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
    // v_color = 0.5 * a_normal + 0.5; // maps the normal direction to an RGB color

    mat4 mv = u_view * u_model;
    // Transform the vertex position to view space (eye coordinates)
    vec3 positionEye = vec3(mv * a_position);
    // Calculate the view-space normal
    vec3 N = normalize(mat3(mv) * a_normal.xyz);
    // Calculate the view-space light direction
    vec3 L = normalize(u_lightPosition - positionEye);
    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N, L));
    // Multiply the diffuse reflection term with the base surface color
    v_color = vec4(diffuse * u_diffuseColor, 1.0);
}
