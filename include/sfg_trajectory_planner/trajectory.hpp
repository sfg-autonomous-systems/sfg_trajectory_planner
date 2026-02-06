#pragma once

#include <vector>

#include "sfg_trajectory_planner/waypoint.hpp"

namespace sfg_trajectory_planner
{
    struct Trajectory
    {
        std::string m_name;
        float m_delay_from_start;
        std::vector<Waypoint> m_points;
    };
}