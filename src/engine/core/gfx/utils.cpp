#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <optional>

#include "sfg_trajectory_planner/engine/core/gfx/ray.hpp"

namespace sfg_trajectory_planner::engine::core::gfx::utils
{
    Ray screen_space_to_ray(const glm::vec2 &position, const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, glm::vec4 viewport)
    {
        // We need to flip the y coordinate because glm::project assumes the origin is at the bottom-left while ImGui assumes the origin is at the top-left.
        viewport = glm::vec4(viewport.x, viewport.y + viewport.w, viewport.z, -viewport.w);
        glm::vec3 origin = glm::unProject(glm::vec3(position, 0.0f), view_matrix, projection_matrix, viewport);
        glm::vec3 direction = glm::normalize(glm::unProject(glm::vec3(position, 1.0f), view_matrix, projection_matrix, viewport) - origin);
        return {origin, direction};
    }

    glm::vec3 world_to_screen_point(const glm::vec3 &point, const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, glm::vec4 viewport)
    {
        viewport = glm::vec4(viewport.x, viewport.y + viewport.w, viewport.z, -viewport.w);
        return glm::project(point, view_matrix, projection_matrix, viewport);
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