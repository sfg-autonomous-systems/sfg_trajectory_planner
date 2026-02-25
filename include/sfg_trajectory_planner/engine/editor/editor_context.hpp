#pragma once

#include "sfg_trajectory_planner/engine/editor/history/action.hpp"
#include "sfg_trajectory_planner/engine/editor/history/undo.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    class SceneObjectEditorFactory;
}

namespace sfg_trajectory_planner::engine::editor
{
    class EditorContext
    {
    public:
        EditorContext(core::Scene &scene, SceneObjectEditorFactory &editor_factory);

        SelectionContext m_selection_context;
        history::Undo m_undo;
        std::unique_ptr<SceneObjectEditor<void>> m_editor;
    };
}