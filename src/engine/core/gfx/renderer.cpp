#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL.h>
#include <stdexcept>
#include <string>
#include <vector>

#include "sfg_trajectory_planner/engine/core/gfx/camera.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/fonts/roboto_regular.hpp"

static constexpr auto s_line_vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 a_PositionWs;
layout (location = 1) in vec4 a_Color;

uniform mat4 u_WsToCsMatrix;

out vec4 v_Color;

void main() {
    v_Color = a_Color;
    gl_Position = u_WsToCsMatrix * vec4(a_PositionWs, 1.0);
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

static constexpr auto s_text_vertex_shader_source = R"(
#version 330 core
layout(location=0) in vec3 a_PositionWs; 
layout(location=1) in vec2 a_UV; 
layout(location=2) in vec4 a_Color;

uniform mat4 u_WsToCsMatrix; 

out vec2 v_UV; 
out vec4 v_Color;

void main() { 
    v_UV = a_UV; 
    v_Color = a_Color; 
    gl_Position = u_WsToCsMatrix * vec4(a_PositionWs, 1.0); 
})";
static constexpr auto s_text_fragment_shader_source = R"(
#version 330 core
in vec2 v_UV; 
in vec4 v_Color; 

out vec4 f_Color;

uniform sampler2D u_FontAtlas; 

void main() { 
    float alpha = texture(u_FontAtlas, v_UV).r;
    
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

    void Renderer::TextVertex::set_vertex_attributes()
    {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *)offsetof(TextVertex, m_position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *)offsetof(TextVertex, m_uv));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *)offsetof(TextVertex, m_color));
    }

    Renderer::Renderer(const Camera &camera, glm::vec3 clear_color)
        : m_camera(camera),
          m_framebuffer(clear_color, m_camera.get_cs_to_ss_vector().zw()),
          m_line_mesh(Mesh<LineVertex>::Topology::Lines),
          m_line_shader(s_line_vertex_shader_source, s_line_fragment_shader_source),
          m_text_mesh(Mesh<TextVertex>::Topology::Triangles),
          m_text_shader(s_text_vertex_shader_source, s_text_fragment_shader_source),
          m_text_font(fonts::roboto_regular_ttf, 64.0f)
    {
        ImGuizmo::AllowAxisFlip(false);
    }

    const Camera &Renderer::get_camera() const
    {
        return m_camera;
    }

    void Renderer::add_line(const glm::vec3 &start_ws, const glm::vec3 &end_ws, const glm::vec3 &color)
    {
        add_line(glm::mat4(1.0f), start_ws, end_ws, color);
    }

    void Renderer::add_line(const glm::mat4 &ls_to_ws_matrix, const glm::vec3 &start_ls, const glm::vec3 &end_ls, const glm::vec3 &color)
    {
        m_line_mesh.add_vertices({{ls_to_ws_matrix * glm::vec4(start_ls, 1.0f), color}});
        m_line_mesh.add_vertices({{ls_to_ws_matrix * glm::vec4(end_ls, 1.0f), color}});
    }

    bool Renderer::add_gizmo(glm::mat4 &ls_to_ws_matrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode, void *id)
    {
        ImGuizmo::PushID(id);
        auto manipulated = ImGuizmo::Manipulate(glm::value_ptr(m_camera.get_ws_to_vs_matrix()), glm::value_ptr(m_camera.get_vs_to_cs_matrix()), operation, mode, glm::value_ptr(ls_to_ws_matrix));
        ImGuizmo::PopID();

        return manipulated;
    }

    bool Renderer::add_view_gizmo(glm::mat4 &ws_to_vs_matrix, void *id)
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
        glm::mat4 imguizmo_ws_to_vs_matrix = ws_to_vs_matrix * basis_alignment * hatch_correction;

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
            glm::value_ptr(imguizmo_ws_to_vs_matrix),
            view_gizmo_distance,
            ImVec2(m_camera.get_cs_to_ss_vector().x + m_camera.get_cs_to_ss_vector().z - view_gizmo_size, m_camera.get_cs_to_ss_vector().y),
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
            ws_to_vs_matrix = imguizmo_ws_to_vs_matrix * hatch_correction * glm::inverse(basis_alignment);
        }
        return changed;
    }

    void Renderer::add_text(
        const glm::vec3 &position_ws,
        const std::string &text,
        float font_size_ss,
        const glm::vec3 &color,
        TextAnchor anchor)
    {
        add_text(glm::mat4(1.0f), position_ws, text, font_size_ss, color, anchor);
    }

    void Renderer::add_text(
        const glm::mat4 &ls_to_ws_matrix,
        const glm::vec3 &position_ls,
        const std::string &text,
        float font_size_ss,
        const glm::vec3 &color,
        TextAnchor anchor)
    {
        const glm::mat4 &ws_to_vs_matrix = m_camera.get_ws_to_vs_matrix();
        glm::vec3 position_ws = ls_to_ws_matrix * glm::vec4(position_ls, 1.0f);
        glm::vec3 position_vs = ws_to_vs_matrix * glm::vec4(position_ws, 1.0f);
        auto depth = -position_vs.z;

        if (depth <= 0.0f)
        {
            return;
        }

        auto projection_scaling = m_camera.get_vs_to_cs_matrix()[1][1];
        auto viewport_height = m_camera.get_cs_to_ss_vector().w;
        auto pixel_world_size = depth / (projection_scaling * viewport_height * 0.5f);
        auto font_scale_factor = font_size_ss / m_text_font.get_baked_height();
        auto final_scale = pixel_world_size * font_scale_factor;

        glm::vec3 camera_right_ws = {ws_to_vs_matrix[0][0], ws_to_vs_matrix[1][0], ws_to_vs_matrix[2][0]};
        glm::vec3 camera_up_ws = {ws_to_vs_matrix[0][1], ws_to_vs_matrix[1][1], ws_to_vs_matrix[2][1]};
        glm::vec2 offset = m_text_font.text_anchor_to_offset(text, anchor);

        stbtt_aligned_quad quad;
        auto x_cursor = 0.0f;
        auto y_cursor = 0.0f;
        auto current_index = m_text_mesh.vertex_count();

        // Reserve memory to avoid reallocations.
        std::vector<TextVertex> vertices;
        vertices.reserve(text.size() * 4);
        std::vector<uint32_t> indices;
        indices.reserve(text.size() * 6);

        for (char character : text)
        {
            m_text_font.get_character_quad(character, &x_cursor, &y_cursor, &quad);

            float min_x = (quad.x0 + offset.x) * final_scale;
            float max_x = (quad.x1 + offset.x) * final_scale;
            float min_y = -(quad.y1 + offset.y) * final_scale;
            float max_y = -(quad.y0 + offset.y) * final_scale;

            glm::vec3 bottom_left_ws = position_ws + (camera_right_ws * min_x) + (camera_up_ws * min_y);
            glm::vec3 bottom_right_ws = position_ws + (camera_right_ws * max_x) + (camera_up_ws * min_y);
            glm::vec3 top_right_ws = position_ws + (camera_right_ws * max_x) + (camera_up_ws * max_y);
            glm::vec3 top_left_ws = position_ws + (camera_right_ws * min_x) + (camera_up_ws * max_y);

            // Add vertices.
            vertices.push_back({bottom_left_ws, {quad.s0, quad.t1}, color});
            vertices.push_back({bottom_right_ws, {quad.s1, quad.t1}, color});
            vertices.push_back({top_right_ws, {quad.s1, quad.t0}, color});
            vertices.push_back({top_left_ws, {quad.s0, quad.t0}, color});

            // Add Indices.
            indices.push_back(current_index + 0);
            indices.push_back(current_index + 1);
            indices.push_back(current_index + 2);
            indices.push_back(current_index + 2);
            indices.push_back(current_index + 3);
            indices.push_back(current_index + 0);

            current_index += 4;
        }
        m_text_mesh.add_vertices(vertices);
        m_text_mesh.add_indices(indices);
    }

    GLuint Renderer::render()
    {
        if (m_camera.get_cs_to_ss_vector().z > 0 && m_camera.get_cs_to_ss_vector().w > 0)
        {
            m_framebuffer.resize_if_needed(m_camera.get_cs_to_ss_vector().zw());
        }

        m_framebuffer.bind();
        m_framebuffer.clear();

        glm::mat4 ws_to_cs_matrix = m_camera.get_vs_to_cs_matrix() * m_camera.get_ws_to_vs_matrix();

        if (!m_line_mesh.empty())
        {
            m_line_shader.bind();
            glUniformMatrix4fv(glGetUniformLocation(m_line_shader.get_id(), "u_WsToCsMatrix"), 1, GL_FALSE, glm::value_ptr(ws_to_cs_matrix));

            if (m_line_mesh.get_topology() == Mesh<LineVertex>::Topology::Lines)
            {
                glLineWidth(2.0f);
            }
            m_line_mesh.render();
            m_line_shader.unbind();
            m_line_mesh.clear();
        }

        if (!m_text_mesh.empty())
        {
            m_text_shader.bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_text_font.get_font_atlas());
            glUniform1i(glGetUniformLocation(m_text_shader.get_id(), "u_FontAtlas"), 0);
            glUniformMatrix4fv(glGetUniformLocation(m_text_shader.get_id(), "u_WsToCsMatrix"), 1, GL_FALSE, glm::value_ptr(ws_to_cs_matrix));

            m_text_mesh.render();
            m_text_shader.unbind();
            m_text_mesh.clear();
        }
        m_framebuffer.unbind();

        return m_framebuffer.get_color_texture();
    }
}