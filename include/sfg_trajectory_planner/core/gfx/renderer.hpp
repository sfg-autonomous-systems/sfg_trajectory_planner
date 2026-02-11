#pragma once

#include <glad/glad.h>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <string>
#include <vector>

#include "sfg_trajectory_planner/core/gfx/vertex.hpp"

namespace sfg_trajectory_planner::core::gfx
{
    class Renderer
    {
    public:
        Renderer(glm::vec3 clear_color = glm::vec3(0.0f, 0.0f, 0.0f));
        ~Renderer();
        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        void set_matrices(const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, glm::ivec4 viewport);
        void add_line(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color);
        void add_line(const glm::mat4 &model_matrix, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color);
        bool add_gizmo(glm::mat4 &model_matrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode, void *id = nullptr);
        void add_text(const glm::vec3 &position, const std::string &text);
        void add_text(const glm::vec3 &position, const std::string &text, float font_size);
        void add_text(const glm::vec3 &position, const std::string &text, float font_size, const glm::vec3 &color);
        GLuint render();

    private:
        void initialize_lazily();
        void resize_fbo(int width, int height);

        bool m_initialized = false;
        glm::vec3 m_clear_color;
        std::vector<Vertex> m_line_vertices;

        glm::mat4 m_view_matrix;
        glm::mat4 m_projection_matrix;
        glm::vec4 m_viewport;

        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_fbo = 0;
        GLuint m_color_texture = 0;
        GLuint m_depth_rbo = 0;
        GLuint m_shader_program = 0;
    };
}