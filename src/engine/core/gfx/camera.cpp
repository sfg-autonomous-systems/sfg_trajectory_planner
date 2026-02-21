#include "sfg_trajectory_planner/engine/core/gfx/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "sfg_trajectory_planner/engine/core/gfx/ray.hpp"

namespace sfg_trajectory_planner::engine::core::gfx
{
    void Camera::set_cs_to_ss_vector(const glm::ivec4 &viewport)
    {
        m_cs_to_ss_vector = viewport;
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

    glm::mat4 Camera::get_ws_to_vs_matrix() const
    {
        return m_ws_to_vs_matrix;
    }

    glm::mat4 Camera::get_vs_to_cs_matrix() const
    {
        return m_vs_to_cs_matrix;
    }

    glm::ivec4 Camera::get_cs_to_ss_vector() const
    {
        return m_cs_to_ss_vector;
    }

    void Camera::orbit(float delta_x, float delta_y)
    {
        m_pitch = glm::clamp(m_pitch - delta_y, s_min_pitch, s_max_pitch);
        m_yaw -= delta_x;
    }

    void Camera::zoom(float delta)
    {
        m_zoom_level = glm::clamp(m_zoom_level - delta, s_near_plane, s_far_plane);
    }

    void Camera::start_pan(const glm::vec2 &position_ss)
    {
        auto ray_ws = screen_point_to_ray(position_ss);
        auto distance_ws = utils::intersect_ray_plane(ray_ws, m_focus_point_ws, glm::inverse(m_ws_to_vs_matrix)[2]);
        glm::vec3 intersection_ws = ray_ws.origin + distance_ws * ray_ws.direction;
        m_pan_start_ws = intersection_ws;
    }

    void Camera::pan(const glm::vec2 &position_ss)
    {
        auto ray_ws = screen_point_to_ray(position_ss);
        auto distance_ws = utils::intersect_ray_plane(ray_ws, m_focus_point_ws, glm::inverse(m_ws_to_vs_matrix)[2]);
        glm::vec3 intersection_ws = ray_ws.origin + distance_ws * ray_ws.direction;
        m_focus_point_ws -= intersection_ws - m_pan_start_ws;
    }

    void Camera::focus_on(const glm::vec3 &point_ws)
    {
        m_focus_point_ws = point_ws;
    }

    void Camera::update()
    {
        auto aspect_ratio = static_cast<float>(m_cs_to_ss_vector.z) / static_cast<float>(m_cs_to_ss_vector.w);

        switch (m_projection)
        {
            case Projection::Orthographic:
                m_vs_to_cs_matrix = glm::ortho(
                    -aspect_ratio * m_zoom_level,
                    aspect_ratio * m_zoom_level,
                    -m_zoom_level,
                    m_zoom_level,
                    s_near_plane,
                    s_far_plane);
                break;
            case Projection::Perspective:
                m_vs_to_cs_matrix = glm::perspective(s_vertical_fov, aspect_ratio, s_near_plane, s_far_plane);
                break;
            default:
                break;
        }

        glm::mat4 base_rotation = glm::inverse(glm::lookAt(glm::vec3(0.0f), utils::s_forward.xyz(), utils::s_up.xyz()));
        glm::mat4 yaw = glm::rotate(glm::mat4(1.0f), m_yaw, utils::s_up.xyz());
        glm::mat4 pitch = glm::rotate(glm::mat4(1.0f), m_pitch, -glm::abs(utils::s_right.xyz()));
        glm::mat4 rotation = yaw * pitch * base_rotation;
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_focus_point_ws);
        // If we are in perspective mode, we need to translate the camera back by the zoom level to maintain the correct distance from the focus point.
        // Otherwise, in orthographic mode, we translate the camera back by half the far plane distance to ensure the entire scene is visible.
        auto distance = m_projection == Camera::Projection::Perspective ? m_zoom_level : Camera::s_far_plane / 2.0f;
        glm::mat4 center_offset = glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, distance});
        m_ws_to_vs_matrix = glm::inverse(translation * rotation * center_offset);
    }

    void Camera::synchronize_from_matrix(const glm::mat4 &ws_to_vs_matrix)
    {
        glm::mat4 vs_to_ws_matrix = glm::inverse(ws_to_vs_matrix);
        glm::vec3 camera_forward_ws = glm::normalize(-vs_to_ws_matrix[2].xyz());
        auto camera_height_ws = glm::dot(camera_forward_ws, utils::s_up.xyz());
        m_pitch = glm::asin(glm::clamp(camera_height_ws, -1.0f, 1.0f));

        const auto epsilon = 0.999f;

        if (camera_height_ws * camera_height_ws < epsilon)
        {
            m_yaw = glm::atan(glm::dot(camera_forward_ws, glm::abs(utils::s_right.xyz())), glm::dot(camera_forward_ws, utils::s_forward.xyz()));
        }
    }

    Ray Camera::screen_point_to_ray(const glm::vec2 &point_ss) const
    {
        return utils::position_ss_to_ray_ws(point_ss, m_ws_to_vs_matrix, m_vs_to_cs_matrix, m_cs_to_ss_vector);
    }

    glm::vec3 Camera::world_to_screen_point(const glm::vec3 &point_ws) const
    {
        return utils::position_ws_to_position_ss(point_ws, m_ws_to_vs_matrix, m_vs_to_cs_matrix, m_cs_to_ss_vector);
    }
}