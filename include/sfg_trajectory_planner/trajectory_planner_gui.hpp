#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <rclcpp/rclcpp.hpp>

#include "sfg_trajectory_planner/inspector.hpp"
#include "sfg_trajectory_planner/scene_hierarchy.hpp"
#include "sfg_trajectory_planner/viewport.hpp"

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

        Scene m_scene;
        SceneHierarchy m_scene_hierarchy;
        Viewport m_viewport;
        Inspector m_inspector;
    };
}