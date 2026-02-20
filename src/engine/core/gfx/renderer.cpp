#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL.h>
#include <stdexcept>
#include <string>
#include <vector>

#include "sfg_trajectory_planner/engine/core/gfx/camera.hpp"

static constexpr auto s_line_width = 2.0f;

static constexpr auto s_line_vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;

uniform mat4 u_ViewProjection;

out vec4 v_Color;

void main() {
    v_Color = a_Color;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
)";
static constexpr auto s_line_fragment_shader_source = R"(
#version 330 core
in vec4 v_Color;
out vec4 f_Color;

void main() {
    f_Color = v_Color;
}
)";

static const auto s_text_vertex_shader_source = R"(
#version 330 core
layout(location=0) in vec3 a_Position; 
layout(location=1) in vec2 a_UV; 
layout(location=2) in vec4 a_Color;

uniform mat4 u_ViewProjection; 

out vec2 v_UV; 
out vec4 v_Color;

void main() { 
    v_UV = a_UV; 
    v_Color = a_Color; 
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0); 
})";
static const auto s_text_fragment_shader_source = R"(
#version 330 core
in vec2 v_UV; 
in vec4 v_Color; 

out vec4 f_Color;

uniform sampler2D u_Font; 

void main() { 
    float alpha = texture(u_Font, v_UV).r;

    if(alpha < 0.1) 
    {
        discard;
    }
    f_Color = vec4(v_Color.rgb, v_Color.a * alpha); 
})";

namespace sfg_trajectory_planner::engine::core::gfx
{
    void Renderer::LineVertex::set_vertex_attributes()
    {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void *)offsetof(LineVertex, m_position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void *)offsetof(LineVertex, m_color));
    }

    Renderer::Renderer(const Camera &camera, glm::vec3 clear_color)
        : m_camera(camera),
          m_framebuffer(clear_color, m_camera.get_viewport().zw()),
          m_line_mesh(Mesh<LineVertex>::Topology::Lines),
          m_line_shader(s_line_vertex_shader_source, s_line_fragment_shader_source)
    {
        ImGuizmo::AllowAxisFlip(false);
    }

    const Camera &Renderer::get_camera() const
    {
        return m_camera;
    }

    void Renderer::add_line(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color)
    {
        add_line(glm::mat4(1.0f), start, end, color);
    }

    void Renderer::add_line(const glm::mat4 &model_matrix, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color)
    {
        m_line_mesh.add_vertices({{model_matrix * glm::vec4(start, 1.0f), color}});
        m_line_mesh.add_vertices({{model_matrix * glm::vec4(end, 1.0f), color}});
    }

    bool Renderer::add_gizmo(glm::mat4 &model_matrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode, void *id)
    {
        ImGuizmo::PushID(id);
        auto manipulated = ImGuizmo::Manipulate(glm::value_ptr(m_camera.get_view_matrix()), glm::value_ptr(m_camera.get_projection_matrix()), operation, mode, glm::value_ptr(model_matrix));
        ImGuizmo::PopID();

        return manipulated;
    }

    bool Renderer::add_view_gizmo(glm::mat4 &view_matrix, void *id)
    {
        const auto view_gizmo_size = 128.0f;
        const auto view_gizmo_distance = 10.0f;
        const auto index_vector = glm::vec3(0.0f, 1.0f, 2.0f);

        // ImGuizmo  internally expects that the
        //     - x column represents the positive right axis, i.e. (1, 0, 0).
        //     - y column represents the positive up axis, i.e. (0, 1, 0).
        //     - z column represents the positive forward axis, i.e. (0, 0, 1).
        // Based on this internal interpretation, ImGuizmo interpolates yaw and pitch angles when the user selects a face on the cube.
        // But since we allow our axiss vectors to be flipped or reoriented, e.g. right could be (0, -1, 0) instead of (1, 0, 0), we need to pretransform
        // our own view matrix into the format that ImGuizmo expects before passing it in, so that the correct yaw and pitch angles are interpolated.
        glm::mat4 basis_alignment = glm::mat4(utils::s_right, utils::s_up, utils::s_forward, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        // Furthermore, since ImGuizmo expects all column vectors to point towards the positive direction, if our own representation points towards the
        // negative direction.
        glm::mat4 hatch_correction = glm::scale(
            glm::mat4(1.0f),
            {glm::sign(glm::dot(utils::s_right, glm::vec4(1.0f))),
             glm::sign(glm::dot(utils::s_up, glm::vec4(1.0f))),
             glm::sign(glm::dot(utils::s_forward, glm::vec4(1.0f)))});

        // Perform the pretransformation to align with ImGuizmo's internal expectation.
        glm::mat4 imguizmo_view_matrix = view_matrix * basis_alignment * hatch_correction;

        auto &style = ImGuizmo::GetStyle();

        ImVec4 old_colors[] = {
            style.Colors[ImGuizmo::DIRECTION_X],
            style.Colors[ImGuizmo::DIRECTION_Y],
            style.Colors[ImGuizmo::DIRECTION_Z]};

        // Since we applied the pretransformation to align with ImGuizmo's internal expectation, we also need to apply the same pretransformation
        // to the gizmo's colors so that the correct colors are shown on the gizmo.
        style.Colors[ImGuizmo::DIRECTION_X] = old_colors[static_cast<size_t>(glm::dot(glm::abs(utils::s_right.xyz()), index_vector))];
        style.Colors[ImGuizmo::DIRECTION_Y] = old_colors[static_cast<size_t>(glm::dot(glm::abs(utils::s_up.xyz()), index_vector))];
        style.Colors[ImGuizmo::DIRECTION_Z] = old_colors[static_cast<size_t>(glm::dot(glm::abs(utils::s_forward.xyz()), index_vector))];

        ImGuizmo::PushID(id);
        ImGuizmo::ViewManipulate(
            glm::value_ptr(imguizmo_view_matrix),
            view_gizmo_distance,
            ImVec2(m_camera.get_viewport().x + m_camera.get_viewport().z - view_gizmo_size, m_camera.get_viewport().y),
            ImVec2(view_gizmo_size, view_gizmo_size),
            0);
        ImGuizmo::PopID();

        style.Colors[ImGuizmo::DIRECTION_X] = old_colors[0];
        style.Colors[ImGuizmo::DIRECTION_Y] = old_colors[1];
        style.Colors[ImGuizmo::DIRECTION_Z] = old_colors[2];

        auto changed = ImGuizmo::IsUsingViewManipulate();

        if (changed)
        {
            // Undo the pretransformation and update the actual view matrix if something has interacted with the view manipulation cube.
            view_matrix = imguizmo_view_matrix * hatch_correction * glm::inverse(basis_alignment);
        }
        return changed;
    }

    void Renderer::add_text(
        const glm::vec3 &position,
        const std::string &text,
        float font_size,
        const glm::vec3 &color,
        TextJustification justification,
        const glm::vec2 &offset_screen_space)
    {
        add_text(glm::mat4(1.0f), position, text, font_size, color, justification, offset_screen_space);
    }

    void Renderer::add_text(
        const glm::mat4 &model_matrix,
        const glm::vec3 &position,
        const std::string &text,
        float font_size,
        const glm::vec3 &color,
        TextJustification justification,
        const glm::vec2 &offset_screen_space)
    {
        glm::vec3 projected = glm::project(
            (model_matrix * glm::vec4(position, 1.0f)).xyz(),
            m_camera.get_view_matrix(),
            m_camera.get_projection_matrix(),
            glm::vec4(0.0f, 0.0f, m_camera.get_viewport().z, m_camera.get_viewport().w));

        if (projected.z < 0.0f || projected.z > 1.0f)
        {
            return;
        }
        auto text_size = ImGui::CalcTextSize(text.c_str());
        text_size.x *= font_size / ImGui::GetFontSize();
        text_size.y *= font_size / ImGui::GetFontSize();

        projected.x += offset_screen_space.x;
        projected.y += offset_screen_space.y;

        switch (justification)
        {
        case TextJustification::TopLeft:
            break;

        case TextJustification::TopCenter:
            projected.x -= 0.5f * text_size.x;
            break;

        case TextJustification::TopRight:
            projected.x -= text_size.x;
            break;

        case TextJustification::CenterLeft:
            projected.y += 0.5f * text_size.y;
            break;

        case TextJustification::Center:
            projected.x -= 0.5f * text_size.x;
            projected.y += 0.5f * text_size.y;
            break;

        case TextJustification::CenterRight:
            projected.x -= text_size.x;
            projected.y += 0.5f * text_size.y;
            break;

        case TextJustification::BottomLeft:
            projected.y += text_size.y;
            break;

        case TextJustification::BottomCenter:
            projected.x -= 0.5f * text_size.x;
            projected.y += text_size.y;
            break;

        case TextJustification::BottomRight:
            projected.x -= text_size.x;
            projected.y += text_size.y;
            break;

        default:
            break;
        }

        ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(m_camera.get_viewport().x + projected.x - 2.0f, m_camera.get_viewport().y + m_camera.get_viewport().w - projected.y - 2.0f),
            ImVec2(m_camera.get_viewport().x + projected.x + text_size.x + 2.0f, m_camera.get_viewport().y + m_camera.get_viewport().w - projected.y + text_size.y + 2.0f),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.75f)));
        ImGui::GetWindowDrawList()->AddText(
            ImGui::GetFont(),
            font_size,
            ImVec2(m_camera.get_viewport().x + projected.x, m_camera.get_viewport().y + m_camera.get_viewport().w - projected.y),
            ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, 1.0f)),
            text.c_str());
    }

    GLuint Renderer::render()
    {
        if (m_camera.get_viewport().z > 0 && m_camera.get_viewport().w > 0)
        {
            m_framebuffer.resize(m_camera.get_viewport().zw());
        }

        m_framebuffer.bind();
        m_framebuffer.clear();

        if (!m_line_mesh.empty())
        {
            // Issue draw call.
            m_line_shader.bind();
            glm::mat4 view_projection_matrix = m_camera.get_projection_matrix() * m_camera.get_view_matrix();
            glUniformMatrix4fv(glGetUniformLocation(m_line_shader.get_id(), "u_ViewProjection"), 1, GL_FALSE, &view_projection_matrix[0][0]);

            if (m_line_mesh.get_topology() == Mesh<LineVertex>::Topology::Lines)
            {
                glLineWidth(s_line_width);
            }
            m_line_mesh.render();
            m_line_shader.unbind();
        }

        m_framebuffer.unbind();
        m_line_mesh.clear();

        return m_framebuffer.get_color_texture();
    }
}