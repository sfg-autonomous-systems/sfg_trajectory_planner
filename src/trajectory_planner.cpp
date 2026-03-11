#include "sfg_trajectory_planner/trajectory_planner.hpp"

#include <SDL3/SDL.h>

#include "sfg_trajectory_planner/app/trajectory_planner_gui.hpp"

namespace sfg_trajectory_planner
{
    TrajectoryPlanner::TrajectoryPlanner(const rclcpp::NodeOptions &options) : Node("trajectory_planner", options)
    {
        m_main_window = std::make_unique<sfg_imgui_vendor::MainWindow>(
            get_name(),
            [this]()
            {
                if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
                {
                    throw std::runtime_error("Failed to initialize GLAD");
                }
                return std::make_unique<sfg_trajectory_planner::app::TrajectoryPlannerGui>(this);
            },
            sfg_imgui_vendor::MainWindow::Flags::Resizable);
    }
}