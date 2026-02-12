#pragma once

#include "sfg_trajectory_planner/core/scene_object.hpp"

namespace sfg_trajectory_planner
{
    class Grid : public core::SceneObject
    {
    public:
        Grid(core::SceneObjectKey key, core::Scene &scene, uuids::uuid uuid);
        void render_object(core::gfx::Renderer &renderer) override;

    protected:
        void render_inspector_internal() override;

    private:
        glm::vec2 m_grid_size = {10.0f, 10.0f};
        float m_grid_spacing = 1.0f;
        glm::vec3 m_color = {1.0f, 1.0f, 1.0f};
    };
}