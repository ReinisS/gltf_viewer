#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_orthographic;
uniform mat4 u_model;
uniform vec3 u_cameraPosition;
uniform vec3 u_ambientColor;
uniform vec3 u_diffuseColor;    // The diffuse surface color of the model
uniform vec3 u_specularColor;
uniform float u_specularPower;
uniform vec3 u_lightPosition;   // The position of your light source
uniform bool u_displaySurfaceNormalsAsRGB;
uniform bool u_orthographicModeOn;
// ...

// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec4 a_normal;

// Vertex shader outputs
out vec4 v_color; // output vertex color

void main()
{
    mat4 projection_matrix;
    if (u_orthographicModeOn) {
        projection_matrix = u_orthographic;
    }
    else {
        projection_matrix = u_projection;
    }
    mat4 mvp_matrix = projection_matrix * u_view * u_model;
    gl_Position = mvp_matrix * a_position;
    if (u_displaySurfaceNormalsAsRGB) {
        v_color = 0.5 * a_normal + 0.5; // maps the normal direction to an RGB color
    }
    else {
        mat4 mv = u_view * u_model;
        // Transform the vertex position to view space (eye coordinates)
        vec3 positionEye = vec3(mv * a_position);
        // Calculate the view-space normal
        vec3 N = normalize(mat3(mv) * a_normal.xyz);
        // Calculate the view-space light direction
        vec3 L = normalize(u_lightPosition - positionEye);
        // Calculate the diffuse (Lambertian) reflection term
        float diffuse = max(0.0, dot(N, L));
        // Calculate the halfway vector
        vec3 E = normalize(u_cameraPosition - positionEye);
        vec3 H = normalize(L + E);
        // Calculate the specular term
        float specular = pow(max(0.0, dot(N, H)), u_specularPower);
        // Combine the Blinn-Phong terms
        v_color = vec4(u_ambientColor + diffuse * u_diffuseColor + specular * u_specularColor, 1.0);
    }
}
