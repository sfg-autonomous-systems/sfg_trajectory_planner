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
        Viewport(rclcpp::Node *node, core::Scene &scene, core::gfx::Camera &camera, core::gfx::Renderer &renderer, SelectionContext &selection_context);
        void render_internal() override;

    private:
        void process_input();
        void render_object(core::SceneObject &object);
        void render_settings();

        // Ros parameters
        float m_orbit_speed = 0.005f;
        float m_zoom_speed = 1.0f;

        core::Scene &m_scene;
        core::gfx::Camera &m_camera;
        core::gfx::Renderer &m_renderer;
        SelectionContext &m_selection_context;
    };
}