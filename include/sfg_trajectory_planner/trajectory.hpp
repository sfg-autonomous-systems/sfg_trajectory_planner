#pragma once

#include "sfg_trajectory_planner/core/scene_object.hpp"
#include "sfg_trajectory_planner/core/gfx/camera.hpp"

namespace sfg_trajectory_planner
{
    namespace editor
    {
        class SelectionContext;
    }

    class Trajectory : public core::SceneObject
    {
    public:
        Trajectory(core::SceneObject::ConstructionKey key, core::Scene &scene, uuids::uuid uuid, const core::gfx::Camera &camera, editor::SelectionContext &selection_context);
        void render_object(core::gfx::Renderer &renderer) override;

    protected:
        void render_inspector_internal() override;

    private:
        struct Waypoint
        {
            core::Transform m_transform;
            float m_time_from_last = 1.0f;
        };

        const core::gfx::Camera &m_camera;
        editor::SelectionContext &m_selection_context;
        std::string m_topic_name = "/trajectory";
        std::string m_frame_id = "base_link";
        float m_time_from_start = 0.0f;
        glm::vec3 m_color = {0.0f, 1.0f, 0.0f};
        std::vector<Waypoint> m_waypoints;
        std::int32_t m_selected_waypoint_index = -1;
    };
}