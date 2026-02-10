#pragma once

#include <glm/glm.hpp>

namespace sfg_trajectory_planner::core::gfx
{
    struct Ray
    {
        glm::vec3 origin;
        glm::vec3 direction;
    };
}