#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <memory>
#include <string>
#include <vector>

#include "sfg_trajectory_planner/engine/core/gfx/framebuffer.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/mesh.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/shader.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/text_justification.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Camera;
}

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Renderer
    {
    public:
        Renderer(const Camera &camera, glm::vec3 clear_color = glm::vec3(0.0f, 0.0f, 0.0f));

        const Camera &get_camera() const;

        void add_line(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color);
        void add_line(const glm::mat4 &model_matrix, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color);

        bool add_gizmo(glm::mat4 &model_matrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode, void *id = nullptr);
        bool add_view_gizmo(glm::mat4 &view_matrix, void *id = nullptr);
        void add_text(
            const glm::mat4 &model_matrix,
            const glm::vec3 &position,
            const std::string &text,
            float font_size = 12.0f,
            const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f),
            TextJustification justification = TextJustification::TopLeft,
            const glm::vec2 &offset_screen_space = glm::vec2(0.0f, 0.0f));
        void add_text(
            const glm::vec3 &position,
            const std::string &text,
            float font_size = 12.0f,
            const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f),
            TextJustification justification = TextJustification::TopLeft,
            const glm::vec2 &offset_screen_space = glm::vec2(0.0f, 0.0f));

        GLuint render();

    private:
        struct LineVertex
        {
        public:
            static void set_vertex_attributes();

            glm::vec3 m_position;
            glm::vec3 m_color;
        };

        const Camera &m_camera;
        FrameBuffer m_framebuffer;

        // Line rendering resources.
        Mesh<LineVertex> m_line_mesh;
        Shader m_line_shader;
    };
}