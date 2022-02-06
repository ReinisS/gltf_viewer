// Model viewer code for the assignments in Computer Graphics 1TD388/1MD150.
//
// Modify this and other source files according to the tasks in the instructions.
//

#include "gltf_io.h"
#include "gltf_scene.h"
#include "gltf_render.h"
#include "cg_utils.h"
#include "cg_trackball.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <iostream>

// Struct for our application context
struct Context {
    int width = 768;
    int height = 768;
    GLFWwindow *window;
    gltf::GLTFAsset asset;
    gltf::DrawableList drawables;
    cg::Trackball trackball;
    GLuint program;
    GLuint emptyVAO;
    float elapsedTime;
    std::string gltfFilename = "armadillo.gltf";
    bool my_tool_active = true;
    glm::vec3 background_color;
    glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 look_at_direction = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up_direction = glm::vec3(0.0f, 1.0f, 0.0f);
    float fov_deg = 45.0f;
    float aspect_ratio = 1.0f;
    float near_clip = 0.1f;
    float far_clip = 100.0f;
    glm::vec3 scale = glm::vec3(0.5f, 0.5f, 0.5f);
    float rotation_angle_deg = 90.0f;
    glm::vec3 rotation = glm::vec3(1.0, 0.0, 0.0);
    glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 ambient_color = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 diffuse_color = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 specular_color = glm::vec3(1.0f, 1.0f, 1.0f);
    float specular_power = 100.0f;
    glm::vec3 light_position = glm::vec3(1.0f, 1.0f, 1.0f);
    bool display_surface_normals_as_RGB = false;
    bool orthographic_mode_on = false;
    float orthographic_left = -1.0f;
    float orthographic_right = 1.0f;
    float orthographic_bottom = -1.0f;
    float orthographic_top = 1.0f;
    float zoom_factor = 1.0f;
    float zoom_step = 0.1f;
    // Add more variables here...
};

// Returns the absolute path to the src/shader directory
std::string shader_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/src/shaders/";
}

// Returns the absolute path to the assets/gltf directory
std::string gltf_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/assets/gltf/";
}

void do_initialization(Context &ctx)
{
    ctx.program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");

    gltf::load_gltf_asset(ctx.gltfFilename, gltf_dir(), ctx.asset);
    gltf::create_drawables_from_gltf_asset(ctx.drawables, ctx.asset);
}

void draw_scene(Context &ctx)
{
    // Activate shader program
    glUseProgram(ctx.program);

    // Set render state
    glEnable(GL_DEPTH_TEST);  // Enable Z-buffering

    // Define per-scene uniforms
    glUniform1f(glGetUniformLocation(ctx.program, "u_time"), ctx.elapsedTime);

    // glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 1.0f);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_cameraPosition"), 1, &ctx.camera_position[0]);
    glm::mat4 view = glm::mat4(ctx.trackball.orient);
    glm::mat4 look_at = glm::lookAt(ctx.camera_position, 
  		                            ctx.look_at_direction, 
  		                            ctx.up_direction);
    view = look_at * view;
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_view"), 1, GL_FALSE, &view[0][0]);

    glm::mat4 perspective_projection = glm::mat4(1.0f);
    perspective_projection = glm::perspective(glm::radians(ctx.fov_deg * ctx.zoom_factor), ctx.aspect_ratio, ctx.near_clip, ctx.far_clip) * perspective_projection;
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_projection"), 1, GL_FALSE, &perspective_projection[0][0]);
    glm::mat4 orthographic_projection = glm::mat4(1.0f);
    orthographic_projection = glm::ortho(ctx.orthographic_left, ctx.orthographic_right, ctx.orthographic_bottom, ctx.orthographic_top, ctx.near_clip, ctx.far_clip) * orthographic_projection;
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_orthographic"), 1, GL_FALSE, &orthographic_projection[0][0]);

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::scale(transform, ctx.scale);
    transform = glm::rotate(transform, glm::radians(ctx.rotation_angle_deg), ctx.rotation);
    transform = glm::translate(transform, ctx.translation);

    glm::mat4 model = glm::mat4(1.0f);
    model = transform * model;
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_model"), 1, GL_FALSE, &model[0][0]);

    glUniform3fv(glGetUniformLocation(ctx.program, "u_ambientColor"), 1, &ctx.ambient_color[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_diffuseColor"), 1, &ctx.diffuse_color[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_specularColor"), 1, &ctx.specular_color[0]);
    glUniform1f(glGetUniformLocation(ctx.program, "u_specularPower"), ctx.specular_power);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_lightPosition"), 1, &ctx.light_position[0]);
    glUniform1i(glGetUniformLocation(ctx.program, "u_displaySurfaceNormalsAsRGB"), ctx.display_surface_normals_as_RGB);
    glUniform1i(glGetUniformLocation(ctx.program, "u_orthographicModeOn"), ctx.orthographic_mode_on);
    // ...

    // Draw scene
    for (unsigned i = 0; i < ctx.asset.nodes.size(); ++i) {
        const gltf::Node &node = ctx.asset.nodes[i];
        const gltf::Drawable &drawable = ctx.drawables[node.mesh];

        // Define per-object uniforms
        // ...

        // Draw object
        glBindVertexArray(drawable.vao);
        glDrawElements(GL_TRIANGLES, drawable.indexCount, drawable.indexType,
                       (GLvoid *)(intptr_t)drawable.indexByteOffset);
        glBindVertexArray(0);
    }

    // Clean up
    cg::reset_gl_render_state();
    glUseProgram(0);
}

void do_rendering(Context &ctx)
{
    // Clear render states at the start of each frame
    cg::reset_gl_render_state();

    // Clear color and depth buffers
    glClearColor(ctx.background_color[0], ctx.background_color[1], ctx.background_color[2], 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_scene(ctx);
}

void reload_shaders(Context *ctx)
{
    glDeleteProgram(ctx->program);
    ctx->program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");
}

void error_callback(int /*error*/, const char *description)
{
    std::cerr << description << std::endl;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_R && action == GLFW_PRESS) { reload_shaders(ctx); }
}

void char_callback(GLFWwindow *window, unsigned int codepoint)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_CharCallback(window, codepoint);
    if (ImGui::GetIO().WantTextInput) return;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx->trackball.center = glm::vec2(x, y);
        ctx->trackball.tracking = (action == GLFW_PRESS);
    }
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui
    if (ImGui::GetIO().WantCaptureMouse) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    cg::trackball_move(ctx->trackball, float(x), float(y));
}

void scroll_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_ScrollCallback(window, x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    ctx->zoom_factor = ctx->zoom_factor + ctx->zoom_step * y * (-1);
}

void resize_callback(GLFWwindow *window, int width, int height)
{
    // Update window size and viewport rectangle
    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    ctx->width = width;
    ctx->height = height;
    glViewport(0, 0, width, height);
}

void ShowMyWindow(Context &ctx) {
    // Create a window called "Tools", with a menu bar.
    ImGui::Begin("Tools", &ctx.my_tool_active, ImGuiWindowFlags_MenuBar);

    ImGui::ColorEdit3("Background color", &ctx.background_color[0]);
    ImGui::DragFloat3("Camera position", &ctx.camera_position[0]);
    ImGui::DragFloat3("Look at direction", &ctx.look_at_direction[0]);
    ImGui::DragFloat3("Up direction", &ctx.up_direction[0]);
    ImGui::DragFloat("FOV (degrees)", &ctx.fov_deg);
    ImGui::DragFloat("Aspect ratio", &ctx.aspect_ratio);
    ImGui::DragFloat("Near clip", &ctx.near_clip);
    ImGui::DragFloat("Far clip", &ctx.far_clip);
    ImGui::DragFloat3("Scale (X, Y, Z)", &ctx.scale[0]);
    ImGui::DragFloat("Rotation angle (degrees)", &ctx.rotation_angle_deg);
    ImGui::DragFloat3("Rotation (X, Y, Z)", &ctx.rotation[0]);
    ImGui::DragFloat3("Translation (X, Y, Z)", &ctx.translation[0]);
    ImGui::ColorEdit3("Ambient color", &ctx.ambient_color[0]);
    ImGui::ColorEdit3("Diffuse color", &ctx.diffuse_color[0]);
    ImGui::ColorEdit3("Specular color", &ctx.specular_color[0]);
    ImGui::DragFloat("Specular power", &ctx.specular_power);
    ImGui::DragFloat3("Light position (X, Y, Z)", &ctx.light_position[0]);
    ImGui::Checkbox("Display surface normals as RGB", &ctx.display_surface_normals_as_RGB);
    ImGui::Checkbox("Turn on orthographic projection mode", &ctx.orthographic_mode_on);
    ImGui::DragFloat("Orthographic left", &ctx.orthographic_left);
    ImGui::DragFloat("Orthographic right", &ctx.orthographic_right);
    ImGui::DragFloat("Orthographic bottom", &ctx.orthographic_bottom);
    ImGui::DragFloat("Orthographic top", &ctx.orthographic_top);

    ImGui::End();
}

int main(int argc, char *argv[])
{
    Context ctx = Context();
    if (argc > 1) { ctx.gltfFilename = std::string(argv[1]); }

    // Create a GLFW window
    glfwSetErrorCallback(error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Model viewer", nullptr, nullptr);
    glfwMakeContextCurrent(ctx.window);
    glfwSetWindowUserPointer(ctx.window, &ctx);
    glfwSetKeyCallback(ctx.window, key_callback);
    glfwSetCharCallback(ctx.window, char_callback);
    glfwSetMouseButtonCallback(ctx.window, mouse_button_callback);
    glfwSetCursorPosCallback(ctx.window, cursor_pos_callback);
    glfwSetScrollCallback(ctx.window, scroll_callback);
    glfwSetFramebufferSizeCallback(ctx.window, resize_callback);

    // Load OpenGL functions
    if (gl3wInit() || !gl3wIsSupported(3, 3) /*check OpenGL version*/) {
        std::cerr << "Error: failed to initialize OpenGL" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // Initialize ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(ctx.window, false /*do not install callbacks*/);
    ImGui_ImplOpenGL3_Init("#version 330" /*GLSL version*/);

    // Initialize rendering
    glGenVertexArrays(1, &ctx.emptyVAO);
    glBindVertexArray(ctx.emptyVAO);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    do_initialization(ctx);

    // Start rendering loop
    while (!glfwWindowShouldClose(ctx.window)) {
        glfwPollEvents();
        ctx.elapsedTime = glfwGetTime();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // ImGui::ShowDemoWindow();
        ShowMyWindow(ctx);
        do_rendering(ctx);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(ctx.window);
    }

    // Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
    std::exit(EXIT_SUCCESS);
}
