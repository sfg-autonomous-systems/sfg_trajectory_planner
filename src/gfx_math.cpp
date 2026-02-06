#include "sfg_trajectory_planner/gfx_math.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "sfg_trajectory_planner/ray.hpp"

namespace sfg_trajectory_planner::gfx_math
{
    Ray screen_space_to_ray(const glm::vec2 &position, const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, glm::vec4 viewport)
    {
        viewport = glm::vec4(viewport.x, viewport.y + viewport.w, viewport.z, -viewport.w);
        glm::vec3 origin = glm::unProject(glm::vec3(position, 0.0f), view_matrix, projection_matrix, viewport);
        glm::vec3 direction = glm::normalize(glm::unProject(glm::vec3(position, 1.0f), view_matrix, projection_matrix, viewport) - origin);
        return {origin, direction};
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
}