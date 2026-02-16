#pragma once

#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <rclcpp/rclcpp.hpp>

#include "sfg_imgui_vendor/gui_element.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class Scene;
    class SceneObject;

}

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Camera;
    class Renderer;

}

namespace sfg_trajectory_planner::engine::editor
{
    class SelectionContext;
}

namespace sfg_trajectory_planner::engine::editor
{
    class Viewport : public sfg_imgui_vendor::GuiElement
    {
    public:
        Viewport(rclcpp::Node *node, engine::core::Scene &scene, engine::core::gfx::Camera &camera, engine::core::gfx::Renderer &renderer, SelectionContext &selection_context);
        void render_internal() override;

    private:
        struct Config
        {
            static constexpr float s_view_gizmo_size = 128.0f;
            static constexpr float s_view_gizmo_distance = 10.0f;

            // Ros parameters
            float m_orbit_speed = 0.005f;
            float m_zoom_speed = 1.0f;
        };

        void process_input();
        void render_object(engine::core::SceneObject &object);
        void render_settings();

        engine::core::Scene &m_scene;
        engine::core::gfx::Camera &m_camera;
        engine::core::gfx::Renderer &m_renderer;
        SelectionContext &m_selection_context;

        Config m_config;
    };
}