#pragma once

#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::app::core
{
    class Trajectory;
}

namespace sfg_trajectory_planner::app::editor
{
    class TrajectoryEditor : public engine::editor::SceneObjectEditor<core::Trajectory>
    {
    public:
        TrajectoryEditor(engine::editor::EditorContext &editor_context);
        void render_editor(engine::core::gfx::Renderer &renderer) override;
        void render_inspector() override;

    private:
        size_t m_selected_waypoint_index = std::numeric_limits<size_t>::max();
    };
}