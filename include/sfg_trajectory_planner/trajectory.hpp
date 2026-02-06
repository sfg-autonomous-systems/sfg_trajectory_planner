#pragma once

#include <vector>

#include "sfg_trajectory_planner/trajectory_point.hpp"

namespace sfg_trajectory_planner
{
    struct Trajectory
    {
        std::vector<TrajectoryPoint> points;
    };
}