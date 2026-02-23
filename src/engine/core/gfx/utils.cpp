#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <optional>

#include "sfg_trajectory_planner/engine/core/gfx/ray.hpp"

namespace sfg_trajectory_planner::engine::core::gfx::utils
{
    Ray position_ss_to_ray_ws(glm::vec2 position_ss, const glm::mat4 &ws_to_vs_matrix, const glm::mat4 &vs_to_cs_matrix, glm::vec4 cs_to_ss_vector)
    {
        // We need to flip the y coordinate because glm::project assumes the origin is at the bottom-left while ImGui assumes the origin is at the top-left.
        cs_to_ss_vector = glm::vec4(cs_to_ss_vector.x, cs_to_ss_vector.y + cs_to_ss_vector.w, cs_to_ss_vector.z, -cs_to_ss_vector.w);
        glm::vec3 origin_ws = glm::unProject(glm::vec3(position_ss, 0.0f), ws_to_vs_matrix, vs_to_cs_matrix, cs_to_ss_vector);
        glm::vec3 direction_ws = glm::normalize(glm::unProject(glm::vec3(position_ss, 1.0f), ws_to_vs_matrix, vs_to_cs_matrix, cs_to_ss_vector) - origin_ws);
        return {origin_ws, direction_ws};
    }

    glm::vec3 position_ws_to_position_ss(glm::vec3 position_ws, const glm::mat4 &ws_to_vs_matrix, const glm::mat4 &vs_to_cs_matrix, glm::vec4 cs_to_ss_vector)
    {
        cs_to_ss_vector = glm::vec4(cs_to_ss_vector.x, cs_to_ss_vector.y + cs_to_ss_vector.w, cs_to_ss_vector.z, -cs_to_ss_vector.w);
        return glm::project(position_ws, ws_to_vs_matrix, vs_to_cs_matrix, cs_to_ss_vector);
    }

    float intersect_ray_plane(const Ray &ray, glm::vec3 plane_point, glm::vec3 plane_normal)
    {
        auto denominator = glm::dot(ray.direction, plane_normal);

        const float epsilon = 0.0001f;

        if (std::abs(denominator) < epsilon)
        {
            return std::numeric_limits<float>::infinity();
        }
        return glm::dot(plane_point - ray.origin, plane_normal) / denominator;
    }

    float distance_ray_point(const Ray &ray, glm::vec3 point)
    {
        glm::vec3 origin_to_point = point - ray.origin;
        float t = glm::dot(origin_to_point, ray.direction);
        glm::vec3 projection = ray.origin + t * ray.direction;
        return glm::length(point - projection);
    }
}