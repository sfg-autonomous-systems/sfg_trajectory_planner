#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
// ImGuizmo needs to be included after imgui.
#include <imgui/ImGuizmo.h>
#include <memory>
#include <string>
#include <vector>

#include "sfg_trajectory_planner/engine/core/gfx/framebuffer.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/mesh.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/shader.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/text_font.hpp"

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

        void add_line(glm::vec3 start_ws, glm::vec3 end_ws, glm::vec3 color);
        void add_line(const glm::mat4 &ls_to_ws_matrix, glm::vec3 start_ls, glm::vec3 end_ls, glm::vec3 color);

        bool add_gizmo(glm::mat4 &ls_to_ws_matrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode, void *id = nullptr);
        bool add_view_gizmo(glm::mat4 &ws_to_vs_matrix, void *id = nullptr);

        void add_text(
            glm::vec3 position_ws,
            const std::string &text,
            float font_size_ss = 12.0f,
            glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f),
            TextAnchor anchor = TextAnchor::TopLeft);
        void add_text(
            const glm::mat4 &ls_to_ws_matrix,
            glm::vec3 position_ls,
            const std::string &text,
            float font_size_ss = 12.0f,
            glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f),
            TextAnchor anchor = TextAnchor::TopLeft);

        GLuint render();

    private:
        struct LineVertex
        {
        public:
            static void set_vertex_attributes();

            glm::vec3 m_position;
            glm::vec3 m_color;
        };

        struct TextVertex
        {
        public:
            static void set_vertex_attributes();

            glm::vec3 m_position;
            glm::vec2 m_uv;
            glm::vec3 m_color;
        };

        const Camera &m_camera;
        FrameBuffer m_framebuffer;

        // Line related resources.
        Mesh<LineVertex> m_line_mesh;
        Shader m_line_shader;

        // Text related resources.
        Mesh<TextVertex> m_text_mesh;
        Shader m_text_shader;
        TextFont m_text_font;
    };
}