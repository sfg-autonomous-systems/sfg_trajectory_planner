#pragma once

#include <glm/glm.hpp>

namespace sfg_trajectory_planner
{
    struct Waypoint
    {
        std::string m_name;
        float m_delay_from_last;
        glm::mat4 m_pose;
    };
}