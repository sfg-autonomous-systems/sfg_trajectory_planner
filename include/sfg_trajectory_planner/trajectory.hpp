#pragma once

#include "sfg_trajectory_planner/core/scene_object.hpp"

namespace sfg_trajectory_planner
{
    class Trajectory : public core::SceneObject
    {
    public:
        Trajectory(core::SceneObjectKey key, core::Scene &scene);
        virtual void render_object(const glm::mat4 &view_matrix, const glm::mat4 &projection_matrix, const glm::vec4 &viewport);
        virtual void render_inspector();
    };
}