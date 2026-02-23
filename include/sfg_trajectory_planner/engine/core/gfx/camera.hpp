#pragma once

#include <glm/glm.hpp>

#include "sfg_trajectory_planner/engine/core/gfx/utils.hpp"

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

        void set_cs_to_ss_vector(glm::ivec4 cs_to_ss_vector);
        void set_projection(Projection projection);

        Projection get_projection() const;
        glm::mat4 get_ws_to_vs_matrix() const;
        glm::mat4 get_vs_to_cs_matrix() const;
        glm::ivec4 get_cs_to_ss_vector() const;

        void orbit(float delta_x, float delta_y);
        void zoom(float delta);
        void start_pan(glm::vec2 position_ss);
        void pan(glm::vec2 position_ss);
        void focus_on(glm::vec3 point_ws);

        void update();
        void synchronize_from_matrix(const glm::mat4 &ws_to_vs_matrix);
        Ray screen_point_to_ray(glm::vec2 point_ss) const;
        glm::vec3 world_to_screen_point(glm::vec3 point_ws) const;

    private:
        Projection m_projection = Projection::Perspective;
        float m_pitch = glm::radians(-30.0f);
        float m_yaw = glm::radians(-135.0f);
        glm::vec3 m_focus_point_ws = {0.0f, 0.0f, 0.0f};
        float m_zoom_level = 10.0f;
        glm::vec3 m_pan_start_ws;
        glm::mat4 m_ws_to_vs_matrix;
        glm::mat4 m_vs_to_cs_matrix;
        glm::ivec4 m_cs_to_ss_vector;
    };
}