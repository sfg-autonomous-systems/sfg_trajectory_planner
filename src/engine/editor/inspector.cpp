#include "sfg_trajectory_planner/engine/editor/inspector.hpp"

#include "sfg_trajectory_planner/engine/editor/scene_object_editor.hpp"
#include "sfg_trajectory_planner/engine/editor/selection_context.hpp"

namespace sfg_trajectory_planner::engine::editor
{
    Inspector::Inspector(const SelectionContext &selection_context) : m_selection_context(selection_context)
    {
    }

    void Inspector::render_internal()
    {
        if (auto editor = m_selection_context.get_selected_editor())
        {
            editor->render_inspector();
        }
    }
}