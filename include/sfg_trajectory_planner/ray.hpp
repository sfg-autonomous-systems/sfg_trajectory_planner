#pragma once

#include <glm/glm.hpp>

namespace sfg_trajectory_planner::gfx_math
{
    struct Ray
    {
        glm::vec3 origin;
        glm::vec3 direction;
    };
}