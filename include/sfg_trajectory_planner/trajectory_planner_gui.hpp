#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <rclcpp/rclcpp.hpp>

#include "sfg_trajectory_planner/trajectory.hpp"
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
        void render_viewport();
        void render_trajectories_inspector();
        void render_transform_inspector();

        Viewport m_viewport;
        std::vector<Trajectory> m_trajectories;
    };
}