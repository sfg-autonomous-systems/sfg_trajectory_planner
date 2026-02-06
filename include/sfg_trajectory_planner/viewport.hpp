#pragma once

#include <glm/glm.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <rclcpp/rclcpp.hpp>

#include "sfg_imgui_vendor/gui_element.hpp"

namespace sfg_trajectory_planner
{
    class Viewport : public sfg_imgui_vendor::GuiElement
    {
    public:
        Viewport(rclcpp::Node *node);

        void render_internal() override;

    private:
        struct Config
        {
            static constexpr float c_view_gizmo_size = 128.0f;
            static constexpr float c_view_gizmo_distance = 10.0f;

            // Ros parameters
            glm::vec3 m_grid_origin = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 m_grid_scale = glm::vec3(1.0f, 1.0f, 1.0f);
            float m_grid_size = 10.0f;
        };

        struct Camera
        {
            enum class Projection
            {
                Perspective,
                Orthographic
            };

            static constexpr float c_near_plane = 0.1f;
            static constexpr float c_far_plane = 1000.0f;
            static constexpr float c_vertical_fov = glm::radians(45.0f);
            static constexpr float c_min_pitch = glm::radians(-90.0f);
            static constexpr float c_max_pitch = glm::radians(90.0f);

            // Ros parameters
            float m_orbit_speed = 0.005f;
            float m_zoom_speed = 1.0f;

            // Camera state
            Projection m_projection = Projection::Perspective;
            glm::vec3 m_orientation = {0.0f, glm::radians(-30.0f), glm::radians(45.0f)};
            glm::vec3 m_focus_point = {0.0f, 0.0f, 0.0f};
            float m_zoom_level = 10.0f;
            glm::vec3 m_pan_start;
            glm::mat4 m_view_matrix;
            glm::mat4 m_projection_matrix;
            glm::vec4 m_viewport;
        };

        void render_line(glm::vec3 start, glm::vec3 end, ImU32 color, float thickness = 1.0f);
        void update_camera_matrices();

        Config m_config;
        Camera m_camera;
        glm::mat4 m_grid_matrix;
        ImGuizmo::MODE m_gizmo_mode = ImGuizmo::LOCAL;
        ImGuizmo::OPERATION m_gizmo_operation = ImGuizmo::TRANSLATE;
    };
}