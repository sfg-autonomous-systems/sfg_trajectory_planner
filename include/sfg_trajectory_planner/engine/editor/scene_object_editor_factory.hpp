#pragma once

#include "sfg_trajectory_planner/engine/core/generic_factory.hpp"
#include "sfg_trajectory_planner/engine/editor/editor_context.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    class SceneObjectEditorFactory : public core::GenericFactory<SceneObjectEditor<void>, EditorContext &>
    {
    };
}