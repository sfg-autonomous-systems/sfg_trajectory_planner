#pragma once

#include <glm/glm.hpp>

namespace sfg_trajectory_planner
{
    struct TrajectoryPoint
    {
        float time_from_start;
        glm::mat4 pose;
    };
}