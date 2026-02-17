#include "sfg_trajectory_planner/engine/core/gfx/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "sfg_trajectory_planner/engine/core/gfx/ray.hpp"
#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    void Camera::set_viewport(const glm::ivec4 &viewport)
    {
        m_viewport = viewport;
    }

    void Camera::set_projection(Projection projection)
    {
        if (projection == m_projection)
        {
            return;
        }
        m_projection = projection;

        switch (m_projection)
        {
        case Projection::Orthographic:
            m_zoom_level = glm::tan(s_vertical_fov / 2.0f) * m_zoom_level;

            break;
        case Projection::Perspective:
            m_zoom_level = m_zoom_level / glm::tan(s_vertical_fov / 2.0f);
            break;
        default:
            break;
        }
    }

    Camera::Projection Camera::get_projection() const
    {
        return m_projection;
    }

    glm::mat4 Camera::get_view_matrix() const
    {
        return m_view_matrix;
    }

    glm::mat4 Camera::get_projection_matrix() const
    {
        return m_projection_matrix;
    }

    glm::ivec4 Camera::get_viewport() const
    {
        return m_viewport;
    }

    void Camera::orbit(float delta_x, float delta_y)
    {
        m_orientation.y += -delta_x;
        m_orientation.x = glm::clamp(m_orientation.x - delta_y, s_min_pitch, s_max_pitch);
    }

    void Camera::zoom(float delta)
    {
        m_zoom_level = glm::clamp(m_zoom_level - delta, s_near_plane, s_far_plane);
    }

    void Camera::pan(const glm::vec2 &position)
    {
        auto ray = screen_point_to_ray(position);
        auto distance = engine::core::gfx::utils::intersect_ray_plane(ray, m_focus_point, glm::inverse(m_view_matrix) * engine::core::gfx::utils::s_forward);
        auto intersection = ray.origin + distance * ray.direction;
        m_focus_point -= intersection - m_pan_start;
    }

    void Camera::start_pan(const glm::vec2 &position)
    {
        auto ray = screen_point_to_ray(position);
        auto distance = engine::core::gfx::utils::intersect_ray_plane(ray, m_focus_point, glm::inverse(m_view_matrix) * engine::core::gfx::utils::s_forward);
        auto intersection = ray.origin + distance * ray.direction;
        m_pan_start = intersection;
    }

    void Camera::focus_on(const glm::vec3 &point)
    {
        m_focus_point = point;
    }

    void Camera::update()
    {
        auto aspect_ratio = static_cast<float>(m_viewport.z) / static_cast<float>(m_viewport.w);

        switch (m_projection)
        {
        case Projection::Orthographic:
            m_projection_matrix = glm::ortho(
                -aspect_ratio * m_zoom_level,
                aspect_ratio * m_zoom_level,
                -m_zoom_level,
                m_zoom_level,
                s_near_plane,
                s_far_plane);
            break;
        case Projection::Perspective:
            m_projection_matrix = glm::perspective(s_vertical_fov, aspect_ratio, s_near_plane, s_far_plane);
            break;
        default:
            break;
        }

        glm::mat4 yaw = glm::rotate(glm::mat4(1.0f), m_orientation.y, engine::core::gfx::utils::s_up.xyz());
        glm::mat4 pitch = glm::rotate(glm::mat4(1.0f), m_orientation.x, engine::core::gfx::utils::s_right.xyz());
        glm::mat4 rotation = yaw * pitch;
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_focus_point);
        // If we are in perspective mode, we need to translate the camera back by the zoom level to maintain the correct distance from the focus point.
        // Otherwise, in orthographic mode, we translate the camera back by half the far plane distance to ensure the entire scene is visible.
        glm::mat4 center_offset = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, m_projection == engine::core::gfx::Camera::Projection::Perspective ? m_zoom_level : engine::core::gfx::Camera::s_far_plane / 2.0f));
        m_view_matrix = glm::inverse(translation * rotation * center_offset);
    }

    void Camera::synchronize_from_matrix(const glm::mat4 &view_matrix)
    {
        glm::mat4 view_inverse = glm::inverse(view_matrix);
        glm::vec3 camera_forward = glm::normalize(view_inverse * engine::core::gfx::utils::s_forward);
        m_orientation.x = -glm::asin(glm::clamp(camera_forward.y, -1.0f, 1.0f));

        const auto epsilon = 0.001f;

        if ((camera_forward.x * camera_forward.x + camera_forward.z * camera_forward.z) > epsilon)
        {
            m_orientation.y = glm::atan(camera_forward.x, camera_forward.z);
        }
    }

    Ray Camera::screen_point_to_ray(const glm::vec2 &point) const
    {
        return engine::core::gfx::utils::screen_space_to_ray(point, m_view_matrix, m_projection_matrix, m_viewport);
    }

    glm::vec3 Camera::world_to_screen_point(const glm::vec3 &point) const
    {
        return engine::core::gfx::utils::world_to_screen_point(point, m_view_matrix, m_projection_matrix, m_viewport);
    }
}