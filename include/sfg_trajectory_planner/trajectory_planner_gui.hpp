#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <rclcpp/rclcpp.hpp>

#include "sfg_trajectory_planner/editor/inspector.hpp"
#include "sfg_trajectory_planner/editor/scene_hierarchy.hpp"
#include "sfg_trajectory_planner/editor/selection_context.hpp"
#include "sfg_trajectory_planner/editor/viewport.hpp"

namespace sfg_trajectory_planner
{
    class TrajectoryPlannerGui : public sfg_imgui_vendor::GuiElement
    {
    public:
        TrajectoryPlannerGui(rclcpp::Node *node);
        TrajectoryPlannerGui(const TrajectoryPlannerGui &) = delete;
        TrajectoryPlannerGui &operator=(const TrajectoryPlannerGui &) = delete;
        TrajectoryPlannerGui(TrajectoryPlannerGui &&) = delete;
        TrajectoryPlannerGui &operator=(TrajectoryPlannerGui &&) = delete;

    protected:
        void render_internal() override;

    private:
        void render_title(std::string_view title, const ImVec2 &position);

        core::Scene m_scene;
        editor::SelectionContext m_selection_context;
        editor::SceneHierarchy m_scene_hierarchy;
        editor::Viewport m_viewport;
        editor::Inspector m_inspector;
    };
}