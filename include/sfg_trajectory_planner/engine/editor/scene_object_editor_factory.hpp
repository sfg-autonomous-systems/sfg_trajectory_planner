#pragma once

#include "sfg_trajectory_planner/engine/core/generic_factory.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    class SceneObjectEditor;
    class SelectionContext;
}

namespace sfg_trajectory_planner::engine::editor
{
    using SceneObjectEditorFactory = core::GenericFactory<SceneObjectEditor, const SelectionContext &>;
}