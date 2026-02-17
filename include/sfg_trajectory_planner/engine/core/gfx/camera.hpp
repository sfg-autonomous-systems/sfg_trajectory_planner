#pragma once

#include <glm/glm.hpp>

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Ray;
}

namespace sfg_trajectory_planner::engine::core::gfx
{
    class Camera
    {
    public:
        enum class Projection
        {
            Perspective,
            Orthographic
        };

        static constexpr float s_near_plane = 0.1f;
        static constexpr float s_far_plane = 1000.0f;
        static constexpr float s_vertical_fov = glm::radians(45.0f);
        static constexpr float s_min_pitch = glm::radians(-90.0f);
        static constexpr float s_max_pitch = glm::radians(90.0f);

        void set_viewport(const glm::ivec4 &viewport);
        void set_projection(Projection projection);

        Projection get_projection() const;
        glm::mat4 get_view_matrix() const;
        glm::mat4 get_projection_matrix() const;
        glm::ivec4 get_viewport() const;

        void orbit(float delta_x, float delta_y);
        void zoom(float delta);
        void pan(const glm::vec2 &position);
        void start_pan(const glm::vec2 &position);
        void focus_on(const glm::vec3 &point);

        void update();
        void synchronize_from_matrix(const glm::mat4 &view_matrix);
        Ray screen_point_to_ray(const glm::vec2 &point) const;
        glm::vec3 world_to_screen_point(const glm::vec3 &point) const;

    private:
        Projection m_projection = Projection::Perspective;
        glm::vec3 m_orientation = {glm::radians(-30.0f), glm::radians(45.0f), 0.0f};
        glm::vec3 m_focus_point = {0.0f, 0.0f, 0.0f};
        float m_zoom_level = 10.0f;
        glm::vec3 m_pan_start;
        glm::mat4 m_view_matrix;
        glm::mat4 m_projection_matrix;
        glm::ivec4 m_viewport;
    };
}