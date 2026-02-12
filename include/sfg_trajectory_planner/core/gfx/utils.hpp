#pragma once

#include "sfg_trajectory_planner/core/gfx/ray.hpp"

#include <glm/glm.hpp>
#include <imgui/imgui.h>

namespace sfg_trajectory_planner::core::gfx::utils
{
    static constexpr glm::vec4 s_right = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    static constexpr glm::vec4 s_up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    static constexpr glm::vec4 s_forward = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

    Ray screen_space_to_ray(const glm::vec2 &position, const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, glm::vec4 viewport);
    glm::vec3 world_to_screen_point(const glm::vec3 &point, const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, glm::vec4 viewport);
    float intersect_ray_plane(const Ray &ray, glm::vec3 plane_point, glm::vec3 plane_normal);
    float distance_ray_point(const Ray &ray, glm::vec3 point);
}