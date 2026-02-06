#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <rclcpp/rclcpp.hpp>

#include "sfg_trajectory_planner/trajectories_inspector.hpp"
#include "sfg_trajectory_planner/transform_inspector.hpp"
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

        Viewport m_viewport;
        TrajectoriesInspector m_trajectories_inspector;
        TransformInspector m_transform_inspector;
        std::vector<Trajectory> m_trajectories;
    };
}