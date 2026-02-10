#pragma once

#include <glm/glm.hpp>

namespace sfg_trajectory_planner::gfx_utils
{
    struct Ray
    {
        glm::vec3 origin;
        glm::vec3 direction;
    };
}