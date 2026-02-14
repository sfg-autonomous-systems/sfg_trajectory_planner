#include "sfg_trajectory_planner/trajectory_planner.hpp"

#include "sfg_trajectory_planner/app/trajectory_planner_gui.hpp"

namespace sfg_trajectory_planner
{
    TrajectoryPlanner::TrajectoryPlanner(const rclcpp::NodeOptions &options) : Node("trajectory_planner", options)
    {
        m_main_window = std::make_unique<sfg_imgui_vendor::MainWindow>(
            get_name(),
            std::make_unique<sfg_trajectory_planner::app::TrajectoryPlannerGui>(this),
            ImVec2(800, 600),
            ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}