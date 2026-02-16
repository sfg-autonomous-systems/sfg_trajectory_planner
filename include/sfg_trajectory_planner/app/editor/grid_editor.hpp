#pragma once

#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::app::editor
{
    class GridEditor : public engine::editor::SceneObjectEditor
    {
    public:
        GridEditor(const engine::editor::SelectionContext &selection_context);
        bool render_inspector() override;
    };
}