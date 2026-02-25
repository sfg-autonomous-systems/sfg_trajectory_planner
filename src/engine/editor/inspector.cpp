#include "sfg_trajectory_planner/engine/editor/inspector.hpp"

#include "sfg_trajectory_planner/engine/editor/editor_context.hpp"
#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    Inspector::Inspector(const EditorContext &editor_context) : m_editor_context(editor_context)
    {
    }

    void Inspector::render_internal()
    {
        if (m_editor_context.m_editor)
        {
            m_editor_context.m_editor->render_inspector();
        }
    }
}