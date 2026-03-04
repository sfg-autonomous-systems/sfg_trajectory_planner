#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"

#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL.h>
#include <stdexcept>

#include "sfg_trajectory_planner/engine/core/gfx/camera.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/material.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/shader.hpp"

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

    Renderer::Renderer(
        const Camera &camera,
        std::shared_ptr<Material> line_material,
        std::shared_ptr<Material> text_material,
        std::shared_ptr<TextFont> text_font,
        glm::vec3 clear_color)
        : m_camera(camera),
          m_framebuffer(clear_color, m_camera.get_cs_to_ss_vector().zw()),
          m_line_mesh(GL_LINES),
          m_line_material(line_material),
          m_text_mesh(GL_TRIANGLES),
          m_text_material(text_material),
          m_text_font(text_font)
    {
        m_text_material->set_texture("u_FontAtlas", m_text_font->get_font_atlas(), GL_TEXTURE_2D, 0);
        ImGuizmo::AllowAxisFlip(false);
    }

    const Camera &Renderer::get_camera() const
    {
        return m_camera;
    }

    void Renderer::add_line(glm::vec3 start_ws, glm::vec3 end_ws, glm::vec3 color)
    {
        add_line(glm::mat4(1.0f), start_ws, end_ws, color);
    }

    void Renderer::add_line(const glm::mat4 &ls_to_ws_matrix, glm::vec3 start_ls, glm::vec3 end_ls, glm::vec3 color)
    {
        m_line_mesh.add_vertices({{ls_to_ws_matrix * glm::vec4(start_ls, 1.0f), color}});
        m_line_mesh.add_vertices({{ls_to_ws_matrix * glm::vec4(end_ls, 1.0f), color}});
    }

    void Renderer::add_renderable(const glm::mat4 &ls_to_ws_matrix, IRenderable *renderable, Material *material)
    {
        m_render_renderable_requests.push_back({ls_to_ws_matrix, renderable, material});
    }

    void Renderer::add_text(
        glm::vec3 position_ws,
        const std::string &text,
        float font_size_ss,
        glm::vec3 color,
        TextAnchor anchor)
    {
        add_text(glm::mat4(1.0f), position_ws, text, font_size_ss, color, anchor);
    }

    void Renderer::add_text(
        const glm::mat4 &ls_to_ws_matrix,
        glm::vec3 position_ls,
        const std::string &text,
        float font_size_ss,
        glm::vec3 color,
        TextAnchor anchor)
    {
        const glm::mat4 &ws_to_vs_matrix = m_camera.get_ws_to_vs_matrix();
        const glm::mat4 &vs_to_cs_matrix = m_camera.get_vs_to_cs_matrix();
        const glm::vec4 &cs_to_ss_vector = m_camera.get_cs_to_ss_vector();

        glm::vec3 position_ws = ls_to_ws_matrix * glm::vec4(position_ls, 1.0f);
        glm::vec3 position_vs = ws_to_vs_matrix * glm::vec4(position_ws, 1.0f);
        glm::vec4 position_cs = vs_to_cs_matrix * glm::vec4(position_vs, 1.0f);
        auto depth_vs = -position_vs.z;

        if (depth_vs <= 0.0f)
        {
            return;
        }

        auto projection_scaling = vs_to_cs_matrix[1][1];
        auto viewport_height_ss = cs_to_ss_vector.w;
        auto pixel_height_ws = position_cs.w / (projection_scaling * viewport_height_ss * 0.5f);
        auto font_scale_factor = font_size_ss / m_text_font->get_baked_height();
        auto scale = pixel_height_ws * font_scale_factor;

        glm::vec3 camera_right_ws = {ws_to_vs_matrix[0][0], ws_to_vs_matrix[1][0], ws_to_vs_matrix[2][0]};
        glm::vec3 camera_up_ws = {ws_to_vs_matrix[0][1], ws_to_vs_matrix[1][1], ws_to_vs_matrix[2][1]};
        glm::vec2 offset = m_text_font->text_anchor_to_offset(text, anchor);

        stbtt_aligned_quad quad;
        auto x_cursor = 0.0f;
        auto y_cursor = 0.0f;

        RenderTextRequest request;
        request.m_distance_from_camera = depth_vs;

        for (char character : text)
        {
            m_text_font->get_character_quad(character, &x_cursor, &y_cursor, &quad);

            float min_x = (quad.x0 + offset.x) * scale;
            float max_x = (quad.x1 + offset.x) * scale;
            float min_y = -(quad.y1 + offset.y) * scale;
            float max_y = -(quad.y0 + offset.y) * scale;

            glm::vec3 bottom_left_ws = position_ws + (camera_right_ws * min_x) + (camera_up_ws * min_y);
            glm::vec3 bottom_right_ws = position_ws + (camera_right_ws * max_x) + (camera_up_ws * min_y);
            glm::vec3 top_right_ws = position_ws + (camera_right_ws * max_x) + (camera_up_ws * max_y);
            glm::vec3 top_left_ws = position_ws + (camera_right_ws * min_x) + (camera_up_ws * max_y);

            // Add vertices.
            request.m_vertices[0] = {bottom_left_ws, {quad.s0, quad.t1}, color};
            request.m_vertices[1] = {bottom_right_ws, {quad.s1, quad.t1}, color};
            request.m_vertices[2] = {top_right_ws, {quad.s1, quad.t0}, color};
            request.m_vertices[3] = {top_left_ws, {quad.s0, quad.t0}, color};
            m_render_text_requests.push_back(request);
        }
    }

    bool Renderer::add_gizmo(glm::mat4 &ls_to_ws_matrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode, void *id)
    {
        ImGuizmo::PushID(id);
        auto dirty = ImGuizmo::Manipulate(glm::value_ptr(m_camera.get_ws_to_vs_matrix()), glm::value_ptr(m_camera.get_vs_to_cs_matrix()), operation, mode, glm::value_ptr(ls_to_ws_matrix));
        ImGuizmo::PopID();

        return dirty;
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

        auto dirty = ImGuizmo::IsUsingViewManipulate();

        if (dirty)
        {
            // Undo the pretransformation and update the actual view matrix if something has interacted with the view manipulation cube.
            ws_to_vs_matrix = imguizmo_ws_to_vs_matrix * hatch_correction * glm::inverse(basis_alignment);
        }
        return dirty;
    }

    GLuint Renderer::render()
    {
        m_framebuffer.resize_if_needed(m_camera.get_cs_to_ss_vector().zw());
        m_framebuffer.bind();
        m_framebuffer.clear();

        glm::mat4 ws_to_cs_matrix = m_camera.get_vs_to_cs_matrix() * m_camera.get_ws_to_vs_matrix();

        // Render lines.
        if (!m_line_mesh.empty())
        {
            m_line_material->set_uniform("u_WsToCsMatrix", ws_to_cs_matrix);
            m_line_material->bind();

            if (m_line_mesh.get_topology() == GL_LINES)
            {
                glLineWidth(2.0f);
            }
            m_line_mesh.render();
            m_line_material->unbind();
            m_line_mesh.clear();
        }

        // Render renderables.
        if (!m_render_renderable_requests.empty())
        {
            for (const auto &request : m_render_renderable_requests)
            {
                auto ls_to_cs_matrix = ws_to_cs_matrix * request.m_ls_to_ws_matrix;

                request.m_material->set_uniform("u_LsToWsMatrix", request.m_ls_to_ws_matrix);
                request.m_material->set_uniform("u_LsToCsMatrix", ls_to_cs_matrix);
                request.m_material->set_uniform("u_WsToCsMatrix", ws_to_cs_matrix);
                request.m_material->bind();
                request.m_renderable->render();
                request.m_material->unbind();
            }
            m_render_renderable_requests.clear();
        }

        m_framebuffer.enable_transparency(true);

        // Render text.
        if (!m_render_text_requests.empty())
        {
            // Sort the pending text by distance from the camera so that alpha blending works correctly.
            std::sort(
                m_render_text_requests.begin(),
                m_render_text_requests.end(),
                [](const RenderTextRequest &a, const RenderTextRequest &b)
                {
                    return a.m_distance_from_camera > b.m_distance_from_camera;
                });

            std::uint32_t current_index = m_text_mesh.vertex_count();

            for (const auto &request : m_render_text_requests)
            {
                m_text_mesh.add_vertices({request.m_vertices[0], request.m_vertices[1], request.m_vertices[2], request.m_vertices[3]});
                m_text_mesh.add_indices({current_index + 0, current_index + 1, current_index + 2, current_index + 2, current_index + 3, current_index + 0});
                current_index += 4;
            }

            m_text_material->set_uniform("u_WsToCsMatrix", ws_to_cs_matrix);
            m_text_material->bind();
            m_text_mesh.render();
            m_text_material->unbind();
            m_text_mesh.clear();
            m_render_text_requests.clear();
        }
        m_framebuffer.enable_transparency(false);
        m_framebuffer.unbind();

        return m_framebuffer.get_color_texture();
    }
}