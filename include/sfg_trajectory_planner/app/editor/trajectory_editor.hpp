#pragma once

#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::app::editor
{
    class TrajectoryEditor : public engine::editor::SceneObjectEditor
    {
    public:
        TrajectoryEditor(engine::editor::SelectionContext &selection_context);
        bool render_editor(engine::core::gfx::Renderer &renderer) override;
        bool render_inspector() override;

    private:
        std::int32_t m_selected_waypoint_index = -1;
    };
}