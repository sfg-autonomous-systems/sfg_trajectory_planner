#pragma once

// We need to include glad before anything else that includes OpenGL headers to avoid compilation/runtime errors.
#include <glad/glad.h>
#include <rclcpp/rclcpp.hpp>

#include "sfg_imgui_vendor/main_window.hpp"

namespace sfg_trajectory_planner
{
    class TrajectoryPlanner : public rclcpp::Node
    {
    public:
        TrajectoryPlanner(const rclcpp::NodeOptions &options);

    private:
        std::unique_ptr<sfg_imgui_vendor::MainWindow> m_main_window;
    };
}