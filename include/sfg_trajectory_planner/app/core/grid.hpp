#pragma once

#include "sfg_trajectory_planner/engine/core/scene_object.hpp"

namespace sfg_trajectory_planner::app::core
{
    class Grid : public engine::core::SceneObject
    {
    public:
        Grid(engine::core::SceneObject::ConstructionKey key, const engine::core::Scene &scene, uuids::uuid uuid);
        void serialize(engine::core::serialization::AbstractSerializer *serializer) const override;
        void deserialize(engine::core::serialization::AbstractSerializer *serializer) override;
        void render_object(engine::core::gfx::Renderer &renderer) override;

        glm::vec2 get_grid_size() const;
        float get_grid_spacing() const;
        glm::vec3 get_color() const;

        void set_grid_size(glm::vec2 grid_size);
        void set_grid_spacing(float grid_spacing);
        void set_color(glm::vec3 color);

    private:
        glm::vec2 m_grid_size = {10.0f, 10.0f};
        float m_grid_spacing = 1.0f;
        glm::vec3 m_color = {1.0f, 1.0f, 1.0f};
    };
}