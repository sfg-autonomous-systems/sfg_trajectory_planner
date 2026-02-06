#pragma once

#include "sfg_trajectory_planner/ray.hpp"

#include <glm/glm.hpp>

namespace sfg_trajectory_planner::gfx_math
{
    static constexpr glm::vec4 right = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    static constexpr glm::vec4 up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    static constexpr glm::vec4 forward = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

    Ray screen_space_to_ray(const glm::vec2 &position, const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, glm::vec4 viewport);
    float intersect_ray_plane(const Ray &ray, glm::vec3 plane_point, glm::vec3 plane_normal);
}