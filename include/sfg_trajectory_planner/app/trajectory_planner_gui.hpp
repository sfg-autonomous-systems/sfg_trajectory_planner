#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <rclcpp/rclcpp.hpp>

#include "sfg_trajectory_planner/engine/core/gfx/renderer.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_hierarchy.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"
#include "sfg_trajectory_planner/engine/editor/viewport.hpp"

namespace sfg_trajectory_planner::app
{
    class TrajectoryPlannerGui : public sfg_imgui_vendor::GuiElement
    {
    public:
        TrajectoryPlannerGui(rclcpp::Node *node);

    protected:
        void render_internal() override;

    private:
        void render_title(std::string_view title, const ImVec2 &position);

        engine::core::SceneObjectFactory m_factory;
        engine::core::Scene m_scene;
        engine::core::gfx::Camera m_camera;
        engine::core::gfx::Renderer m_renderer;
        engine::editor::SelectionContext m_selection_context;
        engine::editor::SceneHierarchy m_scene_hierarchy;
        engine::editor::Viewport m_viewport;

        // ToDo: Integrate editor here.
    };
}