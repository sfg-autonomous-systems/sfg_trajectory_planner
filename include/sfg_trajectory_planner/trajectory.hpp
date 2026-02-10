#pragma once

#include "sfg_trajectory_planner/core/scene_object.hpp"

namespace sfg_trajectory_planner
{
    class Trajectory : public core::SceneObject
    {
    public:
        Trajectory(core::SceneObjectKey key, core::Scene &scene);
        void render_object(core::gfx::Renderer &renderer) override;

    protected:
        void render_inspector_internal() override;

    private:
        std::string m_topic_name;
        std::string m_frame_id;
        float m_time_from_start;
        std::vector<float> m_times_from_last;
    };
}