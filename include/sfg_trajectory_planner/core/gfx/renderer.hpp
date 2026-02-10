#pragma once

#include <glad/glad.h>
#include <vector>

#include "sfg_trajectory_planner/core/gfx/vertex.hpp"

namespace sfg_trajectory_planner::core::gfx
{
    class Renderer
    {
    public:
        Renderer();
        ~Renderer();
        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        void add_line(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color);
        void add_line(const glm::mat4 &model_matrix, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color);
        GLuint render(const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, int viewport_width, int viewport_height);

    private:
        void initialize_lazily();
        void resize_fbo(int width, int height);

        bool m_initialized = false;
        std::vector<Vertex> m_line_vertices;
        int m_viewport_width = 0;
        int m_viewport_height = 0;

        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_fbo = 0;
        GLuint m_color_texture = 0;
        GLuint m_depth_rbo = 0;
        GLuint m_shader_program = 0;
    };
}