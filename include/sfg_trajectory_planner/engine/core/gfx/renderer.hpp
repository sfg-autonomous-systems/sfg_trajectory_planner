#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <string>
#include <vector>

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Camera;
    class Vertex;
}

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Renderer
    {
    public:
        Renderer(const Camera &camera, glm::vec3 clear_color = glm::vec3(0.0f, 0.0f, 0.0f));
        ~Renderer();
        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        const Camera &get_camera() const;

        void add_line(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color);
        void add_line(const glm::mat4 &model_matrix, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color);
        bool add_gizmo(glm::mat4 &model_matrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode, void *id = nullptr);
        bool add_view_gizmo(glm::mat4 &view_matrix, void *id = nullptr);
        void add_text(const glm::vec3 &position, const std::string &text);
        void add_text(const glm::vec3 &position, const std::string &text, float font_size);
        void add_text(const glm::vec3 &position, const std::string &text, float font_size, const glm::vec3 &color);
        GLuint render();

    private:
        struct Mesh
        {
            std::vector<Vertex> vertices;
            GLuint vao = 0;
            GLuint vbo = 0;
        };

        void initialize_lazily();
        void resize_fbo(int width, int height);

        bool m_initialized = false;
        const Camera &m_camera;
        glm::vec3 m_clear_color;
        Mesh m_line_mesh;

        GLuint m_fbo = 0;
        GLuint m_color_texture = 0;
        GLuint m_depth_rbo = 0;
        GLuint m_shader_program = 0;
    };
}