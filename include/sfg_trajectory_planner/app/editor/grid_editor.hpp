#pragma once

#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::app::core
{
    class Grid;
}

namespace sfg_trajectory_planner::app::editor
{
    class GridEditor : public engine::editor::SceneObjectEditor<core::Grid>
    {
    public:
        GridEditor(engine::editor::EditorContext &editor_context);
        void render_inspector() override;
    };
}