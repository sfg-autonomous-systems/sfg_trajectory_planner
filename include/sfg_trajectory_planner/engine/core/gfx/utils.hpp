#pragma once

#include <glm/glm.hpp>
#include <imgui/imgui.h>

namespace sfg_trajectory_planner::engine::core::gfx
{
    struct Ray;
}

namespace sfg_trajectory_planner::engine::core::gfx::utils
{
    // Have to be orthonormal to each other, of unit length, and aligned with the world axes.
    static constexpr glm::vec4 s_right = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    static constexpr glm::vec4 s_up = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    static constexpr glm::vec4 s_forward = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

    Ray position_ss_to_ray_ws(const glm::vec2 &position_ss, const glm::mat4 &ws_to_vs_matrix, const glm::mat4 &vs_to_cs_matrix, glm::vec4 cs_to_ss_vector);
    glm::vec3 position_ws_to_position_ss(const glm::vec3 &position_ws, const glm::mat4 &ws_to_vs_matrix, const glm::mat4 &vs_to_cs_matrix, glm::vec4 cs_to_ss_vector);
    float intersect_ray_plane(const Ray &ray, glm::vec3 plane_point, glm::vec3 plane_normal);
    float distance_ray_point(const Ray &ray, glm::vec3 point);
}